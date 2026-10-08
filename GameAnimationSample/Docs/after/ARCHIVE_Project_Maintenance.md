# ARCHIVE_Project_Maintenance

> [!CAUTION]
> ## SUPERSEDED HISTORICAL ARCHIVE — DO NOT IMPLEMENT AGAINST THIS DOCUMENT
> This file contains historical build verification, audit reports, and maintenance records from earlier development sessions. 
> Refer strictly to [PROJECT_OVERVIEW.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/PROJECT_OVERVIEW.md), [ROADMAP.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/ROADMAP.md), and [BUGS.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/BUGS.md) for canonical architecture, active tasks, and resolved issues.

*This file is a merged archive of older, granular documentation.* 



---
## FROM ORIGINAL FILE: BUILD_VERIFICATION.md
---

# Build Verification Report
**Phase 3.5** | AAA Multiplayer RPG

This document certifies that the project has been verified for successful compilation across standard development configurations, ensuring there are no hidden linker errors, missing modules, or unresolved symbols before beginning gameplay logic development.

---

## 1. Verified Configurations

| Configuration | Target | Status | Notes |
| :--- | :--- | :--- | :--- |
| **Development Editor** | `GameAnimationSampleEditor` | 🟢 Verified | All 12 AAA modules compile and load successfully in the editor context. |
| **DebugGame Editor** | `GameAnimationSampleEditor` | 🟢 Verified | Clean debug symbols generated. No PDB corruption detected. |
| **Development** (Game) | `GameAnimationSample` | 🟢 Verified | Client-side packaged build configuration validates correctly. |

## 2. Linker & Symbol Verification

During the audit, a critical linker vulnerability was identified and resolved:
- **Identified Issue:** `AAAAbility` module required the `GameplayAbilities`, `GameplayTags`, and `GameplayTasks` modules. However, the `GameplayAbilities` plugin was missing from the `GameAnimationSample.uproject` file.
- **Resolution:** The `GameplayAbilities` plugin was explicitly added to the `.uproject` plugin list, preventing an `LNK2019: unresolved external symbol` error that would have occurred during the next editor launch or full rebuild.
- **Current Status:** 
  - **Linker Errors:** 0
  - **Unresolved Symbols:** 0
  - **Missing Modules:** 0

## 3. Module Registration

All 12 modules have been successfully registered in the following build targets:
1. `GameAnimationSample.uproject`
2. `GameAnimationSample.Target.cs`
3. `GameAnimationSampleEditor.Target.cs`

No duplicate registrations or invalid module loading phases were found. The system leverages `LoadingPhase: Default` correctly across the board.


---
## FROM ORIGINAL FILE: CONFIGURATION_GUIDE.md
---

# Gameplay Configuration Guide

To create a new gameplay element (like a new weapon or new NPC) without programming:

## 1. Create a Data Asset
1. In the Content Browser, right click -> **Miscellaneous -> Data Asset**.
2. When prompted for a class, select the appropriate Settings class (e.g., `AAAWeaponSettings`).

## 2. Populate the Values
1. Open the Data Asset.
2. Fill out the properties (e.g., Set Mesh, Base Damage, and assign Gameplay Abilities like `GA_LightAttack`).

## 3. Assign the Data Asset
1. Inside your Blueprint (e.g., your Character Blueprint or Weapon Actor Blueprint), locate the `Settings` property.
2. Assign your newly created Data Asset to that property.
3. The C++ framework will automatically extract the values and inject them into the Ability System, Movement components, and UI widgets!


---
## FROM ORIGINAL FILE: CONTENT_STRUCTURE.md
---

# AAA Project — Content Architecture & Conventions

## Core Philosophy
To maintain scalability, upgradeability, and prevent asset contamination, this project implements a strict namespace and ownership hierarchy.

### The Foundation Layer (Epic Games)
Folders at the root of `Content/` that are NOT inside the `AAA/` directory are strictly **Epic Games Foundation Assets**.
These include:
*   `Blueprints/`
*   `Characters/`
*   `Locomotor/`
*   `Input/`
*   `MetaHumans/`
*   `Traversal/`

**Rule 1:** Epic Games foundation assets **MUST NEVER** be moved, modified, or deleted. 
**Rule 2:** We rely on these via inheritance (creating a Child Blueprint in our `AAA/` folder), composition, or wrapping.

### The Gameplay Layer (AAA)
All custom gameplay logic, new characters, weapons, maps, UI, and data must reside exclusively inside the `Content/AAA/` directory.

#### `AAA/` Folder Structure
*   `AI/`: Artificial Intelligence controllers, StateTrees, and behavior data.
*   `Animation/`: Custom animations, Montages, Pose Search Databases, and Choosers for custom states (e.g., combat).
*   `Audio/`: Sound effects, cues, and dialogue.
*   `Blueprints/`: Core gameplay actors, managers, and non-character blueprints.
*   `Cameras/`: Camera shakes, modifiers, and custom SpringArm configurations.
*   `Characters/`: Player and enemy characters (Meshes, Child AnimBPs, and reparented Actor Blueprints).
*   `Combat/`: Damage types, hit reactions, and combat configurations.
*   `Components/`: Custom Actor Components (e.g., InventoryComponent).
*   `Data/`: Data Tables and struct definitions.
*   `DataAssets/`: Primary Data Assets instances.
*   `Debug/`: Developer testing tools and visualization assets.
*   `Dragons/`: Specific assets relating to dragon entities.
*   `Effects/`: Visual effects (if separated from Niagara).
*   `Factions/`: Data related to reputation and faction alignment.
*   `Gameplay/`: Game Modes, Game States, Player Controllers, and HUD classes.
*   `Input/`: Custom Input Actions and Input Mapping Contexts.
*   `Interfaces/`: Blueprint Interfaces.
*   `Items/`: Collectibles and inventory items.
*   `Levels/`: All `.umap` files (including `L_Developer`).
*   `Materials/`: Master materials and Material Instances.
*   `Meshes/`: Static meshes.
*   `Mounts/`: Data and logic for standard mounts.
*   `Niagara/`: Niagara VFX systems.
*   `NPC/`: Non-Player Character definitions and data.
*   `Physics/`: Physical materials and physics assets.
*   `Quests/`: Quest data and progression logic.
*   `SaveGame/`: Save game blueprint objects.
*   `UI/`: Widget blueprints and slate styling.
*   `Utilities/`: Helper macros, blueprint function libraries.
*   `Weapons/`: Weapon actors, meshes, and specific configs.
*   `World/`: World partitioning, sky atmospheres, and environment logic.

---

## Naming Conventions
Consistency is critical for a project expected to scale to hundreds of thousands of assets. All assets inside `Content/AAA/` must follow these prefixes:

| Asset Type | Prefix | Example |
| :--- | :--- | :--- |
| **Level / Map** | `L_` | `L_Developer` |
| **Blueprint Actor** | `BP_` | `BP_WeaponBase` |
| **Blueprint Component** | `BPC_` | `BPC_Inventory` |
| **Blueprint Interface** | `BPI_` | `BPI_Interactable` |
| **Blueprint Function Library** | `BFL_` | `BFL_MathUtils` |
| **Widget Blueprint (UI)** | `WBP_` | `WBP_PlayerHUD` |
| **Animation Blueprint** | `ABP_` | `ABP_Greatsword_Overlay` |
| **Animation Sequence** | `Anim_` | `Anim_RunForward` |
| **Animation Montage** | `AM_` | `AM_HeavyAttack` |
| **Material (Master)** | `M_` | `M_StoneWall` |
| **Material Instance** | `MI_` | `MI_StoneWall_Dark` |
| **Texture (Diffuse/Albedo)** | `T_` | `T_Wood_D` |
| **Texture (Normal)** | `T_` | `T_Wood_N` |
| **Texture (Mask/ORM)** | `T_` | `T_Wood_ORM` |
| **Static Mesh** | `SM_` | `SM_Barrel` |
| **Skeletal Mesh** | `SKM_` | `SKM_Dragon` |
| **Niagara System** | `NS_` | `NS_Fireball` |
| **Niagara Emitter** | `NE_` | `NE_Sparks` |
| **Data Asset** | `DA_` | `DA_LongswordStats` |
| **Data Table** | `DT_` | `DT_WeaponList` |
| **Enum** | `E_` | `E_WeaponType` |
| **Struct** | `F_` | `F_InventorySlot` (Or C++ default) |
| **Input Action** | `IA_` | `IA_Jump` |
| **Input Mapping Context** | `IMC_` | `IMC_Combat` |
| **Sound Cue** | `SC_` | `SC_Explosion` |
| **Sound Wave** | `SW_` | `SW_Footstep` |
| **Control Rig** | `CR_` | `CR_DragonWings` |


---
## FROM ORIGINAL FILE: DECISIONS.md
---

# AAA Multiplayer RPG — Architectural Decisions

> Record of all major architectural shifts and engineering decisions.

## 1. The Project Reset (2026-07-03)
**Context:** The project initially attempted to build a custom C++ locomotion framework and later fuse Epic's Game Animation Sample into it. This resulted in corrupted migrated assets, duplicated logic, and broken interfaces.
**Decision:** We completely abandoned the old repository (`AAATitle`) and restarted directly from a pristine `GameAnimationSample` Epic project.
**Consequences:** 
- Epic's locomotion is now treated as immutable Engine Code.
- All gameplay features (GAS, Inventory, Combat) must wrap, extend, or compose Epic's foundation.
- We avoid modifying Epic's sample Blueprints unless absolutely necessary (e.g., reparenting `BP_SandboxCharacter` to our C++ base).


---
## FROM ORIGINAL FILE: MANUAL_TASKS_EDITOR.md
---

# Manual Editor Tasks — Required Before Gameplay Testing

> **Created:** 2026-08-21
> **Status:** COMPLETED
> **Priority:** DONE — All Editor configurations have been implemented.

These tasks cannot be automated via Remote Control API (the API doesn't support creating Blueprint assets or writing to protected component properties). They must be done manually in the Unreal Editor.

---

## Task 1: Delete Duplicate BP_AAAGameMode ⏱️ 1 min

1. Content Browser → `Content/AAA/Gameplay/`
2. Right-click `BP_AAAGameMode` → **Delete**
3. The canonical one stays at `Content/AAA/Blueprints/GameFramework/BP_AAAGameMode`

---

## Task 2: Create GE_DirectDamage ⏱️ 3 min

1. Content Browser → `Content/AAA/Gameplay/` → create subfolder `Effects`
2. Right-click → **Blueprint Class** → parent: **GameplayEffect** → name: `GE_DirectDamage`
3. Open it → set:
   - **Duration Policy** = `Instant`
   - Add **Modifier**: Attribute = `AAAAttributeSet.Health`, Op = `Add`
   - **Magnitude Type** = `SetByCaller`, **Data Tag** = `Data.Damage`
4. Save

---

## Task 3: Create GE_DirectHeal ⏱️ 2 min

1. Same folder: `Content/AAA/Gameplay/Effects/`
2. Right-click → **Blueprint Class** → **GameplayEffect** → name: `GE_DirectHeal`
3. Same setup but **Data Tag** = `Data.Healing`
4. Save

---

## Task 4: Wire GE to BP_PlayerCharacter ⏱️ 2 min

1. Open `Content/AAA/Characters/BP_PlayerCharacter`
2. Click `AttributeComponent` in Components panel
3. In Details: set **Direct Damage Effect Class** = `GE_DirectDamage`
4. Set **Direct Heal Effect Class** = `GE_DirectHeal`
5. Compile → Save

---

## Task 5: Create GameplayCue Directory ⏱️ 30 sec

1. Content Browser → `Content/AAA/Effects/` → New Folder → `GameplayCues`

---

## Task 6: Verify DA_PlayerCharacterSettings Input ⏱️ 2 min

1. Open `Content/AAA/DataAssets/Characters/DA_PlayerCharacterSettings`
2. Expand **Input** → **Ability Input Bindings**
3. Add entry: Input Action = `IA_Attack`, Input Tag = `InputTag.Attack`
4. Save

---

## Task 7: Verify DA_TestSword ⏱️ 1 min

1. Open `Content/AAA/DataAssets/Asset/Sword/DA_TestSword`
2. Set **Combat → Base Damage** = `25.0` (or any value > 0)
3. Save

---

## Task 8: Verify World Settings ⏱️ 1 min

1. Open `Content/AAA/Levels/L_Developer`
2. Window → World Settings
3. **Game Mode Override** must point to `Content/AAA/Blueprints/GameFramework/BP_AAAGameMode`

---

## After Completion

Check off items in [PROJECT.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/PROJECT.md) session memory as you complete them.

Once all 8 tasks are done, the combat pipeline is fully connected and you can begin importing 3D assets for gameplay testing.


---
## FROM ORIGINAL FILE: PROJECT.md
---

# AAA Multiplayer RPG - Project Documentation

## Project Vision
To build a professional-quality AAA multiplayer RPG using Unreal Engine 5.4+. The project will feature a large seamless world, multiplayer gameplay, dragon riding, mounts, advanced AI NPCs, faction reputation, dynamic economy, and persistent consequences.

## The Canonical Repository
> [!IMPORTANT]
> **Source of Truth:** `Game/GameAnimationSample`
> The old `AAATitle` repository is officially deprecated and serves only as historical reference for gameplay concepts.

## Core Philosophy: Extend & Compose
Epic's **Game Animation Sample (GAS)** is the absolute foundation of our locomotion.
- **Epic Owns:** Motion Matching, Pose Search, Choosers, Traversal, Motion Warping, Mover, Control Rig, IK.
- **We Own:** Gameplay, Combat, Inventory, Gameplay Ability System (GAS), Quests, World Systems, AI, and Multiplayer.

**Rules of Engagement:**
- NEVER overwrite or duplicate Epic's locomotion assets.
- ALWAYS use inheritance, extension, wrapping, or composition.
- Treat Epic's locomotion Blueprints and databases as Read-Only Engine Code.

## Current Session Memory

| Field | Value |
|---|---|
| **Current Phase** | Phase 7B — Combat Visuals & Custom Content |
| **Current Objective** | Import 3D assets (e.g., sword .glb), wire to Combat Component, and test melee system |
| **Current System** | AAA Combat System + Sandbox Motion Matching |
| **Current Roadblock** | None. Editor is successfully wired. |
| **Next Immediate Step** | Follow `weapon_import_guide.md` to equip the custom sword and test the attack loop |

- [x] Phase 6.5: Core Gameplay Framework (Native Engine Layer) Complete.
- [x] AI Automation Pipeline established (Remote Control API confirmed on localhost:30010)
- [x] All circular module dependencies resolved (5 Build.cs files fixed)
- [x] DebugGame build compiles clean — 0 errors, 0 warnings
- [x] GameplayCueManager registered in DefaultGame.ini
- [x] AAA gameplay tags added to DefaultGameplayTags.ini (InputTag.*, Data.*, GameplayCue.*, etc.)
- [x] Debug weapon trace disabled (EDrawDebugTrace::None)
- [x] GE_DirectDamage / GE_DirectHeal — manually created in editor
- [x] DA_PlayerCharacterSettings — AbilityInputBindings configured
- [x] Duplicate BP_AAAGameMode — manually deleted
- [x] `BP_PlayerCharacter` rebuilt by duplicating Epic's `SandboxCharacter_Mover` and reparenting to `AAAAPlayerPawn`


---
## FROM ORIGINAL FILE: PERFORMANCE_BUDGETS.md
---

# AAA Multiplayer RPG — Performance & Profiling Budgets

> **Version:** 1.0  
> **Target platforms:** High-End PC & Next-Gen Consoles  
> **Target Framerate:** 60 FPS (16.67ms per frame) & 120 FPS Performance Mode (8.33ms per frame)  
> **Status:** Mandatory Architectural Baseline

---

## ⚡ Global Frame Budget (60 FPS Baseline)

To maintain a locked 60 FPS under the heaviest combat loads, total frame time across all threads must never exceed **16.67ms**. Because CPU and GPU work asynchronously, each processor operates within its own dedicated concurrency window:

```
┌──────────────────────────────────────────────────────────────┐
│                  Frame Target: 16.67 ms                      │
├──────────────────────────────────────────────────────────────┤
│  Game Thread (CPU)   │  10.0 ms max  │ (6.67 ms headroom)    │
├──────────────────────────────────────────────────────────────┤
│  Render Thread (CPU) │  10.0 ms max  │ (6.67 ms headroom)    │
├──────────────────────────────────────────────────────────────┤
│  GPU Rendering       │  14.5 ms max  │ (2.17 ms headroom)    │
└──────────────────────────────────────────────────────────────┘
```

| Thread / Hardware | Maximum Hard Budget | Recommended Operational Target | Notes |
| :--- | :--- | :--- | :--- |
| **Game Thread (CPU)** | **≤ 10.0 ms** | 8.0 ms | Handles logic, animation updates, GAS, AI, and collision. |
| **Render Thread (CPU)** | **≤ 10.0 ms** | 7.5 ms | Calculates visibility, occlusion, draw call sorting, and HISM batching. |
| **RHI Thread (CPU)** | **≤ 5.0 ms** | 3.5 ms | Submits command buffers to DirectX 12 / Vulkan APIs. |
| **GPU (60 FPS Mode)** | **≤ 16.67 ms** | 14.5 ms | Geometry rendering, Lumen GI, Virtual Shadow Maps, and post-processing. |
| **GPU (120 FPS Mode)**| **≤ 8.33 ms** | 7.2 ms | Scaled-down rendering resolution via TSR/FSR for competitive fluidity. |

---

## 🧠 Game Thread Allocation (Max 10.0 ms)

The Game Thread executes all core gameplay systems. To prevent CPU bottlenecks during large combat encounters, system budgets are strictly divided:

| System Category | Budget (ms) | Architectural Enforcement Rules |
| :--- | :--- | :--- |
| **Animation & Mover** | **2.5 ms** | Use multi-threaded anim evaluation; enforce rigid animation LODs at distance (>100m = simple transforms). |
| **Gameplay Ability System (GAS)** | **2.0 ms** | Zero GC allocations per activation (`SetByCallerMagnitude` + native tags); avoid synchronous iteration over character arrays. |
| **AI & Navigation (AAAAI)** | **1.5 ms** | Time-slice behavior trees and EQS queries over multiple frames; restrict perception updates to ≤ 5 ticks/sec for distant AI. |
| **Physics & Collision** | **1.5 ms** | Use simplified primitive collision shapes for query traces; avoid complex-as-simple traces on Nanite geometry. |
| **Inventory & Equipment** | **0.5 ms** | Implement `FFastArraySerializer` for net replication; perform inventory filtering asynchronously where applicable. |
| **UI & Audio Updates** | **1.0 ms** | Event-driven UI updates only (no `EventTick` on Widgets); pool sound spawns via GameplayCues. |
| **System Headroom & OS**| **1.0 ms** | Safety buffer to absorb OS thread scheduling variance and prevent micro-stutters. |

---

## 🎨 GPU Render Budget Allocation (Max 16.67 ms)

At full 4K resolution (via Temporal Super Resolution from 1440p internal rendering), the GPU workload is partitioned across render passes:

| GPU Render Pass | Budget (ms) | Optimization Directives |
| :--- | :--- | :--- |
| **Base Pass & Geometry (Nanite)** | **3.5 ms** | All static environment meshes must utilize Nanite; limit masked materials; enforce ≤ 200 instructions per material. |
| **Lumen Global Illumination** | **3.0 ms** | Restrict scene to ≤ 4 movable shadow-casting lights; rely on software ray tracing via Mesh Distance Fields. |
| **Virtual Shadow Maps (VSM)** | **2.5 ms** | Cache static shadow pages; limit character shadow radius to actual combat visibility boundaries. |
| **Lighting & Reflections** | **2.0 ms** | Utilize reflection captures for high-frequency specular backup; avoid oversized point light radii. |
| **Niagara VFX & Particles** | **1.5 ms** | Enforce GPU simulation for systems > 100 particles; pool cosmetic VFX via `UAAAGameplayCueManager`. |
| **Post-Processing & TSR** | **2.0 ms** | Use Temporal Super Resolution (TSR) for clean upscaling; restrict bloom and depth-of-field sample counts. |
| **GPU Headroom** | **2.17 ms** | Buffer to maintain locked framerates during intensive alpha-blended combat explosion overlap. |

---

## 💾 Memory & Storage Budgets

To guarantee stability on minimum spec hardware (16GB System RAM / 8GB VRAM) without paging stutters:

| Memory Pool | Hard Limit | Enforcement Strategy |
| :--- | :--- | :--- |
| **Total System RAM** | **≤ 8.0 GB** | Application process footprint must remain under 8GB to leave space for OS and background applications. |
| **Texture Memory (VRAM)**| **≤ 3.5 GB** | Enforce Oodle texture compression and channel-packed ORM masks; stream high-res maps via Virtual Texturing. |
| **Mesh Geometry (VRAM)** | **≤ 2.0 GB** | Nanite geometry streaming pools; eliminate duplicate asset references in World Partition levels. |
| **Audio & SFX Memory** | **≤ 500 MB** | Use ADPCM/Vorbis streaming for dialogue and background music; cache only transient combat impact sounds. |
| **UI Assets & Fonts** | **≤ 150 MB** | Texture atlases for icons; enforce tight texture bounds without trailing transparent space. |

---

## 🌐 Network Replication & Bandwidth (Multiplayer Preparation)

While initially offline, all structural gameplay code must adhere to network constraints to enable seamless transition to online multiplayer:

| Networking Metric | Hard Budget | Implementation Standards |
| :--- | :--- | :--- |
| **Downstream Bandwidth** | **≤ 64 KB/s** per player | Replicate only state changes (delta serialization via `FFastArraySerializer`). Never replicate visual-only state. |
| **Upstream Bandwidth** | **≤ 16 KB/s** per player | Transmit minimal client input parameters and predicted ability activation handles. |
| **Net Update Frequency** | **≤ 60 Hz (Characters)** <br> **≤ 10 Hz (Interactive Props)**| Adjust actor net tick rates based on distance and combat interaction relevance. |
| **Cosmetic Replication Overhead** | **0.0 Bytes** | **100% of VFX/SFX must trigger via replicated `GameplayCues`**; never transmit RPCs solely to spawn sound or particles. |

---

## 🛠️ Validation & Profiling Console Commands

Every feature contributor must validate code and assets against these budgets using Unreal Engine's integrated telemetry before committing changes:

```bash
# Core real-time thread timing monitor
stat unit

# Detailed scene rendering & draw call counters (Must remain ≤ 2,500 draw calls)
stat scenerendering

# Real-time inspection of Gameplay Ability System overhead & active abilities
stat gameplayabilities

# Niagara VFX compute & GPU particle simulation profiling
stat Niagara

# Deep-dive interactive GPU frame profiling capture (opens visual analyzer)
profilegpu

# CPU flame-graph recording (Capture 10 seconds during combat test)
stat startfile
# ... play through combat encounter ...
stat stopfile

# Nanite overdraw and triangle complexity inspection view modes
r.Nanite.Visualize.Overdraw 1
r.Nanite.Visualize.Triangles 1

# Material instruction cost heat map (Green/Red/White gradient)
r.ShaderComplexity 1
```

---

## 🛑 Strict Zero-Tolerance Architectural Rules

1. **No Direct Input-to-Ability Activation in C++**  
   Input must map to a native `InputTag`, which the ASC evaluates against `AbilityTags`.
2. **No Dynamic GameplayEffect Instantiation During Combat**  
   Calling `NewObject<UGameplayEffect>()` inside attack loops is strictly prohibited. Use pre-authored CDOs with `SetByCallerMagnitude`.
3. **No Direct Niagara or Audio Spawning in Gameplay Abilities**  
   All visual and audio effects must be executed via `ExecuteGameplayCue()` to ensure pooling and eliminate network cosmetic spam.
4. **No Full Array Replication for Inventories or Lists**  
   Any container exceeding 5 elements must implement `FFastArraySerializer` for delta-only transmission.
5. **No Synchronous Asset Loading in Core Combat Loops**  
   All weapon meshes, montage animations, and sound banks must be preloaded asynchronously via Soft Object References in Data Assets.


---
## FROM ORIGINAL FILE: RENDERING_PIPELINE.md
---

# AAA Multiplayer RPG — Rendering Pipeline Bible

> **Version:** 1.0  
> **Engine:** Unreal Engine 5.8  
> **Target:** AAA-quality GPU-intensive visuals at 60fps (4K with TSR/FSR)  
> **Status:** Living document — every artist and programmer must follow these rules.

---

## ⚡ Performance Budgets (Non-Negotiable)

| Metric | Budget | Notes |
|---|---|---|
| **Game Thread** | ≤ 10ms per frame | All gameplay logic, animation, AI |
| **Render Thread** | ≤ 10ms per frame | Draw call submission, visibility |
| **GPU** | ≤ 16.67ms per frame (60fps target) | All rendering, post-processing, VFX |
| **GPU (120fps mode)** | ≤ 8.33ms per frame | Console performance mode |
| **Draw Calls** | ≤ 2,500 per frame | Use instancing to stay under |
| **Triangle Count** | ≤ 10M visible (without Nanite) | Nanite meshes have no hard limit |
| **Texture Memory** | ≤ 4GB VRAM budget | Virtual Texturing for overflow |
| **System RAM** | ≤ 10GB total application | Including streaming pools |
| **Network Bandwidth** | ≤ 64KB/s per player | Delta replication, relevancy |

---

## 🔺 Nanite (Virtualized Geometry)

### When to Use Nanite
| Use Nanite For | Do NOT Use Nanite For |
|---|---|
| Static environment meshes (buildings, rocks, terrain features) | Skeletal meshes (characters, creatures) |
| Props and set dressing (crates, barrels, furniture) | Foliage with complex wind animation |
| Architectural details (walls, floors, columns) | Translucent/masked materials |
| Large-scale world geometry | Instanced meshes under 1K triangles |
| Any static mesh over 50K triangles | Meshes that need to deform |

### Nanite Rules
1. **All static environment art must be Nanite-enabled** — no exceptions.
2. Source meshes should be authored at **full detail** (no manual LODs). Nanite handles LOD automatically.
3. Maximum source mesh: **No hard limit** — but test performance with Nanite visualization modes.
4. Materials on Nanite meshes must use **Opaque** blend mode. Masked/Translucent is NOT supported.
5. Enable `r.Nanite.MaxPixelsPerEdge 1` for quality, `2` for performance mode.
6. Use `Nanite Visualization` viewport mode regularly to audit overdraw.

### Nanite Migration Checklist
```
For every static mesh in the project:
[ ] Enable "Support Nanite" in mesh settings
[ ] Remove all manual LOD groups (Nanite replaces them)
[ ] Verify material uses Opaque blend mode
[ ] Test with Nanite Visualization → Triangles mode
[ ] Verify no visual artifacts at distance
```

---

## 💡 Lumen (Global Illumination & Reflections)

### Configuration
```
// DefaultEngine.ini — recommended settings
[/Script/Engine.RendererSettings]
r.DynamicGlobalIlluminationMethod=1        ; 1=Lumen
r.ReflectionMethod=1                        ; 1=Lumen
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.Lumen.HardwareRayTracing=0               ; Software tracing (wider GPU compatibility)
r.Lumen.TraceMeshSDFs=1
```

### Lumen Rules
1. **Maximum 4 movable lights per scene** — each additional light multiplies trace cost.
2. **No point lights with radius > 5000** — huge lights destroy Lumen performance.
3. **All important geometry must generate Mesh Distance Fields** — enable per-mesh in settings.
4. **Interior scenes** must use `Lumen Scene Detail` set to `High` for small object GI.
5. **Emissive materials** with high intensity (>10) act as area lights in Lumen — use intentionally, not accidentally.
6. **Sky Light** must be set to `Real Time Capture` for outdoor scenes with Lumen.

### Light Complexity Budget
| Scene Type | Max Movable Lights | Max Stationary | Max Static |
|---|---|---|---|
| Open World Exterior | 2 | 4 | Unlimited |
| Interior Room | 3 | 6 | Unlimited |
| Combat Arena | 4 | 4 | Unlimited |
| Dungeon/Cave | 3 | 8 | Unlimited |

---

## 🌑 Virtual Shadow Maps (VSM)

### Rules
1. **All lights must use Virtual Shadow Maps** — enable globally in Project Settings.
2. **Cascaded Shadow Maps are deprecated** — do not use CSM for any light.
3. `r.Shadow.Virtual.MaxPhysicalPages 4096` — baseline for 4K rendering.
4. Movable lights with shadows are the **single most expensive rendering feature** — minimize.
5. Characters: Shadow quality via `r.Shadow.Virtual.ResolutionLodBiasDirectional -0.5` for crisp character shadows.

---

## 🌍 World Partition & Streaming

### Rules for Open World
1. **World Partition must be enabled** on the Chapter 1 map — this is mandatory for any level > 500m².
2. Cell size: `12800` (128m) for general content, `25600` (256m) for landscape.
3. **Data Layers** for separating gameplay tiers (e.g., "Chapter1_Story" vs "Chapter1_Combat").
4. **Level Instances** for repeating structures (camps, villages, dungeons).
5. `Loading Range` per cell: tune based on player visibility and terrain occlusion.

### HLOD (Hierarchical LOD)
1. Enable HLOD generation for World Partition.
2. All static meshes in exterior scenes must participate in HLOD.
3. HLOD meshes should use simplified materials (no tessellation, no parallax).
4. Target: HLOD reduces draw calls by **70%** at distances > 300m.

---

## 🎨 Material Pipeline

### Master Material System
Every material in the game must be a **Material Instance** of one of these Master Materials:

| Master Material | Use Case |
|---|---|
| `MM_Opaque_Standard` | Default for all solid surfaces |
| `MM_Opaque_Foliage` | Vegetation with subsurface scattering and wind |
| `MM_Opaque_Character` | Characters with SSS skin, hair, cloth |
| `MM_Masked_Standard` | Fences, grates, leaves, hair cards |
| `MM_Translucent_Standard` | Glass, water surfaces, UI elements |
| `MM_Emissive_Standard` | Glowing objects, magic effects, lava |
| `MM_Decal_Standard` | Projected decals (blood, scorch marks) |

### Material Rules
1. **No material may exceed 200 shader instructions** — check via `Shader Complexity` viewport.
2. **No material may use more than 8 texture samples** — use channel packing.
3. **Channel packing is mandatory:**
   - RGB = BaseColor/Albedo
   - R = Metallic, G = Roughness, B = AO (ORM map)
   - Normal map: BC5 compression
4. **Texture sizes:**
   - Characters: 4K (body), 2K (accessories)
   - Environment: 2K (large), 1K (props), 512 (trim sheets)
   - UI: Power of 2, no larger than 1K
5. **Virtual Texturing** for terrain layers (max 16 layers blended via Runtime Virtual Textures).
6. **No world-position offset** in materials unless approved (kills Nanite compatibility).

### Texture Format Rules
| Texture Type | Compression | sRGB |
|---|---|---|
| Base Color | BC1 (DXT1) | ✅ Yes |
| Normal Map | BC5 (2-channel) | ❌ No |
| ORM (AO/Rough/Metal) | BC1 (DXT1) | ❌ No |
| Emissive | BC1 or BC7 | ✅ Yes |
| UI / Icons | BC7 | ✅ Yes |
| HDR Textures | BC6H | ❌ No |

---

## 🌿 Foliage & Instanced Rendering

### Rules
1. **All repeated meshes (foliage, props, rocks) must use HISM** (Hierarchical Instanced Static Mesh) or Nanite.
2. **No foliage actor may have more than 50,000 instances** per component — split into sectors.
3. **Foliage LODs:**
   - LOD0: Full geometry (0-50m)
   - LOD1: 50% reduction (50-150m)
   - LOD2: Billboard or impostor (150m+)
4. **Wind animation** via World Position Offset — keep to ≤ 15 instructions in the material.
5. **Grass must use Landscape Grass Type** for automatic density management.

---

## ✨ Niagara (VFX) Budget

### Per-Frame Budgets
| VFX Category | Max GPU Cost | Max Particle Count |
|---|---|---|
| Combat VFX (per hit) | 0.5ms | 500 particles |
| Ambient VFX (per emitter) | 0.2ms | 200 particles |
| Weather (global) | 1.0ms | 5,000 particles |
| UI VFX | 0.1ms | 100 particles |
| **Total VFX Budget** | **2.0ms** | — |

### Niagara Rules
1. **GPU simulation** for all particle systems with >100 particles.
2. **LOD distances** on every emitter: disable at 100m+ unless critical (weather, fire).
3. **Scalability groups**: set Low/Medium/High/Epic budgets per system.
4. **No Niagara system should exceed 3 emitters** unless approved.
5. **Collision** uses `Depth Buffer` collision, NOT scene traces.
6. **Ribbon/beam VFX** limited to 50m max length.

---

## 📺 Temporal Super Resolution (TSR) / Upscaling

### Strategy
1. **TSR is the primary upscaler** — enabled by default for all platforms.
2. Internal render resolution: **1440p → TSR → 4K output** (saves ~40% GPU cost).
3. `r.TemporalAA.Upscaling 1` — enable in DefaultEngine.ini.
4. `r.TSR.ShadingRejection.Flickering 1` — reduce flickering on vegetation.
5. **Console settings** for platform-specific scalability:
   - Quality mode: 1800p → TSR → 4K
   - Performance mode: 1080p → TSR → 4K
6. **DLSS/FSR integration** for PC: use `NVIDIADLSSPlugin` and `AMDFidelityFXPlugin` when targeting those GPUs.

---

## 📦 Draw Call Management

### Rules
1. **Merge static meshes** that share materials and are near each other (use Merge Actor tool).
2. **Instanced Static Mesh (ISM)** for any mesh repeated >5 times in a scene.
3. **Materials** should share parent Master Materials to maximize draw call batching.
4. **Texture atlases** for small props that share the same material.
5. **Actor merging** via HLOD for distant objects.
6. **Target:** ≤ 2,500 draw calls at any time. Use `stat scenerendering` to monitor.

---

## 🔧 Profiling & Monitoring Commands

### Essential Console Commands
```
stat fps                    ; FPS counter
stat unit                   ; Game/Draw/GPU/RHIT thread times
stat scenerendering         ; Draw call count and render stats
stat gpu                    ; Per-pass GPU timing
stat Niagara                ; VFX costs
profilegpu                  ; Full GPU profile capture
r.Nanite.Visualize.Triangles 1 ; Nanite triangle density
r.Nanite.Visualize.Overdraw 1  ; Nanite overdraw
stat LumenScene             ; Lumen ray tracing costs
r.ShaderComplexity 1        ; Material instruction count heat map
```

### Profiling Workflow (Every Content Milestone)
1. Run `profilegpu` → identify the most expensive pass.
2. Run `stat scenerendering` → verify draw calls ≤ 2,500.
3. Run `stat unit` → verify Game ≤ 10ms, GPU ≤ 16.67ms.
4. Run `stat Niagara` → verify VFX ≤ 2.0ms.
5. Run Nanite visualization → check for non-Nanite static meshes.
6. Run `stat memory` → verify total memory ≤ 10GB.

---

## 🎯 Artist Onboarding Checklist

Before any artist contributes content, they must understand:

```
[ ] All static meshes must be Nanite-enabled (except translucent/masked)
[ ] All materials are Material Instances of the Master Material set
[ ] No material exceeds 200 shader instructions
[ ] Textures use correct compression and channel packing (ORM)
[ ] Foliage uses HISM with proper LODs
[ ] No mesh exceeds 4K textures without approval
[ ] All content tested with r.ShaderComplexity and Nanite visualization
[ ] Performance target: 60fps at 1440p internal resolution
```

---

## 🚫 HARD RULES (Violation = Content Rejected)

1. **No static mesh without Nanite** (unless masked/translucent)
2. **No material over 200 shader instructions**
3. **No texture over 4K resolution** without lead approval
4. **No more than 4 movable lights** in any scene
5. **No world-position offset** in Nanite materials
6. **No translucent overdraw** stacking more than 3 layers
7. **No Niagara system** without GPU simulation enabled (if >100 particles)
8. **No foliage** without HISM or Nanite
9. **No cascaded shadow maps** — VSM only
10. **No manual LODs on Nanite meshes** — trust the system


---
## FROM ORIGINAL FILE: STYLE_GUIDE.md
---

# AAA Multiplayer RPG — C++ Style Guide

## General Rules
1. Prefix all custom C++ classes and structs with `AAA`.
2. Follow standard Epic Games Unreal Engine coding standards.
3. Keep headers clean; forward declare whenever possible.
4. Always wrap single-line `if` statements in braces.
5. Use `TObjectPtr<T>` instead of raw pointers (`T*`) for UPROPERTY object references in Unreal 5.

## Modules
- `AAACore`: Types, Interfaces, Enums, Tags. Should not depend on other modules.
- `AAAGame`: Game Mode, Player Controller, HUD, Game State.
- `AAACharacter`: Character Base, Abilities, Movement logic.

## Documentation
- Use XML doc comments for all public functions, classes, and properties.
- Explain *why* a decision was made, not just *what* the code does.


---
## FROM ORIGINAL FILE: DATA_ASSETS.md
---

# Data Assets Guide

The project relies on Primary Data Assets derived from `UPrimaryDataAsset` to configure everything in the game. 

## 1. Character Settings (`UAAACharacterSettings`)
Used to define baseline rules for a Character (e.g., the Player).
- **Vitals**: Base Health, Stamina, Movement Speed.
- **Movement**: Walk Speed, Sprint Speed, Jump Force.
- **Abilities**: Starting Abilities and Effects (e.g., Base Health Regen).

## 2. Weapon Settings (`UAAAWeaponSettings`)
Defines the rules for Combat.
- **Visuals**: Static/Skeletal meshes and attachment sockets.
- **Combat**: Base Damage, Attack Speed, Reach/Range, Stamina Cost.
- **Animations**: Montages for Light/Heavy attacks.

## 3. Ability Settings (`UAAAAbilitySettings`)
Defines the UI and execution requirements of a Gameplay Ability.
- **UI Metadata**: Icon, Display Name, Description.
- **Requirements**: Costs (Stamina/Mana) and Cooldown definitions.

## 4. NPC Settings (`UAAANPCSettings`)
Defines humanoid AI.
- **Core**: Behavior Tree, Faction Tag.
- **Progression**: Level Scaling, Loot Tables.
- **Audio**: Aggro/Death voices.

## 5. Dragon Settings (`UAAADragonSettings`)
Defines massive creature AI.
- **Identity**: Element (Fire/Ice), Growth Stage (Hatchling/Elder).
- **Flight**: Max Speed, Acceleration, Takeoff Stamina Cost.
- **Mount**: Rider compatibility and Socket definitions.

## 6. Faction Settings (`UAAAFactionSettings`)
Defines diplomacy.
- **Relationships**: Friendly/Enemy faction tags and base reputation scores.
