# BP_AAAGameMode

**Asset Path:** `/Game/AAA/Blueprints/GameFramework/BP_AAAGameMode`
**C++ Base Class:** `AAAAGameModeBase`
**Origin:** Custom Blueprint inherited from our AAA module.

## Summary
The master authority of the game session. It defines the rules of the level and dictates exactly which classes (Pawn, Controller, HUD) are spawned when the game starts.

## Upstream Connections (What uses this?)
- **L_Developer:** (or any map) Assigned in the `World Settings` as the Default GameMode. Alternatively, configured in `Project Settings -> Maps & Modes`.

## Downstream Connections (What does this require?)
- **PlayerControllerClass:** Set to `BP_AAAPlayerController`.
- **DefaultPawnClass:** Set to `BP_PlayerCharacter`.
- **HUDClass:** Set to `BP_AAAHUD`.
- **PlayerStateClass:** Set to `BP_AAAPlayerState`.
