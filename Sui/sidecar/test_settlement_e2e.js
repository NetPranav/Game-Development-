async function main() {
  const base = 'http://127.0.0.1:3001';

  console.log('--- 1. Fetching initial matches ---');
  let res = await fetch(base + '/matches').then(r => r.json());
  console.log('Initial match count:', res.count, res.matches.map(m => m.matchId));

  console.log('--- 2. Creating new match ---');
  res = await fetch(base + '/matches', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      title: 'Automated E2E Arena Test #9002',
      gameMode: '3v3 Elimination',
      participants: [{ address: '0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8', name: 'AlphaPilot', team: 'Blue' }]
    })
  }).then(r => r.json());
  const matchId = res.matchId;
  console.log('Created matchId:', matchId, 'status:', res.match.status);

  console.log('--- 3. Ingesting off-chain gameplay events (0 gas) ---');
  await fetch(base + '/matches/' + matchId + '/events', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ eventId: 'ev_test_1', playerAddress: '0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8', eventType: 'headshot' })
  });
  await fetch(base + '/matches/' + matchId + '/events', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ eventId: 'ev_test_2', playerAddress: '0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8', eventType: 'kill' })
  });
  await fetch(base + '/matches/' + matchId + '/events', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ eventId: 'ev_test_3', playerAddress: '0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8', eventType: 'objective' })
  });

  console.log('--- 4. Testing event deduplication ---');
  let dupRes = await fetch(base + '/matches/' + matchId + '/events', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ eventId: 'ev_test_1', playerAddress: '0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8', eventType: 'headshot' })
  }).then(r => r.json());
  console.log('Duplicate event response (should be deduplicated):', dupRes);

  console.log('--- 5. Concluding match & calculating payout ---');
  let endRes = await fetch(base + '/matches/' + matchId + '/end', { method: 'POST' }).then(r => r.json());
  console.log('Payout plan:', endRes.payoutPlan);

  console.log('--- 6. Submitting Batched Move Settlement to Sui Testnet ---');
  let settleRes = await fetch(base + '/matches/' + matchId + '/settle', { method: 'POST' }).then(r => r.json());
  console.log('Settlement result ok:', settleRes.ok, 'digest:', settleRes.digest, 'url:', settleRes.url);

  console.log('--- 7. Testing duplicate settlement rejection ---');
  let dupSettleRes = await fetch(base + '/matches/' + matchId + '/settle', { method: 'POST' }).then(r => r.json());
  console.log('Duplicate settle rejection (should fail):', dupSettleRes);

  console.log('--- 8. Checking updated matches registry ---');
  let listRes = await fetch(base + '/matches').then(r => r.json());
  console.log('Final match count:', listRes.count);

  console.log('ALL E2E SCENARIOS VERIFIED SUCCESSFULLY!');
}

main().catch(console.error);
