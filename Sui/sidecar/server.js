// Robot Rumble sidecar: HTTP bridge between Unreal (C++) and Sui testnet.
import 'dotenv/config';
import express from 'express';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { SuiGrpcClient } from '@mysten/sui/grpc';
import { Transaction } from '@mysten/sui/transactions';
import { Ed25519Keypair } from '@mysten/sui/keypairs/ed25519';
import { decodeSuiPrivateKey } from '@mysten/sui/cryptography';

const { PACKAGE_ID, ADMIN_CAP_ID, VAULT_ID, ADMIN_PRIVATE_KEY, PORT = 3001 } = process.env;
const NETWORK = 'testnet';
const EXPLORER = `https://suiscan.xyz/${NETWORK}/tx/`;
const SUI_RANDOM = '0x8';

const client = new SuiGrpcClient({ network: NETWORK, baseUrl: `https://fullnode.${NETWORK}.sui.io:443` });
const configured = Boolean(PACKAGE_ID && ADMIN_CAP_ID && VAULT_ID && ADMIN_PRIVATE_KEY);
const admin = (ADMIN_PRIVATE_KEY && ADMIN_PRIVATE_KEY.startsWith('suiprivkey'))
  ? Ed25519Keypair.fromSecretKey(decodeSuiPrivateKey(ADMIN_PRIVATE_KEY).secretKey)
  : null;

const KINDS = { Weapon: 'weapon', Armor: 'armor', Ability: 'ability' };
const session = { connected: false, address: null };

const isAddress = (a) => typeof a === 'string' && /^0x[0-9a-fA-F]{1,64}$/.test(a);

function toGear(obj) {
  const data = obj.object ?? obj.data ?? obj;
  const type = data.type ?? '';
  const f = data.json ?? data.content?.fields;
  const kindName = Object.keys(KINDS).find((k) => type.endsWith(`::gear::${k}`));
  if (!kindName || !f) return null;
  return {
    id: data.objectId ?? data.id,
    kind: KINDS[kindName],
    name: f.name,
    seed: String(f.seed ?? '0'),
    damage: Number(f.damage ?? 0),
    armor: Number(f.armor ?? 0),
    power: Number(f.power ?? 0),
    level: Number(f.level ?? 1),
    wins: Number(f.wins ?? f.battles_won ?? 0),
    durability: Number(f.durability ?? 100),
    rarity: f.rarity ?? 'Common',
  };
}

function toProfile(obj) {
  const data = obj.object ?? obj.data ?? obj;
  const type = data.type ?? '';
  if (!type.endsWith('::player::PlayerProfile')) return null;
  const f = data.json ?? data.content?.fields;
  if (!f) return null;
  return {
    id: data.objectId ?? data.id,
    owner: f.owner,
    level: Number(f.level ?? 1),
    xp: Number(f.xp ?? 0),
    wins: Number(f.wins ?? 0),
    losses: Number(f.losses ?? 0),
    battlesPlayed: Number(f.battles_played ?? 0),
  };
}

async function fetchGear(address) {
  const gear = [];
  let profile = null;
  let cursor = null;
  do {
    const page = await client.listOwnedObjects({
      owner: address,
      include: { json: true },
      cursor: cursor ?? undefined,
    });
    for (const o of (page.objects ?? [])) {
      const g = toGear(o);
      if (g) gear.push(g);
      const p = toProfile(o);
      if (p) profile = p;
    }
    cursor = page.hasNextPage ? page.cursor : null;
  } while (cursor);

  let credits = 0;
  if (PACKAGE_ID) {
    try {
      const bal = await client.getBalance({ owner: address, coinType: `${PACKAGE_ID}::credits::CREDITS` });
      credits = Number(bal.balance?.balance ?? bal.balance?.totalBalance ?? 0);
    } catch (e) {
      console.warn('Credits balance error:', e.message);
    }
  }
  return { gear, credits, profile };
}

const app = express();
app.use(express.json());
const PUBLIC_DIR = path.join(path.dirname(fileURLToPath(import.meta.url)), 'public');
app.use(express.static(PUBLIC_DIR));

const wrap = (fn) => async (req, res) => {
  try {
    if (!configured) throw Object.assign(new Error('sidecar not configured: fill .env with PACKAGE_ID, ADMIN_CAP_ID, VAULT_ID, ADMIN_PRIVATE_KEY'), { status: 503 });
    await fn(req, res);
  } catch (e) {
    console.error(e);
    res.status(e.status ?? 500).json({ ok: false, error: e.message ?? String(e) });
  }
};

app.get('/connect', (_req, res) => res.sendFile(path.join(PUBLIC_DIR, 'index.html')));
app.get('/health', (_req, res) => res.json({
  ok: true,
  network: NETWORK,
  package: PACKAGE_ID ?? null,
  configured,
  adminAddress: admin ? admin.toSuiAddress() : null
}));

app.get('/session', (_req, res) => res.json(session));
app.post('/session', (req, res) => {
  const address = req.body?.address ?? req.body?.playerAddress;
  if (!isAddress(address)) return res.status(400).json({ ok: false, error: 'invalid address' });
  session.connected = true;
  session.address = address;
  console.log('wallet connected', address);
  res.json({ ok: true });
});

app.get('/balance/:address', wrap(async (req, res) => {
  if (!isAddress(req.params.address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  let credits = 0;
  let sui = 0;
  try {
    const suiBal = await client.getBalance({ owner: req.params.address });
    sui = Number(suiBal.balance?.balance ?? suiBal.balance?.totalBalance ?? 0);
  } catch (e) {}
  if (PACKAGE_ID) {
    try {
      const bal = await client.getBalance({ owner: req.params.address, coinType: `${PACKAGE_ID}::credits::CREDITS` });
      credits = Number(bal.balance?.balance ?? bal.balance?.totalBalance ?? 0);
    } catch (e) {}
  }
  res.json({ ok: true, address: req.params.address, credits, sui });
}));

app.get('/gear/:address', wrap(async (req, res) => {
  if (!isAddress(req.params.address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  res.json({ ok: true, address: req.params.address, ...(await fetchGear(req.params.address)) });
}));

async function executeTx(tx, signer) {
  let lastErr;
  for (let attempt = 1; attempt <= 3; attempt++) {
    try {
      const res = await client.signAndExecuteTransaction({
        transaction: tx,
        signer,
        include: { events: true, effects: true },
      });
      const result = res.Transaction ?? res.FailedTransaction ?? res;
      if (res.$kind === 'FailedTransaction') {
        throw new Error(`tx failed: ${JSON.stringify(result.status ?? 'unknown')}`);
      }
      await client.waitForTransaction({ digest: result.digest });
      return result;
    } catch (e) {
      lastErr = e;
      if (attempt < 3) {
        console.warn(`Tx attempt ${attempt} transient error (${e.message}). Retrying in 1.5s...`);
        await new Promise((r) => setTimeout(r, 1500));
      }
    }
  }
  throw lastErr;
}

app.post('/reward', wrap(async (req, res) => {
  const address = req.body?.address ?? req.body?.playerAddress;
  const won = req.body?.won;
  const enemiesKilled = req.body?.enemiesKilled ?? 3;
  if (!isAddress(address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  if (won !== true) throw Object.assign(new Error('reward only for a win'), { status: 400 });

  const tx = new Transaction();
  tx.moveCall({
    target: `${PACKAGE_ID}::gear::mint_reward`,
    arguments: [
      tx.object(ADMIN_CAP_ID),
      tx.object(VAULT_ID),
      tx.object(SUI_RANDOM),
      tx.pure.address(address),
      tx.pure.bool(true)
    ],
  });

  const result = await executeTx(tx, admin);

  const ev = (result.events ?? []).find((e) => (e.eventType ?? e.type ?? '').endsWith('::gear::RewardClaimed'))?.json;
  let minted = null;
  if (ev?.gear_id) {
    const obj = await client.getObject({ objectId: ev.gear_id, include: { json: true } });
    minted = toGear(obj);
  }
  res.json({
    ok: true,
    digest: result.digest,
    url: EXPLORER + result.digest,
    minted,
    creditsAdded: Number(ev?.credits ?? 50)
  });
}));

app.post('/upgrade', wrap(async (req, res) => {
  const address = req.body?.address ?? req.body?.playerAddress;
  const weaponId = req.body?.weaponId;
  if (!isAddress(address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  if (!weaponId) throw Object.assign(new Error('weaponId required'), { status: 400 });

  // 1. Fetch weapon object details
  const weaponObj = await client.getObject({ objectId: weaponId, include: { json: true } });
  const wData = weaponObj.object?.json ?? weaponObj.data?.content?.fields;
  if (!wData) throw new Error('weapon object not found on chain');
  const level = Number(wData.level ?? 1);
  const cost = level * 10;

  // 2. Fetch player's CREDITS coins
  const coinsPage = await client.listCoins({
    owner: address,
    coinType: `${PACKAGE_ID}::credits::CREDITS`,
  });
  const coins = coinsPage.objects ?? coinsPage.data ?? [];
  if (!coins.length) throw new Error(`Insufficient CREDITS: player has no RRC coin objects`);

  const tx = new Transaction();
  const coinId = coins[0].objectId ?? coins[0].coinObjectId ?? coins[0].id;
  if (!coinId) throw new Error('Invalid coin object ID');
  const primaryCoin = tx.object(coinId);
  if (coins.length > 1) {
    tx.mergeCoins(primaryCoin, coins.slice(1).map(c => tx.object(c.objectId ?? c.coinObjectId ?? c.id)));
  }
  const [splitPayment] = tx.splitCoins(primaryCoin, [tx.pure.u64(cost)]);

  tx.moveCall({
    target: `${PACKAGE_ID}::gear::upgrade_weapon`,
    arguments: [
      tx.object(weaponId),
      tx.object(VAULT_ID),
      splitPayment,
    ],
  });

  const result = await executeTx(tx, admin);

  const ev = (result.events ?? []).find((e) => (e.eventType ?? e.type ?? '').endsWith('::gear::GearUpgraded'))?.json;

  res.json({
    ok: true,
    digest: result.digest,
    url: EXPLORER + result.digest,
    newLevel: Number(ev?.new_level ?? (level + 1)),
    newDamage: Number(ev?.new_stat ?? (Number(wData.damage ?? 0) + 8)),
    costPaid: cost
  });
}));

app.post('/player/create', wrap(async (req, res) => {
  const tx = new Transaction();
  tx.moveCall({
    target: `${PACKAGE_ID}::player::create_player`,
  });

  const result = await executeTx(tx, admin);

  res.json({
    ok: true,
    digest: result.digest,
    url: EXPLORER + result.digest,
  });
}));

app.listen(PORT, '127.0.0.1', () => console.log(`RR sidecar on http://127.0.0.1:${PORT} (configured=${configured})`));
