# AAA Multiplayer Action RPG — Master Development Roadmap

> A comprehensive, logic-synchronized master roadmap detailing every phase of development: what is complete and how it was implemented, what is currently incomplete, what needs to be built next, and the sequential execution chain (what continues after each phase finishes).

---

## Roadmap Overview & Phase Status Summary

| Phase | Phase Name | Status | Primary Focus |
| :--- | :--- | :--- | :--- |
| **Phase 1** | **C++ Foundation & Multi-Module Architecture** | 🟢 Complete | 13 C++ modules, Native Gameplay Tags, Core Interfaces |
| **Phase 2** | **Project Settings, Enhanced Input & Global Contexts** | 🟢 Complete | `IMC_AAA_Main`, Input priority routing, Mode switching |
| **Phase 3** | **Content Architecture & Standards** | 🟢 Complete | `Content/AAA/` directory structure, asset ownership rules |
| **Phase 4** | **Gameplay Foundation & Composition Root** | 🟢 Complete | `AAAAPlayerPawn` (APawn base), Lyra-style GAS split |
| **Phase 5** | **Gameplay Ability System (GAS) Foundation** | 🟢 Complete | ASC, `AAAAttributeSet`, Death/Respawn, Damage calc |
| **Phase 6** | **Data-Driven Architecture** | 🟢 Complete | Primary Data Assets (`Character`, `Weapon`, `Item`, `Faction`) |
| **Phase 6.5**| **Native Game Framework Engine Layer** | 🟢 Complete | `AAAAAGameModeBase`, `AAAAAPlayerController`, `AAAAAHUD`, `UAAAAGameInstance` |
| **Phase 7** | **Modern Combat Engine & Stance State Machine** | 🟡 85% Complete | Stance FSM, Weapon sweeps, Combo Melee, Sekiro Dodge |
| **Phase 7.5**| **Enterprise Inventory & FastArray Replication** | 🟡 80% Complete | `FFastArraySerializer`, Item Instances, Equipment Slots |
| **Phase 7.6**| **SaveGame & State Persistence Architecture** | 🟡 40% Complete | `UAAASaveGame`, FastArray serialization, PlayerState saving |
| **Phase 8** | **Combat Vertical Slice & Dedicated Server Arena** | 🟡 Active (In Progress) | Multi-client PIE test, `AAAAATestEnemy`, Hit registration, Ragdoll |
| **Phase 9** | **Animation Choosers & Motion Matching Layers** | 🔴 Next Up | Armed locomotion Choosers, Linked Anim Layers on Epic ABP |
| **Phase 10**| **World Interaction & Environmental Loot** | 🔴 Pending | Sweep traces, `IAAAInteractable`, Chests, Doors, HUD Prompts |
| **Phase 11**| **Advanced AI, Perception & StateTrees** | 🔴 Future | `AAAAAIController`, StateTrees, Combat AI behaviors, Patrols |
| **Phase 12**| **Dragon Flight & Mount Framework** | 🔴 Future | Trajectory prediction, Mounting sockets, Flight physics |
| **Phase 13**| **World Partition, Quests & Factions** | 🔴 Future | Seamless streaming, Dialogue trees, Faction reputation |
| **Phase 14**| **Dedicated Server & Multiplayer Hardening** | 🔴 Future | Network bandwidth optimization, RPC validation, Desync guards |

---

## Phase Breakdown: Detailed Logic & Execution Status

### Phase 1: C++ Foundation & Multi-Module Architecture
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **13 C++ Modules Built & Linked:** Partitioned codebase into `AAACore`, `AAAGame`, `AAACharacter`, `AAAAbility`, `AAAAnimation`, `AAACombat`, `AAAInventory`, `AAAItems`, `AAAInteraction`, `AAAAI`, `AAANetwork`, `AAAUI`, `AAAWorld`, plus `GameAnimationSample`.
- **Compile Dependencies Configured:** Wired all `*.Build.cs` files with unidirectional dependencies preventing circular inclusions (refactored `AAADataLibrary` into `AAACore`/`AAAItems`).
- **Target & Engine Registration:** All modules registered in `GameAnimationSample.uproject` and `*.Target.cs` with `PreDefault` loading phase to prevent Blueprint asset corruption on startup.
- **Native Gameplay Tag Registry (`FAAAGameplayTags`):** Implemented native tag singleton in `AAACore` for state tags (`State.Combat.Drawn`, `State.Combat.Invulnerable`, etc.), damage types, and movement tags.
- **Core Interfaces:** Created 8 core interfaces (`IAAAInteractable`, `IAAADamageable`, `IAAAMountable`, `IAAADragonRider`, `IAAALootSource`, `IAAAQuestTarget`, `IAAASaveable`, `IAAATeamAgent`).

#### What Continues After Completion:
- Modules served as the foundation for the Lyra-style Composition Root (`AAACharacter`) and Game Framework classes (`AAAGame`).

---

### Phase 2: Project Settings & Enhanced Input Routing
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **Deterministic Input Hierarchy:** Configured `BP_AAAPlayerController` to inject `IMC_AAA_Main` at Priority 1.
- **bExecuteWhenPaused Support:** Global hotkeys (Tab for Game Menu, M for Map) function with zero game world dilation.
- **Input Mode Switching:** Implemented seamless switching in `AAAAPlayerController` between `FInputModeGameOnly` (camera lock, mouse captured) and `FInputModeGameAndUI` (mouse cursor free, menu navigation).
- **Direct Fallback Keybinds:** Built temporary direct input routes in C++ controller (`R` for Draw/Sheath, `LMB` for Attack, `LeftShift` for Dodge) ensuring zero dependence on Blueprint bindings during initial tests.

#### What Continues After Completion:
- Paved the way for `WBP_GameMenu` UI toggle and GAS Ability Input Binding (`UAAAAbilityInputBindingComponent`).

---

### Phase 3: Project Organization & Content Architecture
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **Strict Folder Isolation:** Created 31 standard directories under `Content/AAA/` (Blueprints, Animation, Characters, Gameplay, Materials, UI, etc.).
- **No Modifying Epic Assets Rule:** Enforced strict isolation—Epic's Game Animation Sample assets (`SandboxCharacter_Mover`, `ABP_SandboxCharacter`) remain untouched, extended only via duplication or child inheritance.
- **Asset Naming Conventions:** Enforced standardized prefixes (`BP_`, `WBP_`, `DA_`, `IMC_`, `IA_`, `GE_`, `GA_`).

#### What Continues After Completion:
- Created clean housing for leaf Blueprints (`BP_PlayerCharacter`, `BP_AAAGameMode`, `WBP_GameMenu`).

---

### Phase 4: Native Gameplay Foundation & Composition Root
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **`AAAAPlayerPawn` (APawn Base):** Inherits from `APawn` rather than `ACharacter` to maintain 100% compatibility with Epic's Mover 2.0 plugin.
- **Lyra-Style Ability Separation:** `UAAAAbilitySystemComponent` (ASC) and `UAAAAttributeSet` are hosted on `AAAAPlayerState`, not the Pawn. Character deaths/respawns do not wipe player attributes or active effects.
- **Actor Info Binding:** Server (`PossessedBy`) and Client (`OnRep_PlayerState`) call `InitAbilityActorInfoFromPlayerState()`, cleanly setting the PlayerState as Owner and the Pawn as Avatar.
- **Modular Component Composition:** Pawn acts strictly as a composition root containing `UAAAAttributeComponent`, `UAAATeamComponent`, `UAAAReplicationComponent`, `UAAAInventoryComponent`, `UAAAEquipmentComponent`, `UAAAInteractionComponent`, and `UAAACombatComponent`.
- **Attribute Component:** Exposes Health and Stamina to Blueprints, handles damage clamping, and broadcasts `OnDeath` dynamic delegate.

#### What Continues After Completion:
- Unlocked GAS Ability execution (`AAAAbility`) and Combat logic (`AAACombat`).

---

### Phase 5: Gameplay Ability System (GAS) Foundation
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **`UAAAAbilitySystemComponent`:** Custom ASC derived from `UAbilitySystemComponent` with tag-based ability blocking and activation checks.
- **`UAAAAttributeSet`:** Replicated attribute set tracking `Health`, `MaxHealth`, `Stamina`, `MaxStamina`, `Mana`, `MaxMana`, `AttackPower`, `Armor`, and `CriticalChance`.
- **Damage Execution Calculation (`UAAADamageExecutionCalc`):** Custom native calculation reading Attacker `AttackPower` and Weapon `BaseDamage` (passed via `Data.Damage` SetByCaller), calculating mitigation through Defender `Armor`, and evaluating `CriticalChance`.
- **Multiplayer Initialization:** Replication setup for attributes and effects using `PreAttributeChange` and `PostGameplayEffectExecute`.

#### What Continues After Completion:
- Enabled creation of concrete combat abilities (`UAAAGameplayAbility_Melee`, `UAAAGameplayAbility_Dodge`).

---

### Phase 6: Data-Driven Gameplay Framework
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **`UAAACharacterSettings`:** Primary Data Asset governing pawn initialization: default input contexts, base attribute overrides, starting abilities, starting items, and team configuration.
- **`UAAAWeaponSettings`:** Primary Data Asset defining weapon meshes (Static/Skeletal), Base Damage, Attack Speed, Critical Multiplier, Hand/Holster socket names, combo attack montages, and weapon category.
- **`UAAAFactionSettings` & `UAAAAbilitySettings`:** Data assets defining team hostility matrix and ability activation cost configurations.

#### What Continues After Completion:
- Enabled instancing items and configuring combat weapons without modifying C++ code.

---

### Phase 6.5: Native Game Framework Engine Layer
| Field | Value |
|---|---|
| **Status** | 🟢 Complete |

#### What is Complete & How it Works:
- **`AAAAGameModeBase`:** Master session authority; spawns configured PlayerController, Pawn, HUD, and PlayerState.
- **`AAAAGameState`:** Replicated match state and server world time synchronization.
- **`AAAAPlayerState`:** Holds replicated ASC, AttributeSet, Player Name, and Level progression.
- **`AAAAHUD`:** Manages UMG lifecycle; creates and toggles `WBP_GameMenu`.
- **`AAAAGameInstance`:** Handles persistent session management and global save state initialization.

#### What Continues After Completion:
- Enabled full game boot flow from level load to pawn possession and UI instantiation.

---

### Phase 7: Modern Combat Engine & Stance State Machine
| Field | Value |
|---|---|
| **Status** | 🟡 85% Complete (C++ Complete, In-Editor Montages Wiring Pending) |

#### What is Complete & How it Works:
- **Dynamic Combat Stance Machine (`UAAACombatComponent`):**
  - Manages `EAAACombatStance` (`Unarmed`, `Armed_Holstered`, `Armed_Drawn`).
  - Controls weapon actor spawning (`AAAAWeaponActor`) and dynamic socket reparenting (`hand_r` vs `spine_03`).
  - Auto-sheaths weapon after 8 seconds of combat inactivity via `RefreshCombatTimeout(8.0f)`.
  - Synchronizes native gameplay tags: `State.Combat.Drawn`, `State.Combat.Holstered`, `State.Combat.Unarmed`.
- **Continuous Sweep Collision (`AAAAWeaponActor`):**
  - Continuous frame-to-frame sweeps between `TraceStart` and `TraceEnd` sockets.
  - Maintains `HitActorsThisSwing` array to guarantee each target is hit exactly once per attack swing.
- **Montage Notify Infrastructure (`AAAAnimation`):**
  - `UAnimNotifyState_WeaponTrace`: Marks the exact start and end of the hit sweep window on attack montages.
  - `UAnimNotify_AttachWeapon`: Triggers weapon re-parenting to hand/holster during equip/sheath montages.
- **Melee Combo Ability (`UAAAGameplayAbility_Melee`):**
  - Cycles through attack montages using combo indices defined in `UAAAWeaponSettings`.
  - Auto-draws weapon if triggered while holstered.
  - Applies damage via `UAAADamageExecutionCalc` with SetByCaller `Data.Damage`.
  - Triggers `GameplayCue.Combat.Hit.Melee` on impact.
- **Sekiro-Style Directional Step-Dodge (`UAAAGameplayAbility_Dodge`):**
  - Calculates movement input direction and executes directional step impulse (1200 units/s).
  - Deducts 15 stamina and applies `State.Combat.Invulnerable` i-frames during the dodge window.

#### What is Incomplete / Pending:
- [ ] Ensure custom 3D weapon meshes have `TraceStart` and `TraceEnd` collision sockets placed along the blade/edge.
- [ ] Place `AnimNotifyState_WeaponTrace` on attack montages inside the Editor.
- [ ] Assign attack and dodge animation montages into `DA_TestSword` and `BP_PlayerCharacter`.

#### What Continues After Completion:
- Live in-game hit detection against enemies, unlocking Phase 8 (Combat Slice verification).

---

### Phase 7.5: Enterprise Inventory & FastArray Replication
| Field | Value |
|---|---|
| **Status** | 🟡 80% Complete (C++ Complete, UI Data Binding Pending) |

#### What is Complete & How it Works:
- **Fast Array Delta Replication (`UAAAInventoryComponent`):**
  - Utilizes `FFastArraySerializer` (`FAAAInventoryList`, `FAAAInventoryEntry`) to send only modified slots across the network instead of replicating the entire inventory array.
  - Replicates dynamic subobjects (`UAAAItemInstance`) across network channels.
  - Manages adding, removing, splitting, and merging item stacks, with total weight calculation.
  - Broadcasts dynamic multicast delegates (`FAAAOnItemAdded`, `FAAAOnItemRemoved`, `FAAAOnStackChanged`, `FAAAOnInventoryChanged`).
- **Strongly Typed Equipment Manager (`UAAAEquipmentComponent`):**
  - Manages slots (`MainHand`, `OffHand`, `Head`, `Chest`, `Hands`, `Legs`, `Feet`, `Ring1`, `Ring2`, `Amulet`, `DragonSaddle`).
  - Enforces equipment level and category requirements.
- **Item Ecosystem (`AAAItems`):**
  - `UAAAItemDefinition` (PrimaryDataAsset base).
  - `UAAAItemInstance` (replicated runtime UObject for instanced durability, quality, stacks).
  - Settings assets: `AAAArmorSettings`, `AAAConsumableSettings`, `AAACraftingMaterialSettings`, `AAAItemWeaponSettings`, `AAAQuestItemSettings`, `AAAResourceSettings`.

#### What is Incomplete / Pending:
- [ ] Bind UMG widgets in `WBP_GameMenu` to `UAAAInventoryComponent` delegates to display real item icons and counts.
- [ ] Implement UI drag-and-drop equipping between Inventory Grid and Equipment Slots.

#### What Continues After Completion:
- Full inventory management, looting world items, and weapon switching directly from UI.
- Enables Phase 7.6 (SaveGame state serialization).

---

### Phase 7.6: SaveGame & State Persistence Architecture
| Field | Value |
|---|---|
| **Status** | 🟡 40% Complete (Serialization Contract Defined, C++ Class Scheduled) |

#### What is Complete & How it Works:
- **`SaveGame` Property Specifiers:** All mutable runtime item properties on `UAAAItemInstance` (GUID, ItemDef, StackCount, CurrentDurability, ItemLevel, CustomTags) and component containers (`UAAAInventoryComponent`, `UAAAEquipmentComponent`) are explicitly decorated with `UPROPERTY(SaveGame)`.
- **Serialization Contract Established:** Documented exact schema for storing inventory slots, equipped gear mappings, and player attributes.

#### Objectives & What Needs to Be Built:
- [ ] Create `UAAASaveGame` class derived from `USaveGame`.
- [ ] Implement `SaveInventoryState()`: Iterates `UAAAInventoryComponent`'s `FAAAInventoryList` and saves active item instances to a binary array/slot.
- [ ] Implement `LoadInventoryState()`: Reconstructs runtime `UAAAItemInstance` objects from saved records and populates inventory slots.
- [ ] Implement `SavePlayerState()`: Serializes `AAAAPlayerState` progression attributes (`Level`, `Experience`, `Gold`) and player transform.
- [ ] Implement Async Save/Load helper functions using `UGameplayStatics::AsyncSaveGameToSlot` and `AsyncLoadGameFromSlot`.

#### What Continues After Completion:
- Guarantees complete progression persistence across game sessions and enables save/load UI slots in `WBP_GameMenu`.

---

### Phase 8: First Playable Combat Vertical Slice & Dedicated Server Arena
| Field | Value |
|---|---|
| **Status** | 🟡 Active (In Progress) |

#### What is Complete & How it Works:
- **Combat Test Dummy (`AAAAATestEnemy`):**
  - Inherits from `AAAAAPlayerPawn`.
  - Holds hard references to ASC and AttributeSet (AI has no PlayerState).
  - Overrides `OnDeath` to enable full physics ragdoll upon reaching 0 health.
- **Weapon Sweep Trace Fallback & Loud Warning:**
  - `AAAAWeaponActor` sweeps continuous hit traces; if sockets are missing, it falls back to bounding box extents and displays a loud on-screen warning in PIE.
- **Test Setup Guide & Tools:**
  - `audit_unreal.py` and `weapon_import_guide.md` created to assist in importing 3D weapons (.glb/.fbx) and validating assets.

#### Objectives & What Needs to Be Built:
- [ ] **Single-Player Baseline Validation:**
  - Spawn `AAAAATestEnemy` in `L_Developer`.
  - Execute attack combo with `DA_TestSword` and verify weapon sweep hits `AAAAATestEnemy`.
  - Verify damage execution via `UAAADamageExecutionCalc` and ragdoll death on 0 HP.
- [ ] **Mandatory Multi-Client Dedicated Server Testing (PIE 2–3 Players):**
  - Set PIE to **2 Players** with **Run Dedicated Server** enabled.
  - **Melee Ability Prediction:** Validate client attacks predict locally with zero hitching and server authoritatively applies damage.
  - **Weapon Socket Reparenting Replication:** Validate that drawing/sheathing weapon on Client 1 properly replicates the socket attachment (`hand_r` vs `spine_03`) to Client 2's simulated proxy.
  - **Inventory FastArray Delta Replication:** Validate that adding/removing items on the server replicates delta updates to client inventory components without full-array stalls.

#### What Continues After Completion:
- Validates the complete combat and networking loop, unlocking Motion Matching combat locomotion (Phase 9) and World Interaction (Phase 10).

---

### Phase 9: Animation Choosers & Motion Matching Layers
| Field | Value |
|---|---|
| **Status** | 🔴 Next Up (Requires Phase 8 Validation) |

#### Objectives & What Needs to Be Built:
- [ ] Create Child AnimBP or Linked Anim Layer from Epic's `SandboxCharacter_Mover_ABP`.
- [ ] Create Chooser table branches for `Armed_Drawn` combat stance (sword-drawn idle, walk, run, sprint, strafe).
- [ ] Link pose search databases for combat-ready locomotion so the character moves naturally with weapon in hand.
- [ ] Integrate hit-reaction animation montages triggered by `GameplayCue.Combat.Hit.Melee`.

#### What Continues After Completion:
- Unlocks seamless transitions between exploration movement and high-fidelity combat movement.

---

### Phase 10: World Interaction & Environmental Loot
| Field | Value |
|---|---|
| **Status** | 🔴 Pending |

#### Objectives & What Needs to Be Built:
- [ ] Implement sphere/cone sweep trace in `UAAAInteractionComponent::TryInteract()` forward from camera/pawn.
- [ ] Implement `IAAAInteractable` on World Loot Actors (`BP_Chest`, `BP_WorldItemPickup`, `BP_Door`).
- [ ] Create HUD interaction prompt widget ("Press E to Open / Pick Up").
- [ ] Connect item pickups to `UAAAInventoryComponent::AddItemDefinition()`.

#### What Continues After Completion:
- Unlocks looting, opening doors, environmental puzzle interactions, and talking to NPCs.

---

### Phase 11: Advanced AI, Perception & StateTrees
| Field | Value |
|---|---|
| **Status** | 🔴 Future |

#### Objectives & What Needs to Be Built:
- [ ] Implement custom `AAAAAIController` with `UAIPerceptionComponent` (sight, hearing, damage).
- [ ] Build StateTree / Behavior Tree for melee humanoid enemies (Patrol $\rightarrow$ Investigate $\rightarrow$ Agro $\rightarrow$ Circle Target $\rightarrow$ Attack Combo $\rightarrow$ Flee on low health).
- [ ] Grant enemies `UAAAGameplayAbility_Melee` and dynamic weapon components.
- [ ] Implement Faction Hostility Matrix (`AAAFactionSettings`) to handle friendly, neutral, and enemy factions.

#### What Continues After Completion:
- Unlocks dynamic combat encounters, dungeon clearing, and faction warfare.

---

### Phase 12: Dragon Flight & Mount Framework
| Field | Value |
|---|---|
| **Status** | 🔴 Future |

#### Objectives & What Needs to Be Built:
- [ ] Implement `IAAAMountable` and `IAAADragonRider` interfaces on Mount Pawn.
- [ ] Build dragon flight physics utilizing Epic's Trajectory Component and Motion Warping.
- [ ] Implement player mount mounting/dismounting animation sync using sockets (`MountSocket`).
- [ ] Implement aerial combat (dragon breath attack via GAS, banking, diving).

#### What Continues After Completion:
- Unlocks high-speed aerial traversal across large-scale open worlds.

---

### Phase 13: World Partition, Quests & Dynamic Economy
| Field | Value |
|---|---|
| **Status** | 🔴 Future |

#### Objectives & What Needs to Be Built:
- [ ] Configure World Partition grid streaming for seamless multi-kilometer terrain.
- [ ] Implement Quest System with objective tracking (`IAAAQuestTarget`) and rewards.
- [ ] Implement NPC Dialogue trees and vendor buy/sell UI linked to `UAAAInventoryComponent`.
- [ ] Create kingdom territory control and faction reputation meters.

#### What Continues After Completion:
- Completes the single-player/co-op RPG loop across the open world.

---

### Phase 14: Dedicated Server & Multiplayer Hardening
| Field | Value |
|---|---|
| **Status** | 🔴 Future |

#### Objectives & What Needs to Be Built:
- [ ] Run headless dedicated server builds (`GameAnimationSampleServer`).
- [ ] Validate client-side prediction and server reconciliation on Mover locomotion and GAS abilities.
- [ ] Optimize network bandwidth replication frequencies on `FastArraySerializer` inventory entries and combat tags.
- [ ] Stress test multi-client combat encounters with desync detection.

#### What Continues After Completion:
- Final production milestone: Shipping-ready multiplayer action RPG.
