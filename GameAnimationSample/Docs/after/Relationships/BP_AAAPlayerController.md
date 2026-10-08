# BP_AAAPlayerController

**Asset Path:** `/Game/AAA/Blueprints/GameFramework/BP_AAAPlayerController`
**C++ Base Class:** `AAAAAPlayerController`
**Origin:** Custom Blueprint inherited from our AAA module.

## Summary
The "brain" of the human player. This persists across character deaths. It handles global input (like pausing the game or opening the inventory) and routes commands to the possessed pawn.

## Upstream Connections (What uses this?)
- **BP_AAAGameMode:** Set as the `Player Controller Class`. The server spawns this for every connecting client.

## Downstream Connections (What does this require?)
- **IA_ToggleMenu:** Assigned in the `AAA | Input` category in Class Defaults. This tells the controller *which* action signifies a menu toggle.
- **IMC_AAA_Main:** Assigned as the `Global UI Mapping Context`. This guarantees that global keys (like Tab or Esc) are always bound at Priority 1, superseding any pawn-specific inputs.
- **BP_AAAHUD:** The controller talks to the HUD to spawn UI widgets.
