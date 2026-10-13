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
