# Progress Log

Plan: `PLAN.md` · C++ specs: `SPEC_CPP.md` · Sui API: `SPEC_SUI.md` · Old docs: `after/`

| Phase | Status | Notes |
|---|---|---|
| 0 Docs + plan | done | Docs/after = old docs |
| 1 PIE sanity of existing combat | done | headless + rendered runs: enemies fight, damage, deaths, match flow |
| 2 RRGame module + Sui bridge (P1) | done | compiles |
| 3 Combat core (P3 poise/effects, P4 melee/combo, P5 block/parry/dodge, P6 hit feedback/lock-on) | done | all compile; verified in headless fights |
| 4 Enemy (P7) + match/HUD/loadout (P8) | done | Greystone enemy BP, canvas HUD, game mode BP, L_RRArena |
| 5 Move contract + sidecar | built, NOT published | needs testnet SUI (faucet web page) |
| 6 end-to-end reward | pending | after publish |
| 7 polish: weapon mesh, robot visuals from gear seed, retarget quality pass, demo video | pending | |

## Log (newest last)
P1 done: RRGame module + Sui Bridge created and integrated
P2 done: Combat audit completed, identified missing features and bugs in trace logic
P3 done: Combat foundation (tags, poise, C++ effects, settings, damage pipeline, regen).
P4 done (verified by build5: full editor build Succeeded)
P5 done: Added Mover dodge, block, parry, and debug console commands.
P5 done (build7 Succeeded)
P6a done: FAAACombatEvents and Knockback values
P6b done: Hit-feedback subsystem and Knockback subsystem
P6a, P6b done (build13 Succeeded)
P6c done (build14 Succeeded)
P7 done (agy part + reviewer-written URREnemyDirector; build19 Succeeded)
P8 done: Match flow, gear loadout, canvas HUD, and Sui reward wiring
P9 done: Inventory tab screen + gear equip visuals
P9 done + robot visual, sword, combo/assist upgrades (build41 Succeeded; smoke tests pass)

- Front end (splash/menu/settings/blockchain/loading), defeat screen + respawn at movable PlayerStart, RE2-style inventory: build47 Succeeded, screens verified by capture.
P10d done: RE8 damage screen, no player health bar, combat montages fixed
P11a done: ARRBoss core mechanics, segments, posture, deathblow, and debug commands
P11c done: Implemented Sekiro-style Boss HUD (Canvas), added Boss getters, and finalized native boss gameplay tags.
P11b done: Implemented Mikiri counter, Perilous attacks, and ARRBossAIController
