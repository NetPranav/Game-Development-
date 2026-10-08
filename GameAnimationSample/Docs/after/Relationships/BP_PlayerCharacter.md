# BP_PlayerCharacter

**Asset Path:** `/Game/AAA/Characters/BP_PlayerCharacter`
**C++ Base Class:** `AAAAAPlayerPawn`
**Origin:** Adapted from Epic's Game Animation Sample (`SandboxCharacter_Mover`)

## Summary
The primary playable character pawn. This Blueprint handles movement, animation, and basic interaction physics. It relies heavily on Data Assets for configuration rather than hardcoding values.

## Upstream Connections (What uses this?)
- **BP_AAAGameMode:** Set as the `Default Pawn Class`. When the game starts, the GameMode spawns this asset.

## Downstream Connections (What does this require?)
- **DA_PlayerCharacterSettings:** Assigned in the `Class Defaults` under the AAA category. This Data Asset injects all the gameplay tags, input contexts, and stats into the character when it spawns. If this is missing or corrupted, the character will not possess properly and inputs will fail.
