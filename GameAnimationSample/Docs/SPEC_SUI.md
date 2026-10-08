# Sui Bridge — HTTP contract (sidecar ⇄ Unreal)

Base URL `http://127.0.0.1:3001`. All bodies JSON. Network: **Sui testnet**.

| Method | Path | Request | Response |
|---|---|---|---|
| GET | `/health` | — | `{ "ok": true, "network": "testnet", "package": "0x…" }` |
| GET | `/connect` | opens in browser | wallet page; on connect it POSTs the address to `/session` |
| GET | `/session` | — | `{ "connected": bool, "address": "0x…"|null }` (game polls every 1 s) |
| GET | `/gear/:address` | — | `{ "address": "0x…", "gear": [Gear], "credits": 123 }` |
| POST | `/reward` | `{ "address": "0x…", "won": true, "enemiesKilled": 3 }` | `{ "ok": true, "digest": "…", "url": "https://suiscan.xyz/testnet/tx/…", "minted": Gear|null, "creditsAdded": 50 }` |
| POST | `/upgrade` | `{ "address":"0x…", "weaponId":"0x…" }` | `{ "ok": true, "digest":"…", "url":"…", "weapon": Gear }` |

Errors: HTTP 4xx/5xx with `{ "ok": false, "error": "message" }`. Game must show the message, never crash.

`Gear` object:
```json
{ "id":"0x…", "kind":"weapon|armor|ability", "name":"Pulse Blade",
  "seed": "123456789", "damage": 25, "armor": 0, "power": 0, "level": 1, "wins": 0 }
```
- `seed` is a **string** (u64 does not fit JS number). C++ parses with `FCString::Atoi64`.
- Only the field relevant to `kind` is non-zero (weapon→damage, armor→armor, ability→power).

## Move package `robot_rumble` (module `gear`)
- `public struct Weapon/Armor/Ability has key, store { id, name, seed: u64, <stat>: u64, level: u64, wins: u64 }`
- `public struct CREDITS has drop` + `sui::coin` currency; `public struct AdminCap has key { id }` (to publisher).
- `entry fun mint_reward(_: &AdminCap, r: &Random, to: address, won: bool, ctx)` — rolls kind + stats with `sui::random` (stats bounded; error codes `E_NOT_WON`).
- `entry fun upgrade_weapon(w: &mut Weapon, payment: Coin<CREDITS>, ctx)` — burns cost = 10×level, `level+=1`, `damage+=5`; `assert!(value >= cost, E_INSUFFICIENT)`.
- Trading = plain `transfer::public_transfer` (objects have `store`).

## Implementation notes (Claude, 2026-10-07)
- `/upgrade` is **not** server-implemented (returns 501): upgrades spend the player's CREDITS and must be player-signed. C++ should treat `UpgradeWeapon` as optional / stretch.
- `/connect` is served as a static page by the sidecar (`public/connect.html`), wallet-standard based, with a paste-address fallback.
- Move modules: `robot_rumble::credits` (CREDITS + shared Vault) and `robot_rumble::gear` (NFTs, `AdminCap`, `mint_reward`, `upgrade_weapon`, `record_win`).
