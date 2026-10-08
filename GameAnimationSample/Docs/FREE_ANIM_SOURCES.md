# Where to get animations for the boss duel (legal + free where possible)

**Do not rip animations from Sekiro itself** (copyrighted game assets, and the files are not usable outside the game's engine anyway). Use it as *reference*: watch boss fights, copy the timing/pose language, then source or record equivalent moves below.

## Fab / Marketplace sword packs (check the listing: price and "UE5 skeleton" before downloading)
| Pack (Fab) | Why | Notes from the listing |
|---|---|---|
| KnightSword (106 anims) | one-handed sword: spin attack, stab, straight cut, locomotion, transitions | UE4/UE5 mannequin-compatible, IK Retarget ready, weapon mesh not included |
| GreatSword (112 anims) | heavy two-handed moves for the boss (slow, perilous sweeps) | works with UE5 mannequin via retargeting, root-motion and in-place variants |
| Essential Sword & Shield | block/deflect/shield bash, kick-like pushes | UE5 skeleton with retargeted anims (UE 5.3+), includes IK rig/retargeter |
Search terms on Fab: "sword combo animation", "parry block animation", "kick animation", "stagger knockback slide animation", "kneel sword plant". Filter: Free, Animation, UE5.

## Mixamo (free, Adobe account) — best source of single moves
- Download as FBX, *Without skin*, 30 fps, "In place" ON for the stab/slide poses (the system moves the actor itself).
- Search: "Sword And Shield Block", "Sword And Shield Kick", "Sword And Shield Slash", "Standing Block Idle", "Kneeling Idle", "Great Sword Slash", "Stunned", "Hit Reaction", "Dying".
- Import onto the Mixamo skeleton, then retarget to the project skeleton (below).

## Turning a video into an animation (free tiers change often: check the tool's page first)
- **Rokoko Video** (browser, free): upload a clip of a person performing the move → export FBX/BVH, clean up in Rokoko Studio (free) if needed.
- **Plask** (browser): video → mocap → FBX.
- **QuickMagic**: exports FBX with Mixamo/UE skeleton (free trial).
- Kapwing "AI Motion Capture" only renders a preview video; it does not give you a rigged animation, so it is only good for reference.
Tips: film or choose clips with the whole body in frame, static camera, plain background, one person, 30 fps, no cuts. Expect to hand-fix feet sliding and hands.

## Getting any of the above into this project (details in `Docs/ANIMATION_GUIDE.md`, written by prompt P12)
1. Import the FBX with the skeleton it was built for (Mixamo → new skeleton; Fab packs → UE mannequin).
2. Retarget to the robot's skeleton (`UE4_Mannequin_Skeleton`) or the Greystone skeleton using an IK Rig + IK Retargeter (see `Tools/editor_scripts/retarget_greystone.py` for a scripted example).
3. Right-click the animation → *Create → Montage*, put it on **DefaultSlot**.
4. Drop it into a slot: `ARRBoss::Attacks[]` (boss moves), `URRCinematicSequence` fields (cinematic specials), `FRRSpecialMove.PlayerAnim` (hero specials), `DA_PlayerWeapon` (deflect/block).
