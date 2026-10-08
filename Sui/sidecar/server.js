// Robot Rumble sidecar: HTTP bridge between Unreal (C++) and Sui testnet. See Docs/SPEC_SUI.md.
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
const admin = ADMIN_PRIVATE_KEY
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
    seed: String(f.seed),
    damage: Number(f.damage ?? 0),
    armor: Number(f.armor ?? 0),
    power: Number(f.power ?? 0),
    level: Number(f.level ?? 1),
    wins: Number(f.wins ?? 0),
  };
}

async function fetchGear(address) {
  const gear = [];
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
    }
    cursor = page.hasNextPage ? page.cursor : null;
  } while (cursor);
  const bal = await client.getBalance({ owner: address, coinType: `${PACKAGE_ID}::credits::CREDITS` });
  const credits = Number(bal.balance?.balance ?? bal.balance?.totalBalance ?? 0);
  return { gear, credits };
}

const app = express();
app.use(express.json());
app.use(express.static(path.join(path.dirname(fileURLToPath(import.meta.url)), 'public')));

const wrap = (fn) => async (req, res) => {
  try {
    if (!configured) throw Object.assign(new Error('sidecar not configured: fill .env (see .env.example)'), { status: 503 });
    await fn(req, res);
  } catch (e) {
    console.error(e);
    res.status(e.status ?? 500).json({ ok: false, error: e.message ?? String(e) });
  }
};

const PUBLIC_DIR = path.join(path.dirname(fileURLToPath(import.meta.url)), 'public');
app.get('/connect', (_req, res) => res.sendFile(path.join(PUBLIC_DIR, 'connect.html')));
app.get('/health', (_req, res) => res.json({ ok: true, network: NETWORK, package: PACKAGE_ID ?? null, configured }));

app.get('/session', (_req, res) => res.json(session));
app.post('/session', (req, res) => {
  const { address } = req.body ?? {};
  if (!isAddress(address)) return res.status(400).json({ ok: false, error: 'invalid address' });
  session.connected = true;
  session.address = address;
  console.log('wallet connected', address);
  res.json({ ok: true });
});

app.get('/gear/:address', wrap(async (req, res) => {
  if (!isAddress(req.params.address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  res.json({ address: req.params.address, ...(await fetchGear(req.params.address)) });
}));

app.post('/reward', wrap(async (req, res) => {
  const { address, won } = req.body ?? {};
  if (!isAddress(address)) throw Object.assign(new Error('invalid address'), { status: 400 });
  if (won !== true) throw Object.assign(new Error('reward only for a win'), { status: 400 });

  const tx = new Transaction();
  tx.moveCall({
    target: `${PACKAGE_ID}::gear::mint_reward`,
    arguments: [tx.object(ADMIN_CAP_ID), tx.object(VAULT_ID), tx.object(SUI_RANDOM), tx.pure.address(address), tx.pure.bool(true)],
  });
  const execResult = await client.signAndExecuteTransaction({
    transaction: tx,
    signer: admin,
    include: { events: true, effects: true },
  });
  const result = execResult.Transaction ?? execResult.FailedTransaction ?? execResult;
  if (execResult.$kind === 'FailedTransaction') {
    throw new Error(`tx failed: ${JSON.stringify(result.status ?? 'unknown')}`);
  }
  await client.waitForTransaction({ digest: result.digest });

  const ev = (result.events ?? []).find((e) => (e.eventType ?? e.type ?? '').endsWith('::gear::RewardMinted'))?.json;
  let minted = null;
  if (ev?.gear_id) {
    const obj = await client.getObject({ objectId: ev.gear_id, include: { json: true } });
    minted = toGear(obj);
  }
  res.json({ ok: true, digest: result.digest, url: EXPLORER + result.digest, minted, creditsAdded: Number(ev?.credits ?? 0) });
}));

// Upgrades are player-signed (spend the player's CREDITS); done from the wallet page, not here.
app.post('/upgrade', (_req, res) => res.status(501).json({ ok: false, error: 'upgrade is signed in the wallet page' }));

app.listen(PORT, '127.0.0.1', () => console.log(`RR sidecar on http://127.0.0.1:${PORT} (configured=${configured})`));
