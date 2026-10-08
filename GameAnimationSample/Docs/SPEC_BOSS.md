# Boss duel spec (Sekiro-style) — read by prompts P10, P11, P12

## Reference research (what Sekiro does, mapped to our controls)
Sources: Fextralife Sekiro combat wiki, Game Wisdom "Breaking down Sekiro's combat", Game Developer "How I broke Sekiro", PC Gamer guide.
- **Posture, not health, decides the fight.** Every enemy has a posture bar; filling it opens a **deathblow**. Health alone is secondary.
- **Deflect** = tap block just before impact: no damage to you, big posture damage to the attacker, orange spark + loud clang, attacker recoils. Plain **block** (hold) is safer but raises YOUR posture and does tiny posture damage. Holding block too long is punished (posture builds on you faster).
- **Dodge/step** has i-frames; used for sweeps and as the answer to thrusts.
- **Perilous attacks** are flagged with a red kanji "danger" mark and a sound cue shortly before they land. They cannot be deflected. Counters: **Mikiri counter** (dodge toward the enemy in the last moment of a *thrust* → huge posture damage), jump over *sweeps*, or just get out of the way. *Grabs* must be dodged.
- If your posture maxes out you are staggered for a few seconds and take extra damage.
- Boss health is split in segments; a deathblow removes one segment (a boss with 2 segments dies after the second deathblow, and the second phase is faster/more aggressive).

## Our mapping (existing systems in italics)
| Sekiro | Robot Rumble |
|---|---|
| Attack | LMB light combo (existing), F heavy |
| Block | hold RMB (*State.Combat.Blocking*, `UAAACombatComponent::SetBlocking`) |
| Deflect | tap RMB: the existing parry window (*State.Combat.Parrying*, `LastParryWindowStart`, `UAAACombatSettings` parry seconds). Must now produce: deflect montage on defender, spark VFX + clang at contact, big **poise damage** to the attacker, attacker recoil montage |
| Posture | the existing **Poise** attribute (`UAAAAttributeSet::Poise/MaxPoise/IncomingPoiseDamage`). UI shows posture = `1 - Poise/MaxPoise` (fills up as poise is lost). Poise regenerates when not hit (existing regen). Broken poise = `State.Combat.Staggered` |
| Deathblow | boss is `Staggered` or its health segment is empty AND player presses attack in range → deathblow (cinematic hit, removes one health segment) |
| Mikiri | dodge pressed during the last 0.3 s before a *Thrust* perilous attack lands and the dodge is toward the boss (dot(dodge dir, to-boss) > 0.3): boss gets heavy poise damage and is staggered |
| Perilous warning | red "danger" glyph drawn above the boss (canvas, no UMG) + audio cue slot, 0.6 s before impact |

## Game design for the single boss
- ONE boss (Greystone, scaled 1.2x) replaces the 3 enemies. `ARRGameMode::EnemyCount`=1, boss class `ARRBoss`.
- **2 health bars** (segments). Segment max health 260. Damage empties the current segment; at 0 the boss is forced into `Staggered` (deathblow-able) for 6 s. Deathblow removes the segment; remaining segment count -1. Phase 2 starts when segment 1 is gone (boss restores 40% of segment 2? no: segment 2 is full), attack cooldown x0.7, extra attacks unlocked, `ChaseSpeed` x1.15.
- Boss posture (Poise) max 120, regen delayed 2.5 s after being hit; broken posture = staggered 4 s, then reset to 0 posture.
- Boss attack set (Paragon Greystone sequences in `/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/`): `Attack_A_Fast`, `Attack_A_Med`, `Attack_B_Fast`, `Attack_B_Med`, `Attack_C_Fast`, `Attack_C_Med`, `Attack_D_Med`, `Attack_D_Slow` (perilous sweep), `Attack_A_Slow` (perilous thrust, windup), `Ability_E` / `Ability_Q` (perilous special -> cinematic), `Attack_RMB`. Plus `Deflect_InPlace`, `HitReact_*` (retargeted already as `/Game/RR/Greystone/AM_GS_*` montages).
- Special attack ("Crushing Kick"): perilous, flagged `bCinematic`. When it connects/gets blocked: the cinematic described in P12 plays.

## Visual feedback
- **No player health bar.** Getting hit shows a Resident-Evil-8 style screen effect: blood-red edge vignette that flashes per hit (intensity ~ damage/maxHealth), plus a persistent pulsing low-health vignette and darkening the lower the health, plus directional hit wedge. Drawn with Canvas only (existing RRHUD style), no UMG.
- Boss HUD (canvas): name at bottom-center like Sekiro, two health segments, posture bar growing from the centre outward (orange→red), perilous glyph.
- Player posture bar: a thin bar at screen bottom-centre (the only bar allowed besides stamina).

## Art/animation sources (what the user can download)
See `Docs/FREE_ANIM_SOURCES.md`.
