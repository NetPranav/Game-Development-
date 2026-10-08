# AAA Multiplayer Action RPG — Known Bugs, Issues & Post-Mortem Ledger

> Master register of resolved critical architectural bugs, known technical constraints, and active watch-items across the project lifecycle.

---

## 1. Active Technical Watch-Items & Considerations

| Issue ID | System | Description & Technical Impact | Workaround / Planned Resolution | Status |
| :--- | :--- | :--- | :--- | :--- |
| **ACT-01** | **AAACombat** | **Weapon Mesh Missing Sockets:** If an imported weapon mesh lacks `TraceStart` or `TraceEnd` sockets, hit detection previously failed silently with only a subtle log warning. | **RESOLVED:** Implemented in `AAAAWeaponActor::PerformTrace()`: calculates bounding box extents from mesh bounds as fallback, and fires loud on-screen red debug warning (`GEngine->AddOnScreenDebugMessage`) + `UE_LOG` in non-shipping builds so missing sockets cannot be missed during playtesting. | 🟢 Mitigated & Gated |
| **ACT-02** | **AAAAbility / Mover** | **Root Motion Interaction with Mover 2.0:** Mover 2.0 uses kinematic character movement simulation. Standard character root motion from montages must align with Mover's movement modes during dodges and attacks. | In `UAAAGameplayAbility_Dodge`, utilize grounded directional velocity impulses or verify Mover's Root Motion Montage Mode is active during ability execution. | 🟡 Active Consideration |
| **ACT-03** | **AAAUI** | **UMG Animation Stalling When Paused:** When opening `WBP_GameMenu` during zero world dilation (`bExecuteWhenPaused = true`), UMG timeline animations stall unless explicitly configured. | Every UMG animation inside `WBP_GameMenu` must have its playback mode set to `Play When Paused` in the widget animation details panel. | 🟢 Documented Standard |
| **ACT-04** | **MetaHuman** | **LOD Sync on Hero MetaHuman:** High-poly facial grooms and face meshes can cause editor frame hitches during rapid camera rotations. | Verify `LODSyncComponent` on `BP_PlayerCharacter` is locked to LOD 1 or 2 during gameplay development to ensure consistent 60+ FPS. | 🟡 Active Consideration |
| **SEC-01** | **Security / CI** | **Remote Control & Python Plugin RCE Risk:** Unauthenticated HTTP endpoint (`localhost:30010`) used for editor automation could expose arbitrary code execution if compiled into packaged builds. | Explicitly gate Remote Control API and Python Editor Script Plugin to `"TargetAllowList": ["Editor"]` in `.uproject`. Enforce that neither plugin is ever packaged into `Shipping` or `Test` configurations. | 🟢 Security Standard Enforced |

---

## 2. Resolved Architectural Bugs & Post-Mortems

### BUG-01: Blueprint Reflection Desync ("Derives from Invalid Class")
- **Discovered:** Phase 4 / 5
- **Symptoms:** `BP_PlayerCharacter` and child Blueprints failed to compile on editor load with error: `"Derives from invalid class 'AAAAPlayerPawn'"`.
- **Root Cause:** C++ module loading phase in `GameAnimationSample.uproject` was set to `Default`. Blueprints were being loaded by the asset registry before the custom C++ reflection data was fully initialized by the Unreal Header Tool (UHT).
- **Resolution:**
  1. Updated `GameAnimationSample.uproject` to set `LoadingPhase: "PreDefault"` for `AAACore`, `AAACharacter`, `AAAAbility`, and `AAAGame`.
  2. Deleted `Intermediate/` and `Binaries/` and performed a clean rebuild in Visual Studio, permanently fixing the reflection cache.

---

### BUG-02: Startup 100% Memory Crash Loop
- **Discovered:** Phase 5
- **Symptoms:** Unreal Editor froze and crashed during level load, consuming 100% of system RAM (32GB+) within 10 seconds.
- **Root Cause:** A rogue UI rendering command inside an early HUD construction tick was recursively allocating Slate brush resources without garbage collection release during editor startup.
- **Resolution:**
  1. Stripped all procedural slate brush allocations from C++ HUD constructors.
  2. Stabilized Editor memory limit settings in `DefaultEngine.ini`.
  3. Relocated widget instantiation to explicit client-side `BeginPlay` events.

---

### BUG-03: Live Link Face Mocap Pin Corruption
- **Discovered:** Phase 4
- **Symptoms:** Live Link facial mocap nodes in `SandboxCharacter_Mover_ABP` and helper functions in `BFL_HelpfulFunctions` lost connections, showing red disconnected pins after C++ hot-reloads.
- **Root Cause:** Unreal Engine's Live Coding system dynamically swaps DLL addresses in memory, which invalidates Blueprint node GUID pins that reference C++ helper signatures.
- **Resolution:**
  1. Reconstructed the affected Blueprint nodes manually.
  2. Utilized the "Auto-Pin" trick to restore pin topology.
  3. Established development protocol: Always perform full C++ builds with the Editor closed when altering function signatures or component hierarchies.

---

### BUG-04: Circular Module Dependency Deadlock
- **Discovered:** Phase 5
- **Symptoms:** Linker failure during build: `AAAGame` depended on `AAACharacter`, `AAACharacter` depended on `AAAAbility`, and `AAAAbility` depended on `AAAGame` via `AAADataLibrary`.
- **Root Cause:** Shared helper functions and data assets were placed in high-level modules and referenced across sibling modules.
- **Resolution:**
  1. Decomposed `AAADataLibrary` into lower-level modules (`AAACore` and `AAAItems`).
  2. Re-architected module dependency graph to follow strict unidirectional flow (`AAACore` $\rightarrow$ `AAAItems` $\rightarrow$ `AAACharacter` $\rightarrow$ `AAACombat` $\rightarrow$ `AAAAbility` $\rightarrow$ `AAAGame`).

---

### BUG-05: Missing GameplayAbilities Plugin Linker Error (`LNK2019`)
- **Discovered:** Phase 3.5
- **Symptoms:** Visual Studio build failed with unresolved external symbol errors referencing `UAbilitySystemComponent` and `FGameplayEffectContext`.
- **Root Cause:** While `AAAAbility.Build.cs` included `"GameplayAbilities"`, the plugin was not explicitly enabled in `GameAnimationSample.uproject`.
- **Resolution:** Added `"GameplayAbilities"` plugin to the `.uproject` descriptor file:
  ```json
  {
    "Name": "GameplayAbilities",
    "Enabled": true
  }
  ```
