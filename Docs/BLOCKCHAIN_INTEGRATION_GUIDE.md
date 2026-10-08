# Robot Rumble — Blockchain (Sui) Integration Guide

This guide is for the **blockchain & smart contract team** to understand how the Unreal Engine 5 C++ client communicates with Sui Move contracts through the Node.js sidecar bridge, and how to deploy, test, and wire the entire pipeline.

---

## 1. Architecture Overview

```
┌─────────────────────────────────┐
│   Unreal Engine 5 (C++ Client)  │
│   • URRSuiBridge (Subsystem)    │
│   • ARRGameMode (Match outcome) │
│   • ARRHUD / Inventory (Display)│
└────────────────┬────────────────┘
                 │ HTTP REST (JSON)
                 ▼
┌─────────────────────────────────┐
│     Node.js Sidecar Bridge      │  (Runs on localhost:3001)
│     • Express.js Server         │
│     • @mysten/sui TypeScript SDK│
│     • Wallet Connect Browser UI │
└────────────────┬────────────────┘
                 │ Sui RPC
                 ▼
┌─────────────────────────────────┐
│      Sui Network (Testnet)      │
│   • robot_rumble::gear (NFTs)   │
│   • robot_rumble::credits (Coin)│
│   • AdminCap & Vault Objects    │
└─────────────────────────────────┘
```

---

## 2. Directory Structure

| Directory | Purpose |
|---|---|
| `Sui/contracts/` | Move smart contract package (`robot_rumble`). Contains `gear.move` (NFTs) and `credits.move` (currency). |
| `Sui/sidecar/` | Express.js bridge service. Signs transactions using `AdminCap`, exposes REST API to Unreal Engine. |
| `Sui/sidecar/public/connect.html` | Browser wallet connection page supporting Sui Wallet Standard (Suiet, Sui Wallet, etc.). |
| `GameAnimationSample/Source/RRGame/` | C++ game module with `RRSuiBridge.h/.cpp` (HTTP client), HUD inventory UI, and match reward hooks. |
| `GameAnimationSample/Docs/SPEC_SUI.md` | Full HTTP endpoint and Move struct specifications. |

---

## 3. Sidecar REST API Contract

The C++ game client sends HTTP requests to `http://127.0.0.1:3001`:

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/health` | Verifies sidecar and RPC status. Returns `{ "ok": true, "network": "testnet", "package": "0x..." }`. |
| `GET` | `/connect` | Opens in player's default browser to connect their Sui wallet via Wallet Standard. |
| `GET` | `/session` | Polled by Unreal every 1s until connected. Returns `{ "connected": bool, "address": "0x..." }`. |
| `GET` | `/gear/:address` | Returns player's owned gear NFTs (`[Gear]`) and off/on-chain CREDITS balance. |
| `POST` | `/reward` | Called by `ARRGameMode` on match end: `{ "address": "0x...", "won": true, "enemiesKilled": 3 }`. Mints reward NFT and credits. |
| `POST` | `/upgrade` | Upgrades weapon level and stats using player's CREDITS. |

### Gear Object JSON Schema
```json
{
  "id": "0xabc123...",
  "kind": "weapon",
  "name": "Pulse Blade",
  "seed": "9823471928374",
  "damage": 25,
  "armor": 0,
  "power": 0,
  "level": 1,
  "wins": 3
}
```
*Note: `seed` is serialized as a string to preserve 64-bit integer precision across JavaScript and C++.*

---

## 4. Deploying & Running (Step-by-Step)

### Step 1: Deploy Move Contracts to Sui Testnet
1. Ensure the Sui CLI is installed (`sui --version`).
2. Switch to testnet:
   ```bash
   sui client switch --env testnet
   ```
3. Request testnet SUI tokens from https://faucet.sui.io for your active address.
4. Publish the contracts:
   ```bash
   cd Sui/contracts
   sui client publish --gas-budget 200000000
   ```
5. Record the output IDs from the publish transaction:
   - **`PackageID`** (the published package ID)
   - **`AdminCap`** object ID
   - **`Vault`** shared object ID

### Step 2: Configure and Start the Sidecar
1. Navigate to the sidecar folder:
   ```bash
   cd Sui/sidecar
   copy .env.example .env
   ```
2. Open `.env` and fill in the values:
   ```env
   PORT=3001
   SUI_NETWORK=testnet
   PACKAGE_ID=0x<your_package_id>
   ADMIN_CAP_ID=0x<your_admin_cap_id>
   VAULT_ID=0x<your_vault_id>
   ADMIN_PRIVATE_KEY=suiprivkey1...<your_private_key>
   ```
   *(**Never commit `.env` to git!** It is already included in `.gitignore`.)*
3. Install dependencies and start the server:
   ```bash
   npm install
   npm start
   ```
4. Verify by opening `http://127.0.0.1:3001/health` in your browser.

---

## 5. In-Game Testing Controls

When testing the game client:
- **`F2`**: Opens the browser wallet connection page (`http://127.0.0.1:3001/connect`).
- **`F3`**: Manually fetches owned on-chain gear for the connected wallet.
- **`Tab`**: Opens the Resident-Evil-style inventory screen with live **SUI WALLET** status, connected address, credits, and equipped gear.
- **`1–9`**: Equips gear slot *n*.
- **`rr.sui.test`** (in `~` console): Triggers a test Sui reward request.
- **Victory Screen**: After killing the enemies, the victory screen displays the minted reward NFT and a clickable/copyable Suiscan explorer transaction link (`Press C to copy`).

---

## 6. Key C++ Files Reference

- [URRSuiBridge](file:///GameAnimationSample/Source/RRGame/Public/RRSuiBridge.h): GameInstanceSubsystem handling asynchronous HTTP REST communication with the sidecar.
- [ARRGameMode](file:///GameAnimationSample/Source/RRGame/Private/RRGameMode.cpp): Handles match start/victory/defeat, calls `SuiBridge->RequestReward()`.
- [ARRHUD_Inventory](file:///GameAnimationSample/Source/RRGame/Private/RRHUD_Inventory.cpp): Renders wallet address, CREDITS count, and owned gear items.
- [DA_PlayerWeapon](file:///GameAnimationSample/Source/AAACombat/Public/DataAssets/AAAWeaponSettings.h): Weapon data asset holding damage, combo chains, and mesh bindings.
