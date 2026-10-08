# Robot Rumble — Prototype Plan

Goal: a **working vertical slice** — 1 arena, 1 enemy type, PvE, gear NFTs on Sui testnet, reward after victory.
Everything not on this page is out of scope until the slice works. Old docs: `Docs/after/` (reference only).

## What we reuse (already in repo, compiles)
| Existing | Use |
|---|---|
| `AAAAPlayerPawn` + Mover + Motion Matching locomotion | Player movement/animation. Do not touch. |
| `UAAACombatComponent`, `AAAAWeaponActor`, GAS (`UAAAAttributeSet`, `UAAADamageExecutionCalc`) | Melee + damage pipeline. |
| `UAAAAttributeComponent` (OnDeath / OnHealthChanged delegates) | Health/death events for win/lose. |
| `AAAATestEnemy` | Base class for our enemy. |
| `AAAAGameInstance`, `AAAAHUD`, `AAAAPlayerController` | Parent classes for our overrides. |
| `/Game/AAA/Levels/L_Developer` | Arena for now (swap for Medieval Village / Downtown Alley later). |

## What we add — one new module `RRGame` (keeps the 13 AAA modules untouched)
| Class | Job |
|---|---|
| `URRSuiBridge` (GameInstanceSubsystem) | HTTP to Node sidecar `127.0.0.1:3001`. Wallet connect, fetch gear, request reward. |
| `FRRGearData` | Struct mirrored from on-chain gear (id, kind, name, seed, stats, level, wins). |
| `URRLoadoutComponent` | Holds equipped Weapon/Armor/Ability `FRRGearData`, applies stats to the pawn. |
| `URRRobotAppearanceComponent` | Seed → dynamic material params (hue, emissive, metallic) so every robot looks unique. |
| `ARREnemy` (: `AAAATestEnemy`) | Chases player, melee attacks on cooldown, dies → ragdoll. |
| `ARRGameMode` (: `AAAAAGameModeBase`) | Match state machine, enemy spawning, win/lose, triggers reward. |
| `ARRHUD` (: `AAAAAHUD`) | **Canvas-drawn** HUD (no UMG Blueprint): health bars, gear list, victory/defeat screen, Suiscan link. |
| `URRCinematicComponent` | Dueling camera, hit-stop, dash FOV, finisher slow-mo, damage vignette. |

Design rule: **no Blueprint work required for the loop to run.** Blueprints only for final polish.

## Sui side (`/Sui`, repo root)
- `contracts/` Move package `robot_rumble`: `Weapon/Armor/Ability` (key, store), `CREDITS` coin, `AdminCap`, `mint_reward` (random loot via `sui::random`), `upgrade_weapon`, transfer.
- `sidecar/` Node + `@mysten/sui`: REST for the game, wallet page for player signing, admin key from `.env` (never committed).
- Contract of the HTTP API is in `SPEC_SUI.md`; C++ must match it exactly.

## Phases (each ends with something runnable)
| # | Deliverable | Owner | Done when |
|---|---|---|---|
| 0 | Docs moved, plan + specs written | Claude | this commit |
| 1 | PIE sanity: player spawns in L_Developer, moves, attacks `BP_TestEnemy` | Claude (RC/editor) + user | Enemy loses HP in PIE |
| 2 | `RRGame` module + `URRSuiBridge` + `FRRGearData` compile | Antigravity T1 | Editor builds |
| 3 | `ARRGameMode` + `ARREnemy` + `URRLoadoutComponent` | Antigravity T2 | Kill enemy → "VICTORY" log |
| 4 | `ARRHUD` + `URRCinematicComponent` | Antigravity T3 | HUD visible, finisher plays |
| 5 | Move contract deployed to testnet, sidecar live | Claude | `curl /gear/<addr>` returns real objects |
| 6 | Bridge wired: victory → mint → tx link on screen | Claude + Antigravity | Real testnet digest shown |
| 7 | Polish: robot visuals by seed, arena swap, demo video | all | — |

## Workflow (token-saving)
1. Claude writes the spec (`SPEC_CPP.md`) → dispatches to Antigravity with `antigravity-ide chat -m agent -r "<prompt>"`.
2. Claude **reviews every generated file** (`git diff`, read) against the spec *before* building.
3. Build: editor open + Live Coding for edits to existing modules; **new module / .uproject / Target.cs change = close editor, build, reopen**.
4. Editor-side checks via Remote Control (`127.0.0.1:30010`).
5. Log what happened in `PROGRESS.md`.

## Risks
- Existing combat was never PIE-validated (all old TASKS unchecked) → phase 1 is mandatory before building on it.
- Mover pawns don't use `CharacterMovement`; enemy chase uses kinematic movement (see spec), not `MoveToActor`.
- Sui CLI is **not installed** on this machine (needed once to publish the contract).
- Judges/rules: repo derives from Epic's Game Animation Sample → list it (and any Fab/Epic assets) as third-party in the submission.
