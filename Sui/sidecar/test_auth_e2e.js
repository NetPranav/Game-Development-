import { Ed25519Keypair } from '@mysten/sui/keypairs/ed25519';

async function testAuth() {
  const base = 'http://127.0.0.1:3001';

  // Generate test keypair
  const kp = new Ed25519Keypair();
  const address = kp.toSuiAddress();
  console.log('Testing with keypair address:', address);

  // 1. Request single-use nonce
  console.log('--- 1. Requesting authentication nonce ---');
  const nonceRes = await fetch(`${base}/auth/nonce?address=${address}`).then(r => r.json());
  console.log('Nonce response:', nonceRes);
  if (!nonceRes.ok || !nonceRes.nonce || !nonceRes.message) {
    throw new Error('Failed to obtain nonce');
  }

  // 2. Sign personal message
  console.log('--- 2. Signing personal message with private key ---');
  const messageBytes = new TextEncoder().encode(nonceRes.message);
  const { signature } = await kp.signPersonalMessage(messageBytes);
  console.log('Signature generated:', signature.slice(0, 30) + '...');

  // 3. Verify signature on backend
  console.log('--- 3. Submitting signature for cryptographic verification ---');
  const verifyRes = await fetch(`${base}/auth/verify`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      address,
      signature,
      nonce: nonceRes.nonce,
    }),
  }).then(r => r.json());
  console.log('Verify response:', verifyRes);
  if (!verifyRes.ok || !verifyRes.authenticated || !verifyRes.sessionToken) {
    throw new Error('Authentication verification failed');
  }

  // 4. Test anti-replay protection (resubmitting same nonce must fail!)
  console.log('--- 4. Testing anti-replay protection (submitting same nonce) ---');
  const replayRes = await fetch(`${base}/auth/verify`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      address,
      signature,
      nonce: nonceRes.nonce,
    }),
  }).then(r => r.json());
  console.log('Replay attempt response (must fail):', replayRes);
  if (replayRes.ok) {
    throw new Error('Anti-replay test failed: used nonce was accepted!');
  }

  // 5. Inspect status with session token
  console.log('--- 5. Checking auth status with session token ---');
  const statusRes = await fetch(`${base}/auth/status`, {
    headers: { Authorization: `Bearer ${verifyRes.sessionToken}` },
  }).then(r => r.json());
  console.log('Auth status:', statusRes);

  // 6. Inspect personalized account data
  console.log('--- 6. Querying account overview ---');
  const accRes = await fetch(`${base}/account/${address}`, {
    headers: { Authorization: `Bearer ${verifyRes.sessionToken}` },
  }).then(r => r.json());
  console.log('Account overview balances:', accRes.balances, 'authenticated:', accRes.authenticated);

  // 7. Logout
  console.log('--- 7. Logging out ---');
  const logoutRes = await fetch(`${base}/auth/logout`, {
    method: 'POST',
    headers: { Authorization: `Bearer ${verifyRes.sessionToken}` },
  }).then(r => r.json());
  console.log('Logout response:', logoutRes);

  console.log('ALL CRYPTOGRAPHIC WALLET AUTH TESTS PASSED!');
}

testAuth().catch(console.error);
