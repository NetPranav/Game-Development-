# IMC_AAA_Main

**Asset Path:** `/Game/AAA/Input/Contexts/IMC_AAA_Main`
**C++ Base Class:** `UInputMappingContext`
**Origin:** Custom Data Asset

## Summary
The master input mapping context. It acts as the bridge between raw hardware inputs (Keyboard 'Tab', 'M') and logical Input Actions (`IA_ToggleMenu`).

## Upstream Connections (What uses this?)
- **BP_AAAPlayerController:** Assigned as the `Global UI Mapping Context`. Applied globally with Priority 1 to ensure UI keys are never overridden.
- **DA_PlayerCharacterSettings:** Assigned to ensure character-specific inputs (if any are stored here) are granted upon possession.

## Downstream Connections (What does this require?)
- **IA_ToggleMenu:** Assigned inside the context map, linked to specific keys.
- **IA_Move / IA_Jump / etc:** (If configured here) Links character actions to specific keys.
