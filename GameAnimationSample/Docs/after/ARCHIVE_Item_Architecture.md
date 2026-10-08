# ARCHIVE_Item_Architecture

> [!CAUTION]
> ## SUPERSEDED HISTORICAL ARCHIVE — DO NOT IMPLEMENT AGAINST THIS DOCUMENT
> This file contains historical design documentation from earlier development sessions. 
> Component ownership and serialization patterns described herein have been integrated into canonical architecture. Note that the SaveGame system contract is formally scheduled in **Phase 7.6 (`UAAASaveGame`)** of the Master Roadmap.
> Refer strictly to [PROJECT_OVERVIEW.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/PROJECT_OVERVIEW.md) and [ROADMAP.md](file:///a:/UNREAL%20PROJECT/Game/GameAnimationSample/Docs/ROADMAP.md) for canonical architecture.

*This file is a merged archive of older, granular documentation.* 



---
## FROM ORIGINAL FILE: ITEM_ARCHITECTURE.md
---

# Item Architecture

> Complete architectural reference for the AAA Item Ecosystem. This document covers the class hierarchy, module boundaries, and data flow for all item-related systems.

---

## Module Layout

```
AAAItems/          ← NEW module. All item definitions, instances, and types.
├── Public/
│   ├── AAAItemTypes.h           ← All enums (Category, Rarity, Quality, etc.)
│   ├── AAAItemDefinition.h      ← Base UPrimaryDataAsset for item archetypes
│   ├── AAAItemInstance.h        ← Runtime replicated UObject per owned item
│   └── DataAssets/
│       ├── AAAWeaponSettings.h
│       ├── AAAArmorSettings.h
│       ├── AAAConsumableSettings.h
│       ├── AAACraftingMaterialSettings.h
│       ├── AAAQuestItemSettings.h
│       └── AAAResourceSettings.h
└── Private/
    ├── AAAItemDefinition.cpp
    ├── AAAItemInstance.cpp
    └── DataAssets/
        ├── AAAWeaponSettings.cpp
        ├── AAAArmorSettings.cpp
        ├── AAAConsumableSettings.cpp
        ├── AAACraftingMaterialSettings.cpp
        ├── AAAQuestItemSettings.cpp
        └── AAAResourceSettings.cpp

AAAInventory/      ← EXISTING module. Components for inventory and equipment.
├── Public/
│   ├── AAAInventoryComponent.h
│   └── AAAEquipmentComponent.h
└── Private/
    ├── AAAInventoryComponent.cpp
    └── AAAEquipmentComponent.cpp
```

---

## Class Hierarchy

```
UPrimaryDataAsset
└── UAAAItemDefinition              ← Base item archetype (static data)
    ├── UAAAWeaponSettings          ← Weapon-specific properties
    ├── UAAAArmorSettings           ← Armor-specific properties
    ├── UAAAConsumableSettings      ← Consumable-specific properties
    ├── UAAACraftingMaterialSettings ← Crafting material properties
    ├── UAAAQuestItemSettings       ← Quest item properties
    └── UAAAResourceSettings        ← World resource properties

UObject
└── UAAAItemInstance                ← Runtime owned item (mutable state)

UActorComponent
├── UAAAInventoryComponent          ← Manages array of ItemInstances
└── UAAAEquipmentComponent          ← Manages equipment slot map
```

---

## Dependency Graph

```
AAACore ← AAAItems ← AAAInventory
```

- `AAAItems` depends on `AAACore` (for gameplay tags).
- `AAAInventory` depends on `AAAItems` (for definitions and instances).
- Neither depends on `AAACombat`, `AAACharacter`, or `AAAUI`.

---

## Enum Types

| Enum | Values | Purpose |
|---|---|---|
| `EAAAItemCategory` | Weapon, Armor, Consumable, QuestItem, CraftingMaterial, KeyItem, DragonEquipment, Resource, Currency, Book, SkillTome, Food, Potion, Miscellaneous | Broad item classification |
| `EAAAItemRarity` | Common → Divine, Developer | Loot tables, visual presentation |
| `EAAAItemQuality` | Poor → Legendary | Crafting quality |
| `EAAAWeaponType` | Sword, GreatSword, Dagger, etc. (15 types) | Animation set, damage formula |
| `EAAAArmorType` | Light, Medium, Heavy, Mage, Dragon | Movement penalties, resistances |
| `EAAAEquipmentSlot` | Head, Chest, Legs, Feet, Gloves, RingLeft, RingRight, Necklace, MainWeapon, Offhand, Bow, Shield, DragonSaddle, DragonArmor, DragonAccessory | Strongly typed slot IDs |


---
## FROM ORIGINAL FILE: ITEM_LIFECYCLE.md
---

# Item Lifecycle

> Tracks the full life of an item from creation to destruction.

---

## Lifecycle Stages

```
1. DEFINITION (Design Time)
   └── Designer creates UAAAItemDefinition Data Asset in Editor
       ├── Sets static properties (Name, Icon, Weight, etc.)
       └── Saved as .uasset in Content/AAA/Items/

2. INSTANTIATION (Runtime)
   └── UAAAInventoryComponent::AddItem(Definition, Count)
       ├── Creates UAAAItemInstance UObject
       ├── Generates unique FGuid
       ├── Sets StackCount, Durability, Quality
       └── Adds to Items array

3. OWNERSHIP
   └── ItemInstance lives in InventoryComponent's Items array
       ├── OwnerActor is set
       ├── Replicated to all clients
       └── Can be queried, moved, split, merged

4. EQUIPMENT (Optional)
   └── UAAAEquipmentComponent::EquipItem(ItemInstance)
       ├── Item moves from "inventory storage" to "equipped slot"
       └── Delegates fire for visual/gameplay response

5. CONSUMPTION (Optional)
   └── UAAAInventoryComponent::ConsumeItem(ItemInstance)
       ├── Decrements StackCount
       └── Removes ItemInstance if StackCount reaches 0

6. DESTRUCTION
   └── UAAAInventoryComponent::RemoveItem(ItemInstance)
       ├── Removed from Items array
       ├── OwnerActor cleared
       ├── Delegates fire
       └── UObject is garbage collected
```

---

## Key Rules

1. **Definitions are immutable.** They are Data Assets authored by designers. Never modify them at runtime.
2. **Instances are mutable.** They hold stack counts, durability, and dynamic tags that change during gameplay.
3. **Every Instance has a GUID.** This ensures save/load stability and unique network identity.
4. **Authority owns mutations.** All Add/Remove/Split/Merge/Equip/Unequip operations are server-authoritative.


---
## FROM ORIGINAL FILE: ITEM_NAMING.md
---

# Item Naming Conventions

> Standard naming rules for all item-related assets, classes, and files.

---

## C++ Classes

| Pattern | Example | Description |
|---|---|---|
| `UAAA<Name>` | `UAAAItemDefinition` | All UObject-derived classes |
| `EAAA<Name>` | `EAAAItemCategory` | All enums |
| `FAAA<Name>` | `FAAAOnItemAdded` | All structs and delegates |

---

## Data Assets (Content Browser)

| Pattern | Example | Description |
|---|---|---|
| `DA_<Name>` | `DA_IronSword` | Item Definition data assets |
| `DA_Weapon_<Name>` | `DA_Weapon_IronSword` | Weapon settings |
| `DA_Armor_<Name>` | `DA_Armor_IronChestplate` | Armor settings |
| `DA_Consumable_<Name>` | `DA_Consumable_HealthPotion` | Consumable settings |
| `DA_Resource_<Name>` | `DA_Resource_IronOre` | Resource settings |
| `DA_Quest_<Name>` | `DA_Quest_AncientKey` | Quest item settings |
| `DA_Crafting_<Name>` | `DA_Crafting_DragonScale` | Crafting material settings |

---

## Content Folder Structure

```
Content/AAA/Items/
├── Weapons/
│   ├── Swords/
│   ├── Bows/
│   ├── Staffs/
│   └── ...
├── Armor/
│   ├── Light/
│   ├── Medium/
│   ├── Heavy/
│   └── Dragon/
├── Consumables/
│   ├── Potions/
│   └── Food/
├── Resources/
├── QuestItems/
├── CraftingMaterials/
└── DragonEquipment/
```

---

## Gameplay Tags

| Pattern | Example | Description |
|---|---|---|
| `Item.<Category>` | `Item.Weapon` | Item category tags |
| `Item.<Modifier>` | `Item.TwoHanded` | Item modifier tags |
| `Equipment.<Slot>` | `Equipment.MainHand` | Equipment slot tags |

---

## Source Files

| Pattern | Example | Module |
|---|---|---|
| `AAAItemTypes.h` | Enums only | `AAAItems` |
| `AAAItemDefinition.h/.cpp` | Base data asset | `AAAItems` |
| `AAA<Type>Settings.h/.cpp` | Specialized data assets | `AAAItems` |
| `AAAItemInstance.h/.cpp` | Runtime instance | `AAAItems` |
| `AAAInventoryComponent.h/.cpp` | Inventory logic | `AAAInventory` |
| `AAAEquipmentComponent.h/.cpp` | Equipment logic | `AAAInventory` |


---
## FROM ORIGINAL FILE: ITEM_REPLICATION.md
---

# Item Replication

> Documents how the item ecosystem replicates across the network for multiplayer.

---

## Replication Strategy

### Authority Model
All item mutations (Add, Remove, Split, Merge, Equip, Unequip) are **server-authoritative**. Every mutating function checks `GetOwner()->HasAuthority()` before executing. Clients never directly modify item state.

### Replicated Properties

#### UAAAItemInstance
| Property | Replication | Description |
|---|---|---|
| `ItemDefinition` | `DOREPLIFETIME` | The data asset reference |
| `InstanceID` | `DOREPLIFETIME` | Unique FGuid |
| `StackCount` | `DOREPLIFETIME` | Current stack size |
| `CurrentDurability` | `DOREPLIFETIME` | Durability state |
| `Quality` | `DOREPLIFETIME` | Quality roll |
| `DynamicTags` | `DOREPLIFETIME` | Enchantments, curses |

#### UAAAInventoryComponent
| Property | Replication | Description |
|---|---|---|
| `Items` | `DOREPLIFETIME` | TArray of ItemInstances |

#### UAAAEquipmentComponent
| Property | Replication | Description |
|---|---|---|
| `EquippedItems` | `DOREPLIFETIME` | TMap of Slot → ItemInstance |

### Network Flow

```
Client Request (e.g., "Equip Sword")
    │
    ▼
Server RPC (future phase)
    │
    ▼
Server validates request
    │
    ├── Authority check passes
    ├── CanEquipToSlot validates
    ├── EquippedItems map updated
    └── Replication pushes to all clients
        └── Clients see equipment change
```

---

## Prediction Strategy (Future)

The current implementation is **non-predictive**. Clients must wait for server confirmation before seeing changes. A future phase may add client-side prediction for:
- Inventory reordering
- Equipment visual preview
- Stack merging feedback

This will use Unreal's Push Model or custom RepNotify callbacks.

---

## Important Notes

1. `UAAAItemInstance` overrides `IsSupportedForNetworking()` to return `true`, allowing it to be replicated as a UObject subobject.
2. `TObjectPtr<UAAAItemInstance>` is used instead of raw pointers for proper GC and replication support.
3. Delegates (`OnItemAdded`, `OnEquipmentChanged`, etc.) are **local-only**. They fire on the server but can also be triggered on clients via RepNotify in future phases.


---
## FROM ORIGINAL FILE: SAVEGAME_INVENTORY.md
---

# SaveGame: Inventory & Equipment

> Documents how the inventory and equipment systems serialize for save/load.

---

## SaveGame Properties

All mutable item state is marked with the `SaveGame` UPROPERTY specifier, making it automatically compatible with Unreal's serialization system.

### UAAAItemInstance (SaveGame Fields)
| Property | Type | Description |
|---|---|---|
| `ItemDefinition` | `TObjectPtr<UAAAItemDefinition>` | Reference to the data asset (resolved by path) |
| `InstanceID` | `FGuid` | Unique identifier for this specific instance |
| `StackCount` | `int32` | Current stack size |
| `CurrentDurability` | `float` | Current durability value |
| `Quality` | `EAAAItemQuality` | Quality roll |
| `DynamicTags` | `FGameplayTagContainer` | Runtime-applied tags |

### UAAAInventoryComponent (SaveGame Fields)
| Property | Type | Description |
|---|---|---|
| `Items` | `TArray<TObjectPtr<UAAAItemInstance>>` | All owned item instances |

### UAAAEquipmentComponent (SaveGame Fields)
| Property | Type | Description |
|---|---|---|
| `EquippedItems` | `TMap<EAAAEquipmentSlot, TObjectPtr<UAAAItemInstance>>` | Slot-to-item mapping |

---

## Serialization Flow

```
Save:
  1. USaveGame subclass iterates over InventoryComponent->Items
  2. For each UAAAItemInstance, serialize:
     - ItemDefinition path (FPrimaryAssetId or FSoftObjectPath)
     - InstanceID (FGuid)
     - StackCount, Durability, Quality, DynamicTags
  3. Serialize EquipmentComponent->EquippedItems similarly
  4. Write to disk via UGameplayStatics::SaveGameToSlot

Load:
  1. Load USaveGame from slot
  2. For each serialized item:
     - Resolve ItemDefinition from asset path
     - Recreate UAAAItemInstance with saved state
     - Restore InstanceID (do NOT generate new GUID)
  3. Populate InventoryComponent->Items
  4. Populate EquipmentComponent->EquippedItems
```

---

## Dragon Inventory

Dragon mounts will have their own `UAAAInventoryComponent` and `UAAAEquipmentComponent` attached to the dragon actor. The serialization flow is identical — the save system simply iterates over the dragon's components the same way it does the player's.

---

## Important Notes

1. **Do NOT generate new GUIDs on load.** The `InstanceID` must persist across save/load cycles to maintain identity.
2. **Asset paths must be stable.** Moving a Data Asset in the Content Browser will break save files unless migration logic is added.
3. **The save system itself is NOT implemented in this phase.** This document defines the contract that a future `AAASaveGame` class must fulfill.
