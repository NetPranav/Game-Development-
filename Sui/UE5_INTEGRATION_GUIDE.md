# Robot Rumble: Unreal Engine 5 Blockchain Integration Guide (Part 2)

**Version:** 2.0.0  
**Protocol:** Sui Blockchain (Testnet)  
**Package ID (v1):** `0x77c5a43a31826c687dd2d3152bbbfe56eca8c8c2c979618c62468f4f1b7fb017`  
**Package ID (v2 - Governance):** `0xc9fc9cd0c33961967cf0c0a039676cd9bf2ea29f409bd2b1748631b288bb0353`  
**Shared Treasury Vault:** `0x1e7e0fa2ae5bb60a392be64d3d3f4d0c3f0cbc7ab3dd097f28fa6fea130804ba`  
**Governed Config:** `0xd93431be0bf329dec9f5b094a90a57847ab49edca36ddcad08570dc959e0cb35`  
**Default Sidecar Host:** `http://127.0.0.1:3001`  

---

## 1. Overview for Game Developers

The blockchain layer for **Robot Rumble** is designed around a lightweight, local HTTP bridge pattern (`URRSuiBridge` ⇄ Node.js Sidecar ⇄ Sui Testnet).

This architecture allows the 3D Unreal Engine 5 client to remain completely decoupled from blockchain cryptography, SDK dependencies, and RPC connection management:
- Unreal Engine makes standard asynchronous HTTP REST calls to `http://127.0.0.1:3001`.
- The Node.js Sidecar manages cryptographic signing via `AdminCap`, communicates with Sui Testnet fullnodes using the modern gRPC client, enforces retry policies for transient network drops, and updates on-chain state.
- All assets (Weapons, Armor, Abilities, Player Profiles, and RRC Tokens) exist as real, verifiable objects on the Sui blockchain.

---

## 2. Endpoints & Integration Schemas

### 2.1 Health Check
- **Method:** `GET /health`
- **Purpose:** Used by UE5 during subsystem initialization (`URRSuiBridge::CheckHealth()`) to verify that the local sidecar bridge is operational.
- **Response (200 OK):**
```json
{
  "ok": true,
  "network": "testnet",
  "package": "0x77c5a43a31826c687dd2d3152bbbfe56eca8c8c2c979618c62468f4f1b7fb017",
  "packageV2": "0xc9fc9cd0c33961967cf0c0a039676cd9bf2ea29f409bd2b1748631b288bb0353",
  "configured": true,
  "adminAddress": "0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8",
  "governedConfigId": "0xd93431be0bf329dec9f5b094a90a57847ab49edca36ddcad08570dc959e0cb35"
}
```

---

### 2.2 Wallet Connection (`F2` Flow)
- **Method:** `GET /connect` & `GET /session`
- **Workflow:**
  1. User presses `F2` in the game.
  2. UE5 calls `FPlatformProcess::LaunchURL(TEXT("http://127.0.0.1:3001/connect"))` which opens the dashboard in the default browser.
  3. UE5 starts a 1.0s polling loop against `GET /session` (up to 120 seconds).
  4. Once the player connects or pastes their Sui wallet in the dashboard, the sidecar registers the session.
  5. `GET /session` returns `{ "connected": true, "address": "0x..." }`.
  6. UE5 triggers `OnWalletConnected(Address)` and initiates `FetchGear()`.

---

### 2.3 Gear & Profile Fetching (`F3` / Tab Flow)
- **Method:** `GET /gear/:address`
- **Purpose:** Fetches all player-owned NFTs (Weapons, Armor, Abilities) and the on-chain PlayerProfile.
- **Response (200 OK):**
```json
{
  "ok": true,
  "address": "0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8",
  "gear": [
    {
      "id": "0x65a7cc1abe03490e02823ff0f8f0f7890298755a266ef906a89503f38c6a2094",
      "kind": "weapon",
      "name": "Pulse Blade",
      "seed": "12188907278779384079",
      "damage": 57,
      "armor": 0,
      "power": 0,
      "level": 3,
      "wins": 1,
      "durability": 100,
      "rarity": "Rare",
      "url": "https://suiscan.xyz/testnet/object/0x65a7cc1abe03490e02823ff0f8f0f7890298755a266ef906a89503f38c6a2094"
    }
  ],
  "credits": 40,
  "profile": {
    "id": "0x3de8d6a2a9c0f2aacef82614c7a608383913700ed0c04e0f81a35d0a853d7082",
    "owner": "0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8",
    "level": 1,
    "xp": 0,
    "wins": 0,
    "losses": 0,
    "battlesPlayed": 0
  }
}
```

---

### 2.4 Arena Battle Victory Reward
- **Method:** `POST /reward`
- **Request Body:**
```json
{
  "address": "0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8",
  "won": true,
  "enemiesKilled": 3
}
```
- **Security Constraint:** Requests with `won: false` are rejected with HTTP 400 (`"reward only for a win"`).
- **Response (200 OK):**
```json
{
  "ok": true,
  "digest": "3yZbb9XdAMyTQwqZZkXYACpWM6uose57Gp8QecfHgtUB",
  "url": "https://suiscan.xyz/testnet/tx/3yZbb9XdAMyTQwqZZkXYACpWM6uose57Gp8QecfHgtUB",
  "minted": {
    "id": "0x37e7600707d0721419bb29611e4e4f5c3d0e49fb7d36887959f51db387f4ba6e",
    "kind": "weapon",
    "name": "Arc Hammer",
    "damage": 30,
    "level": 1,
    "durability": 100,
    "rarity": "Common"
  },
  "creditsAdded": 50
}
```

---

### 2.5 Weapon Upgrade & Token Burn
- **Method:** `POST /upgrade`
- **Request Body:**
```json
{
  "address": "0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8",
  "weaponId": "0x65a7cc1abe03490e02823ff0f8f0f7890298755a266ef906a89503f38c6a2094"
}
```
- **On-Chain Action:**
  - Calculates cost: `cost = level * 10 RRC`.
  - Splits payment from player's `Coin<CREDITS>` objects.
  - Calls `gear::upgrade_weapon`, burning the RRC payment into the shared Vault.
  - Increases damage stat by +8 and increments weapon level.
- **Response (200 OK):**
```json
{
  "ok": true,
  "digest": "J7Yn9gmxmCGioZXJXiJNU6ban7hBEuedRBhtyuhB7Vx7",
  "url": "https://suiscan.xyz/testnet/tx/J7Yn9gmxmCGioZXJXiJNU6ban7hBEuedRBhtyuhB7Vx7",
  "newLevel": 3,
  "newDamage": 57,
  "costPaid": 20
}
```

---

## 3. On-Chain Governance (Part 2 Interface)

The Part 2 governance system allows community parameter changes enforced entirely in Move:
- **`GET /governance/config`**: Reads current governed game parameters (Victory Reward, Base Upgrade Cost, Max Level, Marketplace Fee).
- **`GET /governance/proposals`**: Returns all proposals, voting power, quorum status, and timelock countdowns.
- **`POST /governance/proposals`**: Proposes a parameter change.
- **`POST /governance/proposals/:id/vote`**: Submits a vote (`voteType: 1` = For, `0` = Against, `2` = Abstain).

---

## 4. Error Handling & Retry Engine

1. **Transient 503 RPC Retries:** The sidecar incorporates an automatic 3-attempt exponential backoff retry mechanism (`executeTx`) for transient Sui Testnet gateway dropped connections.
2. **Offline Fallback:** If the sidecar is not running, `URRSuiBridge` gracefully emits `OnBridgeError("Sui sidecar unreachable")` once without crashing or freezing the game loop.
3. **Invalid Addresses:** All endpoints strictly validate hex address syntax (`0x[0-9a-fA-F]{1,64}`) and return HTTP 400 on malformed input.

---

## 5. Summary of Decisions & Outstanding Items

| Topic | Status | Architectural Decision |
| :--- | :---: | :--- |
| **Token Standard** | Finalized | Native Sui Coin (`robot_rumble::credits::CREDITS`), 0 decimals, shared Vault treasury |
| **NFT Standard** | Finalized | Native Sui Move objects with `key, store` capabilities |
| **Governance Quorum** | Finalized | 20% of snapshot eligible voting power |
| **Governance Approval** | Finalized | Strictly > 51% of participating non-abstaining votes |
| **Governance Timelock** | Finalized | 48 hours mandatory delay before execution |
| **Voting Eligibility Rule** | Undecided | Decoupled via `VotingPowerVoucher`. Eligibility threshold (pure tokens vs staked tokens vs NFT ownership) remains intentionally configurable by the team. |
| **Multi-Avatar Policy** | Undecided | Wallets can hold multiple gear objects. Equipping constraints (one avatar vs loadout presets) remain an Unreal Engine UI level decision. |
| **Marketplace Royalties** | Undecided | Marketplace supports P2P trade; final creator royalty percentage is configurable via Governance Parameter #4 (`PARAM_MARKETPLACE_FEE_BPS`). |
| **Match Staking Model** | Undecided | Supported in test harness with zero-financial risk; final token staking terms remain configurable pending legal & design review. |
| **Scoring Token Rate** | Configurable | Base 30 RRC + performance bonus (1 RRC / 10 pts, capped at +20 RRC); max 1,000 RRC per match. |

---

## 6. Gameplay Scoring & Batched Settlement Integration (Part 2 Extension)

### Architecture Overview

To eliminate gas fees during active gameplay and prevent high-frequency blockchain latency, Robot Rumble strictly separates:
1. **Gameplay Points (Off-Chain):** Recorded in memory / server database during combat. 0 gas.
2. **Match Stakes (Off-Chain / Isolated):** Pre-match commitment status tracked off-chain until formal economy approval.
3. **RRC Rewards (On-Chain Settlement):** Single atomic transaction (`settlement::settle_match_batch`) submitted upon match conclusion.

```
Unreal Engine 5 (Dedicated Server)
   │
   ├─► POST /matches (Register Match & Participants)
   │
   ├─► POST /matches/:id/events (Stream combat events in real time: headshot, kill, assist, objective)
   │      [Off-chain verification & deduplication: 0 gas]
   │
   ├─► POST /matches/:id/end (Conclude match & calculate deterministic rewards)
   │
   └─► POST /matches/:id/settle (Trusted Sidecar executes single batched Sui settlement transaction)
          │
          ▼
       Sui Testnet (settlement.move::settle_match_batch)
          - Idempotent match registry verification (prevents replay/duplicate settlement)
          - Enforces MAX_MATCH_REWARD_BUDGET (1,000 RRC)
          - Atomic payout loop to all player wallets
```

### Unreal Engine Dedicated Server API Contracts

#### 1. Create Match
- **Endpoint:** `POST /matches`
- **Request:**
  ```json
  {
    "title": "Cyber Arena Elimination #9001",
    "gameMode": "3v3 Elimination",
    "stakeAmount": 0,
    "participants": [
      { "address": "0xaf41e408...99f8", "name": "Pilot_Alpha", "team": "Blue" },
      { "address": "0x58219c01...12ab", "name": "Pilot_Beta", "team": "Red" }
    ]
  }
  ```
- **Response:** `{ "ok": true, "matchId": 1791892132, "match": { ... } }`

#### 2. Ingest Off-Chain Combat Event
- **Endpoint:** `POST /matches/:id/events`
- **Request:**
  ```json
  {
    "eventId": "match_9001_seq_42",
    "playerAddress": "0xaf41e408...99f8",
    "eventType": "headshot"
  }
  ```
- **Supported `eventType` values & base scores:**
  - `headshot`: +25 points
  - `kill`: +50 points
  - `assist`: +15 points
  - `objective`: +100 points
  - `level_complete`: +75 points
- **Response:**
  ```json
  {
    "ok": true,
    "matchId": 1791892132,
    "event": { "eventId": "match_9001_seq_42", "points": 25, "seq": 5 },
    "updatedScore": 190,
    "totalEvents": 5
  }
  ```
- **Idempotency Note:** If `eventId` has already been recorded, returns `{ "ok": true, "deduplicated": true }` with no score change.

#### 3. Conclude Match & Calculate Payouts
- **Endpoint:** `POST /matches/:id/end`
- **Response:**
  ```json
  {
    "ok": true,
    "matchId": 1791892132,
    "payoutPlan": {
      "totalPayoutRRC": 47,
      "recipientsCount": 1,
      "recipients": ["0xaf41e408...99f8"],
      "amounts": [47]
    }
  }
  ```

#### 4. Settle Match on Sui Testnet (Single Batched Transaction)
- **Endpoint:** `POST /matches/:id/settle`
- **Response:**
  ```json
  {
    "ok": true,
    "matchId": 1791892132,
    "digest": "7BBdnpapfHTELoRyBsb8fjPbdyivWkS6EsVsDCCHse22",
    "url": "https://suiscan.xyz/testnet/tx/7BBdnpapfHTELoRyBsb8fjPbdyivWkS6EsVsDCCHse22",
    "settlement": {
      "totalPayoutRRC": 47,
      "gasUsedSui": 0.003881844,
      "eventsAvoided": 3
    }
  }
  ```

---

## 7. Measured Gas Efficiency Benchmark

| Metric | Batched Settlement Architecture | Per-Action Individual Transactions (Rejected) | Improvement |
| :--- | :---: | :---: | :---: |
| **Transactions per Match** | **1** | 40+ (1 per combat action) | **> 97% transaction reduction** |
| **Total Gas Consumption** | **~0.00388 SUI** | ~0.10000 SUI (40 × 0.0025 SUI) | **> 96% gas cost savings** |
| **Gameplay Latency** | **0 ms (instant local response)** | 400 - 800 ms per action (Sui consensus wait) | **Zero gameplay stuttering** |
| **Replay & Cheat Resistance** | **On-Chain Table Idempotency** | Vulnerable to event stream flooding | **Cryptographically secured** |

