# AAA Multiplayer Action RPG — Project Timeline & Milestone Ledger

> Chronological ledger of major project milestones, architectural breakthroughs, bug resolutions, and system implementations.

---

## Completed Milestones

### Phase 1–3: Project Reset, Multi-Module Architecture & Content Governance (July 2026)
- **Strategic Reset to Epic Game Animation Sample:**
  - Adopted the "Extend, Wrap, and Compose" architectural philosophy.
  - Epic's locomotion, Mover 2.0, Motion Matching, and Pose Search databases established as immutable Engine code.
- **13 C++ Modules Built & Linked:**
  - Architected modular domain layout: `AAACore`, `AAAGame`, `AAACharacter`, `AAAAbility`, `AAAAnimation`, `AAACombat`, `AAAInventory`, `AAAItems`, `AAAInteraction`, `AAAAI`, `AAANetwork`, `AAAUI`, `AAAWorld`, and `GameAnimationSample`.
  - Configured unidirectional module dependencies in `*.Build.cs` to eliminate circular coupling.
- **Native Gameplay Tags & Core Interfaces:**
  - Implemented `FAAAGameplayTags` singleton struct for compile-time safe gameplay tags.
  - Implemented 8 core engine interfaces (`IAAAInteractable`, `IAAADamageable`, `IAAAMountable`, `IAAADragonRider`, `IAAALootSource`, `IAAAQuestTarget`, `IAAASaveable`, `IAAATeamAgent`).
- **Content Governance:**
  - Established standard 31-folder structure under `Content/AAA/` with strict asset naming conventions (`BP_`, `WBP_`, `DA_`, `IMC_`).

---

### Phase 4: Native Gameplay Foundation & Composition Root (July 2026)
- **Lyra-Style Pawn Composition Root (`AAAAPlayerPawn`):**
  - Inherited from `APawn` (not `ACharacter`) to maintain 100% native compatibility with Epic's Mover 2.0 plugin.
  - Reparented Epic's `SandboxCharacter_Mover` to `AAAAPlayerPawn`.
  - Implemented `IAbilitySystemInterface` with weak-pointer caching to PlayerState ASC.
- **Lyra Architectural Separation:**
  - `UAAAAbilitySystemComponent` (ASC) and `UAAAAttributeSet` hosted on `AAAAPlayerState` to persist across character death and respawn.
  - Implemented `InitAbilityActorInfoFromPlayerState()` called on server possession and client replication.
- **Modular Component Composition:**
  - Added `UAAAAttributeComponent`, `UAAATeamComponent`, `UAAAReplicationComponent`, `UAAAInventoryComponent`, `UAAAEquipmentComponent`, `UAAAInteractionComponent`, and `UAAACombatComponent` to the pawn.

---

### Phase 5: Architecture Stabilization, Memory Fixes & Live Link (July–August 2026)
- **Memory Crash Loop Resolution:**
  - Diagnosed and resolved 100% memory crash loop on editor startup caused by rogue UI rendering commands.
  - Stabilized Editor memory allocations and garbage collection settings.
- **Blueprint Corruption & UHT Desync Fix:**
  - Implemented `PreDefault` loading phase in `GameAnimationSample.uproject` to stop Blueprint reflection desync ("derives from invalid class").
  - Forced UHT cache rebuilds to safely expose C++ composition roots to the Editor.
- **Live Link Facial Mocap Restoration:**
  - Permanently restored Live Link facial mocap compatibility and debug visualizers through Blueprint reconstruction and the "Auto-Pin" trick.

---

### Phase 6: Game Framework, Priority Input & Asymmetric UI (August 2026)
- **Native Game Framework:**
  - Implemented `AAAAGameModeBase`, `AAAAGameState`, `AAAAPlayerState`, `AAAAPlayerController`, `AAAAHUD`, and `AAAAGameInstance`.
- **Priority Input Routing:**
  - Configured `BP_AAAPlayerController` to inject `IMC_AAA_Main` with Priority 1, superseding character-level mappings.
  - Enabled `bExecuteWhenPaused = true` on global hotkeys (`Tab` for menu, `M` for map) to ensure responsiveness during zero world dilation.
  - Built seamless input mode switching between `FInputModeGameOnly` and `FInputModeGameAndUI`.
- **RE2-Style Asymmetric UI (`WBP_GameMenu`):**
  - Engineered master container with 50/50 split (Inventory Grid on left, Equipment/Inspector on right).
  - Managed via `AAAAAHUD::ToggleGameMenu()` with UMG `Play When Paused` transitions.

---

### Phase 6.5: Master Item & Enterprise Inventory Architecture (August 2026)
- **Fast Array Delta Network Replication (`UAAAInventoryComponent`):**
  - Implemented Lyra-pattern `FFastArraySerializer` (`FAAAInventoryList` / `FAAAInventoryEntry`) to serialize only modified slots rather than full arrays.
  - Enabled dynamic subobject replication (`UAAAItemInstance`) for mutable runtime item data (durability, stacks, quality, level).
  - Built stack splitting, merging, category searching, and dynamic multicast delegates (`FAAAOnItemAdded`, `FAAAOnItemRemoved`, etc.).
- **Typed Equipment Manager (`UAAAEquipmentComponent`):**
  - Implemented strongly typed equipment slots (`MainHand`, `OffHand`, `Head`, `Chest`, `Hands`, `Legs`, `Feet`, `Rings`, `DragonSaddle`).
- **Data-Driven Item Archetypes (`AAAItems`):**
  - Created `UAAAItemDefinition` (PrimaryDataAsset) and specific settings assets (`AAAArmorSettings`, `AAAConsumableSettings`, `AAACraftingMaterialSettings`, `AAAItemWeaponSettings`, `AAAQuestItemSettings`, `AAAResourceSettings`).

---

### Phase 7: Modern AAA Combat Engine & Sekiro Dodge (August–September 2026)
- **Dynamic Combat Stance State Machine (`UAAACombatComponent`):**
  - Implemented `EAAACombatStance` (`Unarmed`, `Armed_Holstered`, `Armed_Drawn`).
  - Added weapon actor spawning (`AAAAWeaponActor`) and dynamic socket reparenting (`hand_r` vs `spine_03`).
  - Implemented auto-sheath timer with 8.0-second combat timeout (`RefreshCombatTimeout()`).
  - Synchronized native tags: `State.Combat.Drawn`, `State.Combat.Holstered`, `State.Combat.Unarmed`.
- **Continuous Frame-to-Frame Collision Sweep (`AAAAWeaponActor`):**
  - Sweeps between `TraceStart` and `TraceEnd` sockets from previous frame to current frame.
  - Prevents multi-hitting via `HitActorsThisSwing` tracking.
  - Governed by `UAnimNotifyState_WeaponTrace` on attack montages.
- **Sekiro-Style Directional Step-Dodge (`UAAAGameplayAbility_Dodge`):**
  - Computes movement input direction vector and applies rapid grounded impulse (1200 units/s).
  - Deducts 15 stamina and applies `State.Combat.Invulnerable` i-frames during the dodge window.
- **Dynamic Combo Melee Ability (`UAAAGameplayAbility_Melee`):**
  - Cycles through attack montages using combo indices from `UAAAWeaponSettings`.
  - Auto-draws weapon if attacking while holstered.
  - Executes SetByCaller damage via `UAAADamageExecutionCalc` with `Data.Damage`.
  - Fires `GameplayCue.Combat.Hit.Melee` for synchronized audio/visual effects.
- **AI Combat Dummy (`AAAATestEnemy`):**
  - Derived from `AAAAPlayerPawn` with hard-referenced ASC/AttributeSet and ragdoll on death.
- **MetaHuman Kristofer Pipeline:**
  - Integrated Kristofer preset MetaHuman into `Content/MetaHumans/Kristofer/`.
  - Established asset folder hierarchies under `Content/AAA/Characters/Hero/`.

---

## Active Milestone: Phase 8 — First Playable Combat Slice & Dedicated Server Replication
- **Goal:** In-Editor live validation of the complete combat loop and multi-client replication.
- **Status:** 🟡 Active.
- **Action Items:**
  1. Add `TraceStart` and `TraceEnd` sockets to test weapon static mesh (bounding-box fallback + red on-screen warning active).
  2. Place `AnimNotifyState_WeaponTrace` on attack montages.
  3. Validate hit detection, damage application, and ragdoll death on `AAAAATestEnemy`.
  4. **Multi-Client Verification:** Run PIE with 2 players + Dedicated Server to test melee prediction, socket reparenting replication (`hand_r` vs `spine_03`), and FastArray delta updates.

---

## Next Milestone Targets

1. **Phase 7.5: Inventory UI Data Binding**
   - Bind `WBP_GameMenu` UMG tiles to `UAAAInventoryComponent` delegates for interactive inventory management.
2. **Phase 7.6: SaveGame & Persistence Architecture (`UAAASaveGame`)**
   - Implement `USaveGame` subclass serializing inventory `SaveGame` properties, equipment slot mappings, player progression (`Level`, `Experience`, `Gold`), and transform.
3. **Phase 9: Animation Choosers & Armed Locomotion Layers**
   - Inject combat pose search databases into Epic's `SandboxCharacter_Mover_ABP` Chooser table for `Armed_Drawn` locomotion.
4. **Phase 10: World Interaction Sweeps & World Loot**
   - Implement line/sphere trace in `UAAAInteractionComponent` and create world pickup actors.
