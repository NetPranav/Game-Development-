# Project Overview & Master Architecture

> **AAA Multiplayer Action RPG** built directly on top of Epic Games' **Game Animation Sample (GAS)** and **Mover 2.0** framework, utilizing the **Gameplay Ability System (GAS)**, Lyra-style modular C++ composition, data-driven Primary Data Assets, and delta-replicated systems.

---

## 1. Architectural Philosophy: "Extend, Wrap, and Compose"

This project adheres to a strict enterprise architectural principle:
- **Epic's Systems are Engine Code:** Epic's locomotion, Motion Matching, Pose Search Databases, Choosers, Trajectory generation, and Control Rig are treated as core engine infrastructure. We never hack, overwrite, or mutate Epic's core assets.
- **C++ Handles Logic & Authority:** Core gameplay logic, state machines, networking, inventory algorithms, and combat execution live entirely in modular C++.
- **Data Assets Govern Configuration:** All tunable values, stats, input mappings, weapon definitions, and asset references are stored in `UPrimaryDataAsset` derivations (`UAAACharacterSettings`, `UAAAWeaponSettings`, `UAAAItemDefinition`, etc.). No gameplay numbers are hardcoded in C++ or Blueprints.
- **Blueprints Handle Presentation & Visuals:** Blueprints are strictly leaf nodes (`BP_PlayerCharacter`, `WBP_GameMenu`, anim montages) responsible for visual styling, particle assignment, and UI layouts.
- **Component-Driven Composition:** Features are isolated in discrete `UActorComponent` classes rather than bloated monolithic character classes. Inter-component and inter-actor communication relies on native interfaces (`IAAAInteractable`, `IAAADamageable`, `IAAATeamAgent`, `IAAAItemInstanceInterface`).

---

## 2. Multi-Module C++ Architecture

The C++ codebase is partitioned into **13 isolated modules** with strict dependency layers:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                            GameAnimationSample                              │
└──────────────────────────────────────┬──────────────────────────────────────┘
                                       │
       ┌───────────────────────────────┴───────────────────────────────┐
       ▼                                                               ▼
┌──────────────┐                                                ┌──────────────┐
│   AAAGame    │ (GameMode, GameState, PlayerController, HUD)   │    AAAAI     │ (NPCs, Bosses, StateTrees)
└──────┬───────┘                                                └──────┬───────┘
       │                                                               │
       ▼                                                               ▼
┌──────────────┐   ┌──────────────┐   ┌──────────────┐          ┌──────────────┐
│ AAACharacter │◄──┤  AAACombat   │◄──┤  AAAAbility  │          │AAAInteraction│
└──────┬───────┘   └──────┬───────┘   └──────┬───────┘          └──────┬───────┘
       │                  │                  │                         │
       ▼                  ▼                  ▼                         ▼
┌──────────────┐   ┌──────────────┐   ┌──────────────┐          ┌──────────────┐
│ AAAInventory │◄──┤   AAAItems   │   │ AAAAnimation │          │   AAAWorld   │
└──────┬───────┘   └──────────────┘   └──────────────┘          └──────────────┘
       │
       ▼
┌──────────────┐   ┌──────────────┐
│  AAANetwork  │   │    AAAUI     │
└──────┬───────┘   └──────────────┘
       │
       ▼
┌──────────────┐
│   AAACore    │ (Gameplay Tags, Core Interfaces, Logging Macros)
└──────────────┘
```

### Module Responsibilities & Status

| Module | Core Responsibilities | Implementation Status |
| :--- | :--- | :--- |
| **`AAACore`** | Native Gameplay Tags (`FAAAGameplayTags`), 8 core Interfaces, Debug Logger macros. | 🟢 Complete & Verified |
| **`AAAGame`** | Engine framework layer: `AAAAAGameModeBase`, `AAAAAGameState`, `AAAAAPlayerState`, `AAAAAPlayerController`, `AAAAAHUD`, `UAAAAGameInstance`. Input routing and UI mode switching. | 🟢 Complete & Verified |
| **`AAACharacter`** | Composition Root (`AAAAAPlayerPawn`), `UAAAAttributeComponent`, `UAAATeamComponent`, `UAAAReplicationComponent`, `UAAACharacterSettings`. | 🟢 Complete & Verified |
| **`AAAAbility`** | GAS Integration: `UAAAAbilitySystemComponent`, `UAAAAttributeSet`, `UAAADamageExecutionCalc`, `UAAAGameplayAbility_Melee`, `UAAAGameplayAbility_Dodge`, `UAAAGameplayCueManager`. | 🟢 Complete & Verified |
| **`AAACombat`** | Combat Stance State Machine (`UAAACombatComponent`, `EAAACombatStance`), dynamic socket re-parenting, continuous collision sweep actor (`AAAAWeaponActor`), auto-sheath timer. | 🟢 Complete & Verified |
| **`AAAAnimation`** | Frame-precise montage notifies: `UAnimNotifyState_WeaponTrace` (continuous collision window), `UAnimNotify_AttachWeapon` (socket reparenting during equip/sheath). | 🟢 Complete & Verified |
| **`AAAInventory`** | Enterprise inventory container: `UAAAInventoryComponent` (FastArraySerializer delta replication), `UAAAEquipmentComponent` (typed slot manager). | 🟢 Complete & Verified |
| **`AAAItems`** | Master item architecture: `UAAAItemDefinition`, `UAAAItemInstance`, setting data assets (`AAAArmorSettings`, `AAAConsumableSettings`, etc.), type enums. | 🟢 Complete & Verified |
| **`AAAInteraction`** | World interaction foundation: `UAAAInteractionComponent`, `TryInteract()` trigger, delegate broadcasting. | 🟡 Framework Complete (Trace sweep pending) |
| **`AAAAI`** | Combat test dummy (`AAAAATestEnemy`), data assets (`AAANPCSettings`, `AAADragonSettings`). | 🟡 Combat Dummy Complete (StateTree / BT pending) |
| **`AAAUI`** | UI base classes and widget wrappers for asymmetric menu and HUD elements. | 🟡 Foundation Complete (UMG Blueprint binding pending) |
| **`AAAWorld`** | World partition management, zone streaming, procedural encounters. | 🔴 Scaffolded (Phase 11) |
| **`AAANetwork`** | Dedicated server optimization, bandwidth throttling, connection migration. | 🔴 Scaffolded (Phase 12) |

---

## 2.1. C++ Symbol & Naming Convention Authority

To prevent symbol desync across development sessions, this project follows a strict prefix convention based on Unreal Engine rules:

| Category | Prefix Rule | C++ Class Example | Header File Name | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **Actor Framework** | Engine `A` + Project `AAA` + Class (`AAAAA...`) | `AAAAAPlayerPawn`<br>`AAAAAPlayerController`<br>`AAAAAGameModeBase`<br>`AAAAAHUD`<br>`AAAAAGameState`<br>`AAAAAPlayerState`<br>`AAAAATestEnemy` | `AAAAPlayerPawn.h`<br>`AAAAPlayerController.h`<br>`AAAAGameModeBase.h`<br>`AAAAHUD.h`<br>`AAAAGameState.h`<br>`AAAAPlayerState.h`<br>`AAAATestEnemy.h` | 5 A's in class names due to Actor class name prefixing; 4 A's in filename. |
| **Actor Entities** | Engine `A` + Project `AAA` (`AAAA...`) | `AAAAWeaponActor` | `AAAAWeaponActor.h` | 4 A's total. |
| **Non-Actor UObjects** | Engine `U` + Project `AAAA` (`UAAAA...`) | `UAAAAGameInstance` | `AAAAGameInstance.h` | Extends `UGameInstance`. |
| **Components & Data** | Engine `U` + Project `AAA` (`UAAA...`) | `UAAACombatComponent`<br>`UAAAInventoryComponent`<br>`UAAAEquipmentComponent`<br>`UAAAAttributeComponent`<br>`UAAAAttributeSet`<br>`UAAAAbilitySystemComponent` | `AAACombatComponent.h`<br>`AAAInventoryComponent.h`<br>`AAAEquipmentComponent.h`<br>`AAAAttributeComponent.h`<br>`AAAAttributeSet.h`<br>`AAAAbilitySystemComponent.h` | 3 A's total for standard components and sets. |
| **Primary Data Assets** | Engine `U` + Project `AAA` (`UAAA...`) | `UAAACharacterSettings`<br>`UAAAWeaponSettings`<br>`UAAAItemDefinition` | `AAACharacterSettings.h`<br>`AAAWeaponSettings.h`<br>`AAAItemDefinition.h` | Data-driven configuration objects. |

---

## 2.2. Critical Security Governance: Automation & Remote APIs

- **Editor & Development Targets ONLY:**
  - Tools such as the **Remote Control API** (`localhost:30010`) and **Python Editor Script Plugin** allow external processes to execute arbitrary Python scripts and manipulate `UObject` reflection.
  - These plugins and endpoints must **NEVER** be packaged into `Shipping` or `Test` builds.
  - In `GameAnimationSample.uproject` and build targets, automation tools must always specify `"TargetAllowList": ["Editor"]` or be gated behind `#if WITH_EDITOR`.
  - An open, unauthenticated Remote Control endpoint on a shipped multiplayer client or dedicated server represents an immediate Remote Code Execution (RCE) vector.

## 3. Core Systems & Technical Implementation Details

### 3.1. Composition Root & Pawn Architecture (`AAACharacter`)
The playable character uses `AAAAPlayerPawn` as its composition root:
- **Base Class APawn:** Inherits directly from `APawn` (not `ACharacter`) to guarantee 100% native compatibility with Epic's Mover plugin (`SandboxCharacter_Mover`).
- **Lyra-Style GAS Separation:** The `UAAAAbilitySystemComponent` (ASC) and `UAAAAttributeSet` are **not owned by the Pawn**. They live on `AAAAPlayerState` so that player attributes, active effects, and granted abilities survive character death and respawn.
- **Actor Info Initialization:** On server possession (`PossessedBy`) and client replication (`OnRep_PlayerState`), the pawn executes `InitAbilityActorInfoFromPlayerState()`, binding the PlayerState as the Owner Actor and the Pawn as the Avatar Actor (`InitAbilityActorInfo(PS, this)`).
- **Component Composition:**
  - `UAAAAttributeComponent`: Bridges GAS attributes to Blueprints, handles health/stamina clamping, and broadcasts `OnDeath`.
  - `UAAACombatComponent`: Controls weapon actors, holster/draw sockets, combat timeouts, and stance tags.
  - `UAAAInventoryComponent`: Replicated item container with FastArray delta serialization.
  - `UAAAEquipmentComponent`: Manages gear equipped in defined body/weapon slots.
  - `UAAAInteractionComponent`: Handles interaction traces and triggering `IAAAInteractable`.
  - `UAAAAbilityInputBindingComponent`: Binds Enhanced Input Actions directly to GAS ability specs.

### 3.2. Modern Combat & Stance State Machine (`AAACombat` + `AAAAbility`)
Combat delivers responsive, Souls/Sekiro-inspired responsiveness:
- **Stance State Machine (`EAAACombatStance`):**
  - `Unarmed`: No weapon equipped or drawn. Character uses standard locomotion.
  - `Armed_Holstered`: Weapon equipped and attached to `spine_03` holster socket.
  - `Armed_Drawn`: Weapon drawn and attached to `hand_r` socket. Triggers `State.Combat.Drawn` tag.
- **Auto-Sheath Countdown:** When in `Armed_Drawn`, any attack or combat action refreshes an 8.0-second timer (`RefreshCombatTimeout()`). If no combat actions occur for 8 seconds, the character automatically plays the sheath montage and transitions back to `Armed_Holstered`.
- **Continuous Sweep Collision (`AAAAWeaponActor`):**
  - Hit detection does not rely on imprecise tick traces or simple collision boxes.
  - `StartWeaponTrace()` initializes continuous sweeps between `PreviousSocketLocation` and current socket locations on `TraceStart` and `TraceEnd`.
  - Maintains `HitActorsThisSwing` array throughout the montage window to prevent multi-hitting the same target in a single swing.
  - Trace duration is driven precisely by `UAnimNotifyState_WeaponTrace` placed directly on combat animation montages.
- **Dynamic Melee Combos (`UAAAGameplayAbility_Melee`):**
  - Sequences through attack montages using combo indices defined in `UAAAWeaponSettings`.
  - If a player attacks while holstered, the ability automatically draws the weapon and executes the initial attack.
  - Hits trigger `UAAADamageExecutionCalc`, which calculates base damage, attack power scaling, and target armor mitigation before deducting from the target's `UAAAAttributeSet`.
  - Executes `GameplayCue.Combat.Hit.Melee` for synchronized visual/sound effects without RPC overhead.
- **Sekiro-Style Directional Step-Dodge (`UAAAGameplayAbility_Dodge`):**
  - Evaluates current movement input vector (`Acceleration` / controller input) to determine direction (Forward, Backward, Left, Right).
  - Deducts 15 Stamina and applies an instant grounded movement impulse (1200 units/sec) or root motion montage.
  - Grants the native tag `State.Combat.Invulnerable` during the dodge window, making the character immune to incoming hit collision.

### 3.3. Enterprise Item & Inventory Architecture (`AAAInventory` + `AAAItems`)
Built to support massive multiplayer inventories with minimal network bandwidth:
- **Fast Array Delta Replication:** `UAAAInventoryComponent` uses Unreal Engine's `FFastArraySerializer` (`FAAAInventoryList` / `FAAAInventoryEntry`). When an item is added, moved, or updated, only the modified entry is serialized and sent to clients, preventing full-array bandwidth spikes.
- **Instanced Items (`UAAAItemInstance`):** Derived from `UObject`, replicated via `ReplicateSubobjects`. Stores runtime dynamic data: unique GUID, stack count, item level, durability, gem slots, and custom tags.
- **Data-Driven Definitions (`UAAAItemDefinition`):** `UPrimaryDataAsset` holding static metadata: display name, description, category, rarity, icon, weight, and max stack count. References specific settings data assets:
  - `UAAAWeaponSettings`: Meshes, damage, attack speed, combo montages, sockets.
  - `UAAAArmorSettings`: Defense, elemental resistances, mesh overlays.
  - `UAAAConsumableSettings`: Gameplay effects granted upon consumption (potions, food).
  - `UAAACraftingMaterialSettings`, `UAAAQuestItemSettings`, `UAAAResourceSettings`.
- **Typed Equipment Slots (`UAAAEquipmentComponent`):** Manages slots (`MainHand`, `OffHand`, `Head`, `Chest`, `Hands`, `Legs`, `Feet`, `Ring1`, `Ring2`, `Amulet`, `DragonSaddle`, etc.). Enforces level/class requirements and broadcasts strongly typed delegates (`FAAAOnItemEquipped`, `FAAAOnEquipmentChanged`).

### 3.4. Input Routing & Asymmetric UI System (`AAAGame` + `AAAUI`)
- **Deterministic Input Hierarchy:**
  - `BP_AAAPlayerController` binds `IMC_AAA_Main` with Priority 1, guaranteeing UI toggle hotkeys (`Tab` for Game Menu, `M` for Map) cannot be suppressed by character locomotion or ability inputs.
  - `bExecuteWhenPaused = true` ensures menus can be toggled even when world dilation is zero.
  - Toggling menu calls `FInputModeGameAndUI` and exposes the mouse cursor; closing returns to `FInputModeGameOnly`.
- **Asymmetric RE2-Style UI:**
  - Managed by `AAAAAHUD::ToggleGameMenu()`.
  - Spawns `WBP_GameMenu`, featuring a 50/50 split layout (Inventory Grid on one side, Equipment/Inspect/Scratchpad on the other).
  - UMG animations configured to `Play When Paused` for seamless transitions.

### 3.5. Progression & Attribute Boundary (`UAAAAttributeSet` vs `AAAAAPlayerState`)
To avoid redundancy and maximize performance, responsibility between GAS attributes and PlayerState properties is strictly partitioned:
- **GAS AttributeSet (`UAAAAttributeSet`):** Owns attributes that directly feed into combat math, ability costs, and damage scaling formulas:
  - `Health`, `MaxHealth`, `Stamina`, `MaxStamina`, `Armor`, `Damage` (meta-attribute), `MovementSpeed`.
  - `Level` and `Experience`: Modeled as `FGameplayAttributeData` because level directly modulates ability magnitude curves and base stat multipliers via GAS calculation execution rules.
- **PlayerState (`AAAAAPlayerState`):** Owns non-combat meta-progression that does not require GAS execution curves:
  - `Gold`, `Guild/Party`, `Playtime`, `Achievement Progress`.
  - Replicated using standard `UPROPERTY(Replicated)` with minimal network overhead.

### 3.6. SaveGame & Persistence Architecture (`UAAASaveGame` — Phase 7.6)
Built on Unreal Engine's `USaveGame` and `SaveGame` UPROPERTY tags:
- **Item & Inventory Serialization:** Serializes `UAAAInventoryComponent`'s `FAAAInventoryList` (including dynamic `UAAAItemInstance` GUIDs, stack counts, durability, and custom tags).
- **Equipment Slot Serialization:** Saves equipped item GUIDs mapped to `EAAAEquipmentSlot`.
- **Player & Attribute State:** Persists player progression (`Level`, `Experience`, `Gold`), learned abilities, and world transform.
- **Async Disk I/O:** Serialized via `UGameplayStatics::AsyncSaveGameToSlot` to guarantee stutter-free saving during gameplay.

### 3.7. Multi-Client Dedicated Server Replication Standards
"Server-Authoritative" is an active engineering standard, not an assumption:
- **Mandatory Multi-Client Testing:** Validation occurs in PIE with 2–3 clients and `Run Dedicated Server` enabled before certifying any combat or inventory phase.
- **Key Multiplayer Failure Modes Guarded:**
  - Ability activation prediction/misprediction on melee combo chains.
  - Weapon draw/sheath socket reparenting (`hand_r` vs `spine_03`) replicating to simulated proxies.
  - Inventory `FastArraySerializer` delta updates arriving cleanly at autonomous clients.

---

## 4. Current Implementation Status Matrix

```
[System]                 [C++ Logic]      [Data Assets]     [Editor/BP Setup]  [Multiplayer Verified]
Core Modules             🟢 100%          🟢 100%           🟢 100%            🟢 Yes
GAS Architecture         🟢 100%          🟢 100%           🟢 100%            🟢 Yes
Combat Stance FSM        🟢 100%          🟢 100%           🟢 100%            🟡 Dedicated Server Test Active
Weapon Tracing / Hit     🟢 100% (Fallback)🟢 100%          🟡 80% (Montages)  🟡 In Progress
Melee Combo Ability      🟢 100%          🟢 100%           🟡 80% (Montages)  🟡 Prediction Test Active
Directional Step-Dodge   🟢 100%          🟢 100%           🟢 100%            🟢 Yes
Inventory Fast-Array     🟢 100%          🟢 100%           🟡 50% (UI Bind)   🟡 Delta Rep Test Active
Equipment Manager        🟢 100%          🟢 100%           🟡 50% (UI Bind)   🟡 In Progress
SaveGame System (P7.6)   🟡 40% (Contract)🟢 100%           🔴 0%              🔴 Scheduled
Asymmetric Game Menu     🟢 100%          🟢 100%           🟡 70% (Visuals)   🟢 Yes (Toggle)
Interaction System       🟢 60% (Stub)    🟢 100%           🔴 20%             🔴 No
Enemy AI Dummy           🟢 100% (Dummy)  🟢 100%           🟢 100%            🟢 Yes (Ragdoll)
AI StateTrees / BT       🔴 0%            🟡 50%            🔴 0%              🔴 No
Mount / Dragon Riding    🔴 10% (Tags)    🟡 50%            🔴 0%              🔴 No
```

---

## 5. Downstream Execution Pipelines (What Unlocks What)

1. **Combat Montages & Sockets (Phase 7 Finalization)**  
   $\rightarrow$ Wire `TraceStart`/`TraceEnd` sockets on weapon mesh and place `AnimNotifyState_WeaponTrace` on attack montages.  
   $\rightarrow$ **Unlocks:** Verified live melee hit registration against `AAAATestEnemy`, triggering damage execution and ragdoll.

2. **Animation Layering (Choosers / Linked Anim Layers)**  
   $\rightarrow$ Inject combat locomotion into Epic's `SandboxCharacter_Mover_ABP` when in `Armed_Drawn` stance.  
   $\rightarrow$ **Unlocks:** Full combat mobility (strafing, drawn sprint, seamless weapon combat stance transitions).

3. **Inventory UI Binding (`WBP_GameMenu` $\leftrightarrow$ `UAAAInventoryComponent`)**  
   $\rightarrow$ Bind UMG item tiles to `FAAAOnItemAdded`, `FAAAOnItemRemoved`, and `FAAAOnStackChanged`.  
   $\rightarrow$ **Unlocks:** Real-time visual inventory management, drag-and-drop equipping, and consumption.

4. **World Interaction Sweeps (`UAAAInteractionComponent`)**  
   $\rightarrow$ Implement line/sphere trace forward from camera/pawn to detect `IAAAInteractable` actors and display HUD prompt.  
   $\rightarrow$ **Unlocks:** World loot pickup (chests, dropped weapons), door interaction, and NPC dialogue triggers.

5. **AI StateTree & Perception (`AAAAI`)**  
   $\rightarrow$ Implement `AAAIController`, StateTree behaviors for combat, agro ranges, and attack abilities.  
   $\rightarrow$ **Unlocks:** Dynamic hostile encounters, patrol routes, and multi-actor melee combat.
