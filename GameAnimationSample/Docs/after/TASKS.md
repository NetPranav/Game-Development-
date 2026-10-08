# AAA Multiplayer Action RPG — Master Task Tracker

> Actionable task breakdown synchronized with the master roadmap and C++ architecture. Tasks are categorized by immediacy, technical domain, and sequential dependencies.

---

## 1. Immediate In-Editor Tasks (Phase 7 & 8: Combat Validation)

These tasks validate the C++ combat engine and hit registration in the Unreal Editor.

- [ ] **1.1. Weapon Mesh Socket Setup:**
  - Open weapon static mesh (e.g., test sword) in the Static Mesh Editor.
  - Add socket `TraceStart` at the weapon hilt/base.
  - Add socket `TraceEnd` at the weapon blade tip.
  - *Dependency:* Required by `AAAAWeaponActor::PerformTrace()` to sweep collision between frames.

- [ ] **1.2. Attack Montage Animation Notifies:**
  - Open Melee Attack Montages assigned in `DA_TestSword`.
  - Add a notify track and insert `AnimNotifyState_WeaponTrace` spanning from the beginning of the swing swing-arc to the end of the swing.
  - Add `AnimNotify_AttachWeapon` with `bAttachToHand = true` on the weapon draw montage, and `bAttachToHand = false` on the weapon sheath montage.
  - *Dependency:* Required for continuous sweep hit detection and visual weapon attachment.

- [ ] **1.3. Combat Test Arena Setup (`L_Developer`):**
  - Open `L_Developer` (or test map).
  - Place an instance of `BP_TestEnemy` (derived from `AAAATestEnemy`).
  - Ensure NavMeshBoundsVolume is present in the level.
  - Set GameMode override in World Settings to `BP_AAAGameMode`.

- [ ] **1.4. Live Melee & Stance Loop Validation:**
  - Launch Play In Editor (PIE).
  - Press `R` $\rightarrow$ verify character transitions to `Armed_Drawn`, weapon snaps from `spine_03` to `hand_r`, and `State.Combat.Drawn` tag is applied.
  - Wait 8 seconds without input $\rightarrow$ verify auto-sheath timer fires, weapon transitions back to `Armed_Holstered` at `spine_03`.
  - Press `LMB` $\rightarrow$ verify auto-draw triggers if holstered, combo montage plays, and `AnimNotifyState_WeaponTrace` activates sweep traces.
  - Swing at `BP_TestEnemy` $\rightarrow$ verify trace hits enemy, SetByCaller `Data.Damage` executes via `UAAADamageExecutionCalc`, enemy health decreases, and enemy ragdolls upon reaching 0 HP.
  - Press `LeftShift` with movement input $\rightarrow$ verify Sekiro directional step-dodge activates, stamina deducts by 15, and `State.Combat.Invulnerable` tag is active during the step.

- [ ] **1.5. Mandatory Multi-Client Replication Testing (PIE 2–3 Players + Dedicated Server):**
  - In Editor Preferences $\rightarrow$ Play Settings: Set `Number of Players = 2`, check `Run Dedicated Server`.
  - **Melee Attack Prediction Check:** Launch PIE as Client 1 and Client 2. Perform `LMB` melee combo on Client 1 $\rightarrow$ confirm attack anim predicts locally with zero rollback stutter, and server applies damage to target enemy.
  - **Weapon Draw/Sheath Socket Replication:** On Client 1, press `R` to draw weapon $\rightarrow$ confirm weapon actor physically attaches to `hand_r` on Client 1's view AND on Client 2's remote view of Client 1. Wait 8s $\rightarrow$ confirm weapon sheaths back to `spine_03` on both clients.
  - **Inventory FastArray Delta Replication:** Trigger an item addition on the server $\rightarrow$ confirm only the modified slot serializes and arrives on Client 1 without full-array spikes or desync.

---

## 2. Short-Term Technical Tasks (Phase 7.5, 7.6, 9 & 10)

Tasks to be completed immediately following successful combat validation.

### Phase 7.5: Inventory UI Data Binding
- [ ] **2.1. Bind `WBP_GameMenu` to `UAAAInventoryComponent`:**
  - Open `WBP_GameMenu` UMG graph.
  - On `Event Construct`, get owning PlayerPawn $\rightarrow$ get `UAAAInventoryComponent`.
  - Bind custom events to `FAAAOnItemAdded`, `FAAAOnItemRemoved`, and `FAAAOnStackChanged`.
  - Dynamically populate the left 50% grid with item tiles displaying item icon, name, and stack count.
- [ ] **2.2. Equipment Slot UI Interaction:**
  - Bind equipment slots (MainHand, Armor slots) to `UAAAEquipmentComponent`.
  - Handle clicking an item in inventory to equip into corresponding typed slot.

### Phase 7.6: SaveGame & Persistence Architecture
- [ ] **2.3. Implement `UAAASaveGame` Class:**
  - Inherit from `USaveGame` in `AAAGame` or `AAAInventory`.
  - Add serialized records for inventory item entries (ItemDef asset path, GUID, stack count, durability, level, custom tags).
  - Add serialized records for equipped slot mappings.
  - Add serialized player progression (`Level`, `Experience`, `Gold`) and world transform.
- [ ] **2.4. Save & Load Integration:**
  - Add `SavePlayerGame(int32 SlotIndex)` and `LoadPlayerGame(int32 SlotIndex)` to `AAAAGameInstance`.
  - Use `UGameplayStatics::AsyncSaveGameToSlot` to guarantee asynchronous, hitch-free saves.
  - Connect Save/Load buttons in `WBP_GameMenu`.

### Phase 9: Animation Choosers & Motion Matching Layers
- [ ] **2.5. Armed Locomotion Chooser Table:**
  - *Requires:* Phase 8 combat validation complete.
  - Open Epic's `SandboxCharacter_Mover_ABP` Chooser table.
  - Add branch for `State.Combat.Drawn` tag to route to armed locomotion pose search databases (sword-drawn idle, jog, sprint, strafe).
- [ ] **2.6. Hit Reaction GameplayCue Montage:**
  - Create or assign hit-reaction montage to `GameplayCue.Combat.Hit.Melee`.
  - Trigger directional flinch on target actor upon receiving melee impact.

### Phase 10: World Interaction System
- [ ] **2.5. Implement Forward Sweep in `UAAAInteractionComponent`:**
  - Add sphere/capsule sweep forward from the pawn/camera (range: 250 units).
  - Check if hit actor implements `IAAAInteractable`.
  - Cache current interactable target and broadcast to HUD.
- [ ] **2.6. Create Interaction Prompt Widget (`WBP_InteractPrompt`):**
  - Display "Press E to Interact / Pick Up" when looking at an interactable object.
  - Bind `IA_Interact` in Enhanced Input to call `UAAAInteractionComponent::TryInteract()`.
- [ ] **2.7. World Item Actor (`BP_WorldItemPickup`):**
  - Create actor with StaticMeshComponent, sphere collision, and `IAAAInteractable` implementation.
  - Interacting adds item to player's `UAAAInventoryComponent` and destroys the world actor.

---

## 3. Medium-Term Tasks (Phase 11: Enemy AI & StateTrees)

Tasks for building dynamic combatants and encounter systems.

- [ ] **3.1. Enemy AI Controller (`AAAAAIController`):**
  - Implement AIController class with `UAIPerceptionComponent` configured for Sight and Damage sensing.
  - Wire perception updates to register player character as combat target.
- [ ] **3.2. Melee Enemy StateTree:**
  - *Requires:* Phase 8 combat loop and Phase 9 locomotion complete.
  - Build StateTree with states: `Idle/Patrol` $\rightarrow$ `Investigate` $\rightarrow$ `Agro/Combat` $\rightarrow$ `Attack Execution` $\rightarrow$ `Recovery/Strafing`.
  - In `Attack Execution`, invoke enemy `UAAAGameplayAbility_Melee` with weapon traces.
- [ ] **3.3. Faction Hostility Matrix:**
  - Implement hostility checks using `AAAFactionSettings` (Player faction vs Bandits vs Beasts).

---

## 4. Long-Term Architectural Tasks (Phase 12, 13 & 14)

Long-range production systems.

- [ ] **4.1. Mount & Dragon Riding Framework:**
  - *Requires:* Character locomotion and interaction systems fully stable.
  - Implement `IAAAMountable` and `IAAADragonRider` interfaces.
  - Implement Dragon flight locomotion using Epic's Trajectory Component and Motion Warping.
  - Build mount attachment sockets and mounting/dismounting montages.
- [ ] **4.2. World Partition & Open World Setup:**
  - Configure multi-kilometer World Partition grid and data layers.
  - Implement streaming level bounds and procedural encounter spawners.
- [ ] **4.3. Quests & Dialogue System:**
  - Implement `IAAAQuestTarget` objective listening.
  - Build dialogue UI widget and NPC interaction logic.
- [ ] **4.4. Dedicated Server Optimization:**
  - Build and run headless dedicated server target.
  - Optimize `FastArraySerializer` replication intervals and network relevancy.
  - Verify desync-free combat and ability prediction under network latency.

---

## 5. Execution Dependency Flowchart

```
[Phase 7 & 8: Weapon Sockets, Montages & Slices]
       │
       ▼
[Live In-Editor Melee Hit & Ragdoll Validated]
       │
       ▼
[Phase 8 Multi-Client Dedicated Server Replication Check]
(Melee Prediction + Socket Reparenting + FastArray Delta Rep)
       │
       ├────────────────────────────────────────┬───────────────────────────────────────┐
       ▼                                        ▼                                       ▼
[Phase 9: Anim Choosers & Armed Locomotion] [Phase 7.5: Inventory UI Binding] [Phase 10: Interaction Trace & World Loot]
       │                                        │                                       │
       │                                        ▼                                       │
       │                            [Phase 7.6: SaveGame & Persistence]                 │
       │                            (Inventory, Equipment, Stats Serialization)         │
       │                                        │                                       │
       ▼                                        ▼                                       ▼
[Full Combat Stance Locomotion]             [Persistent Saved Inventory]             [World Item Pickups & Chests]
       │                                        │                                       │
       └────────────────────────────────────────┼───────────────────────────────────────┘
                                                │
                                                ▼
                                    [Phase 11: Enemy AI StateTree]
                                                │
                                                ▼
                                 [Dynamic Hostile Encounters]
                                                │
                                                ▼
                                   [Phase 12: Dragon Mounts]
                                                │
                                                ▼
                              [Phase 13 & 14: World Partition & Dedicated Server]
```
