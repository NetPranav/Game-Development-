# DA_PlayerCharacterSettings

**Asset Path:** `/Game/AAA/Characters/DA_PlayerCharacterSettings`
**C++ Base Class:** `UAAACharacterSettings`
**Origin:** Custom Data Asset

## Summary
The central configuration hub for the Player Character. Instead of hardcoding max health, starting items, or input contexts directly into the Character Blueprint, they are defined here. This allows multiple characters to share the same Blueprint but behave entirely differently based on which Data Asset is assigned.

## Upstream Connections (What uses this?)
- **BP_PlayerCharacter:** Assigned in its `Class Defaults`. If this connection breaks, the character defaults to empty variables and fails to possess inputs.

## Downstream Connections (What does this require?)
- **IMC_Sandbox (or similar):** Contains the array of Enhanced Input Mapping Contexts that should be granted to the character for basic movement and combat.
- **Starting Equipment/Items:** (Future Implementation) Will dictate what the character spawns with.
