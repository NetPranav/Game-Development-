# Robot Rumble — Sui Blockchain & Move Architecture

This directory contains the **Sui Blockchain layer** for **Robot Rumble**, providing on-chain gear NFTs, RRC utility tokens, player progression, battle verification, and an asynchronous HTTP sidecar bridge.

---

## 1. Architecture Overview

```
┌──────────────────────────────────────────────┐
│  Browser Test Harness / UE5 Client (Future)  │
│  • Player Wallet Standard (Sloth, Sui Wallet)│
│  • Simulated Victory (+50 RRC / Gear NFT)    │
│  • Live Inventory & Gear Card Inspector      │
│  • Real-time Upgrade Controls & Status Toast │
└──────────────────────┬───────────────────────┘
                       │ HTTP REST (127.0.0.1:3001)
                       ▼
┌──────────────────────────────────────────────┐
│            Node.js Sidecar Bridge            │
│  • Express.js Server (@mysten/sui v2.35.0)   │
│  • Endpoints: /health, /gear, /reward, etc.  │
│  • Secures ADMIN_PRIVATE_KEY server-side     │
└──────────────────────┬───────────────────────┘
                       │ Sui RPC (Testnet)
                       ▼
┌──────────────────────────────────────────────┐
│         Sui Move Contracts (robot_rumble)    │
│  • player.move      : PlayerProfile Objects  │
│  • gear.move        : Weapon/Armor/Ability   │
│  • credits.move     : RRC Fungible Token     │
│  • battle.move      : Authorized Results     │
│  • marketplace.move : P2P Trading (Kiosk)    │
│  • sui::random      : Native VRF Randomness  │
└──────────────────────────────────────────────┘
```

---

## 2. Sui Testnet Deployment Details

The Move package is published and verified on **Sui Testnet**:

| Parameter | Value / Object ID | Explorer Link |
|---|---|---|
| **Network** | `testnet` | [Suiscan Testnet](https://suiscan.xyz/testnet) |
| **Package ID** | `0x77c5a43a31826c687dd2d3152bbbfe56eca8c8c2c979618c62468f4f1b7fb017` | [View Package](https://suiscan.xyz/testnet/package/0x77c5a43a31826c687dd2d3152bbbfe56eca8c8c2c979618c62468f4f1b7fb017) |
| **Publish Tx Digest**| `Egg2gWuvvjvzYKfaQHjyitVgLdA8ARutNJtD8XEd27Gz` | [View Publish Tx](https://suiscan.xyz/testnet/tx/Egg2gWuvvjvzYKfaQHjyitVgLdA8ARutNJtD8XEd27Gz) |
| **AdminCap ID** | `0xf4a906e9be5c122d92045e5cf581e6801a9594a412020f726729d452edded1a3` | [View AdminCap](https://suiscan.xyz/testnet/object/0xf4a906e9be5c122d92045e5cf581e6801a9594a412020f726729d452edded1a3) |
| **Vault (Shared)** | `0x1e7e0fa2ae5bb60a392be64d3d3f4d0c3f0cbc7ab3dd097f28fa6fea130804ba` | [View Vault](https://suiscan.xyz/testnet/object/0x1e7e0fa2ae5bb60a392be64d3d3f4d0c3f0cbc7ab3dd097f28fa6fea130804ba) |
| **UpgradeCap ID** | `0xc61010cbddd5a2dced38bd563e76f838d653051f07bde32e44af69cb21b3bb2a` | [View UpgradeCap](https://suiscan.xyz/testnet/object/0xc61010cbddd5a2dced38bd563e76f838d653051f07bde32e44af69cb21b3bb2a) |
| **Coin Metadata (RRC)** | `0xf539969304bed78d37c34938437a1c9e5ca7599179cbdfce8f8d295229d3a89e` | [View Metadata](https://suiscan.xyz/testnet/object/0xf539969304bed78d37c34938437a1c9e5ca7599179cbdfce8f8d295229d3a89e) |
| **Coin Type** | `0x77c5a43a31826c687dd2d3152bbbfe56eca8c8c2c979618c62468f4f1b7fb017::credits::CREDITS` | - |
| **Admin Address** | `0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8` | [View Address](https://suiscan.xyz/testnet/account/0xaf41e4087b2d2b86a89c506eaf31cf571150a62c043a27b5d34ca4b03eb299f8) |

---

## 3. Move Modules Breakdown

### 1. `player.move`
* **Object**: `PlayerProfile has key`
  * `id: UID`
  * `owner: address`
  * `level: u64`, `xp: u64`, `wins: u64`, `losses: u64`, `battles_played: u64`
* **Functions**:
  * `create_player(ctx: &mut TxContext)`: Mints new profile to transaction sender.
  * `record_battle_result(profile: &mut PlayerProfile, won: bool, xp_gained: u64)`: Authoritative progression; auto-levels up when XP threshold is met.
* **Events**: `PlayerCreated`, `PlayerProgressed`.

### 2. `gear.move`
* **Objects**:
  * `Weapon has key, store`: `damage`, `level`, `durability`, `wins`, `rarity`, `seed`, `weapon_type`.
  * `Armor has key, store`: `armor`, `level`, `durability`, `wins`, `rarity`, `seed`, `armor_type`.
  * `Ability has key, store`: `power`, `level`, `cooldown`, `wins`, `rarity`, `seed`, `ability_type`.
* **Randomized Minting (`mint_reward`)**: Uses Sui on-chain VRF randomness (`sui::random::Random`, object `0x8`) to determine item slot, base stats, unique seed, and rarity tier (Common, Rare, Epic, Legendary).
* **On-Chain Upgrades (`upgrade_weapon`, `upgrade_armor`, `upgrade_ability`)**:
  * Validates and burns player's `Coin<CREDITS>`.
  * Increments item level and boosts gameplay stats (+8 Damage for Weapons, +6 Armor, +7 Power).
* **Events**: `GearMinted`, `RewardClaimed`, `GearUpgraded`, `GearRepaired`.

### 3. `credits.move`
* **Coin**: `CREDITS` (`RRC - Robot Credits`, 0 decimals).
* **Vault**: Shared object holding `TreasuryCap<CREDITS>`, allowing the package to mint rewards on verified wins and burn coins on equipment upgrades.

### 4. `battle.move`
* **Function**: `record_battle(_cap: &AdminCap, profile: &mut PlayerProfile, battle_id: u64, won: bool, enemies_killed: u64)`
* **Authorization**: Gated by `AdminCap`. Awards XP and increments match statistics authoritative on-chain.
* **Events**: `BattleCompleted`.

### 5. `marketplace.move`
* **Object**: `Listing has key, store`
* **Functions**:
  * `list_weapon(weapon: Weapon, price: u64, ctx: &mut TxContext)`
  * `buy_weapon(listing: Listing, payment: Coin<CREDITS>, ctx: &mut TxContext)`
  * `cancel_listing(listing: Listing, ctx: &mut TxContext)`
* **Events**: `GearListed`, `GearPurchased`, `ListingCancelled`.

---

## 4. Sidecar REST API Contract

The Express sidecar bridge runs on `http://127.0.0.1:3001`:

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/health` | Returns service status, network, package ID, and admin address. |
| `GET` | `/connect` | Opens the Cyberpunk Blockchain Test Harness dashboard. |
| `POST` | `/session` | Stores active wallet session. |
| `GET` | `/session` | Returns current connected wallet session. |
| `GET` | `/gear/:address` | Returns owned on-chain Gear NFTs, RRC token balance, and PlayerProfile. |
| `GET` | `/balance/:address` | Returns RRC coin balance and native SUI gas balance. |
| `POST` | `/reward` | Backend signs on-chain VRF minting: `{ "address": "0x...", "won": true, "enemiesKilled": 3 }`. |
| `POST` | `/upgrade` | Upgrades weapon on-chain by spending RRC: `{ "address": "0x...", "weaponId": "0x..." }`. |
| `POST` | `/player/create` | Initializes on-chain `PlayerProfile` for the connected address. |

---

## 5. Live Testnet Verified Transactions

All transactions are verifiable on Suiscan Testnet Explorer:

1. **Package Deployment**:
   * Digest: [`Egg2gWuvvjvzYKfaQHjyitVgLdA8ARutNJtD8XEd27Gz`](https://suiscan.xyz/testnet/tx/Egg2gWuvvjvzYKfaQHjyitVgLdA8ARutNJtD8XEd27Gz)
2. **Player Profile Creation**:
   * Digest: [`AwCoAPhJNK4HMjzr4eKuUcNwkQFoHMbPVBoH7CAt23mT`](https://suiscan.xyz/testnet/tx/AwCoAPhJNK4HMjzr4eKuUcNwkQFoHMbPVBoH7CAt23mT)
3. **Simulated Match Victory (VRF Gear NFT + 50 RRC Minted)**:
   * Digest: [`HZPhXe7pCXYDhF72N2sCGDahbkRnyPnbqFLPGs752Qht`](https://suiscan.xyz/testnet/tx/HZPhXe7pCXYDhF72N2sCGDahbkRnyPnbqFLPGs752Qht)
   * Minted Object: `Pulse Blade` (`0x65a7cc1abe03490e02823ff0f8f0f7890298755a266ef906a89503f38c6a2094`), Damage 41, Rarity Rare.
4. **Weapon Upgrade Transaction (10 RRC Burned, Level 1 -> 2, Damage 41 -> 49)**:
   * Digest: [`HYQu1DVnuuecu3QXRYNhyzWaNAnM53dL7tFkjLLkDb7`](https://suiscan.xyz/testnet/tx/HYQu1DVnuuecu3QXRYNhyzWaNAnM53dL7tFkjLLkDb7)

---

## 6. How to Run & Verify

1. **Verify Sui CLI & Move Tests**:
   ```bash
   cd Sui/contracts
   sui --version
   sui move test
   ```
   *Result: 3 / 3 unit tests passed.*

2. **Start the Sidecar Bridge**:
   ```bash
   cd Sui/sidecar
   npm start
   ```

3. **Open the Test Harness Dashboard**:
   Open [`http://127.0.0.1:3001`](http://127.0.0.1:3001) in your browser:
   * Connect your Sui Wallet or paste your test address.
   * Click **"Simulate Match Victory"** to mint real on-chain gear NFTs and RRC coins.
   * Inspect your owned gear cards with live level, durability, rarity, and stats.
   * Click **"Upgrade"** to execute a real Move transaction burning RRC and boosting damage.
