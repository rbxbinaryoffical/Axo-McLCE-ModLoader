# Forge 1.8.9 vs Axo Mod Loader — Gap Analysis & Roadmap

> **Reference:** [Forge 1.8.9-11.15.1.2318 JavaDocs](https://skmedix.github.io/ForgeJavaDocs/javadoc/forge/1.8.9-11.15.1.2318/)
>
> **Axo Mod Loader:** C-stable ABI mod loader for Minecraft Legacy Console Edition (LCE)
>
> **Goal:** Achieve feature parity with Forge where possible, while maintaining compatibility and minimizing changes to LCE's core functionality.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Current Axo Capabilities](#2-current-axo-capabilities)
3. [Registry System](#3-registry-system)
4. [Event System](#4-event-system)
5. [Block System](#5-block-system)
6. [Item System](#6-item-system)
7. [Entity System](#7-entity-system)
8. [Tile Entity System](#8-tile-entity-system)
9. [World Generation](#9-world-generation)
10. [Recipe System](#10-recipe-system)
11. [Capability System](#11-capability-system)
12. [Fluid System](#12-fluid-system)
13. [Ore Dictionary](#13-ore-dictionary)
14. [Networking](#14-networking)
15. [Creative Tabs](#15-creative-tabs)
16. [Sound & Particles](#16-sound--particles)
17. [GUI / Container System](#17-gui--container-system)
18. [Terrain Generation Events](#18-terrain-generation-events)
19. [Engine Modifications Required](#19-engine-modifications-required)
20. [Implementation Priority & Roadmap](#20-implementation-priority--roadmap)

---

## 1. Executive Summary

### Axo Today (v1/v2 ABI)

| Feature | Status |
|---------|--------|
| Custom Blocks (basic) | ✅ Implemented |
| Custom Items (basic) | ✅ Implemented |
| Custom Food | ✅ Implemented |
| Custom Crops (8-stage) | ✅ Implemented |
| Custom Biomes | ✅ Implemented |
| Shaped/Shapeless/Furnace Recipes | ✅ Implemented |
| World Gen (ore veins, surface patches) | ✅ Implemented |
| JSON Block Models | ✅ Implemented |
| Entity Spawning (vanilla IDs only) | ✅ Implemented |
| Item Drops / Lightning / TNT / Falling Blocks | ✅ Implemented |
| Per-face Block Textures | ✅ Implemented |
| `onDestroyed` Block Callback | ✅ Implemented |
| Mod Lifecycle (PreInit/MidInit/Init/Tick/Shutdown) | ✅ Implemented |
| Texture Atlas Hot-patching | ✅ Implemented |
| SEH Crash Protection | ✅ Implemented |

### Critical Gaps vs Forge

| Forge System | Axo Status | Priority |
|-------------|------------|----------|
| **Event Bus** (50+ event types) | ❌ Not started | 🔴 Critical |
| **Block States / Metadata** | ❌ Not started | 🔴 Critical |
| **Tile Entities** | ❌ Not started | 🔴 Critical |
| **Custom Entities** (registration, AI, rendering) | ❌ Not started | 🔴 Critical |
| **Item Use Callbacks** (`onUse`/`onUseOn` wired) | ⚠️ Fields exist, not wired | 🟡 High |
| **Block Interaction Callbacks** | ❌ Not started | 🟡 High |
| **Networking / Packets** | ❌ Not started | 🟡 High |
| **Capability System** | ❌ Not started | 🟡 High |
| **Custom Armor** | ❌ Not started | 🟡 High |
| **Enchantment System** | ❌ Not started | 🟡 High |
| **Fluid System** | ❌ Not started | 🟠 Medium |
| **Ore Dictionary** | ❌ Not started | 🟠 Medium |
| **GUI / Container System** | ❌ Not started | 🟠 Medium |
| **Custom Sounds** | ❌ Not started | 🟠 Medium |
| **Custom Particles** | ❌ Not started | 🔵 Low |
| **Terrain Gen Events** | ❌ Not started | 🔵 Low |
| **String-based Registry with Namespaces** | ❌ Not started | 🟠 Medium |
| **NBT Data Support** | ❌ Not started | 🟡 High |
| **Block Ticking (random/scheduled)** | ❌ Not started | 🟡 High |
| **Redstone Integration** | ❌ Not started | 🟠 Medium |

---

## 2. Current Axo Capabilities

### API Table (11 functions)

```
AxoAPITable {
    Log, RegisterItem, RegisterBlock, RegisterRecipe,
    RegisterBiome, RegisterCrop,
    SpawnEntity, DropItem, StrikeLightning, SpawnTnt, SpawnFallingBlock
}
```

### ID Ranges

| Type | Range | Notes |
|------|-------|-------|
| Items | 422 – 31999 | Auto-assigned from 422 upward |
| Blocks | 174 – 255 | Only 82 slots available (hard limit) |
| Biomes | 23 – 255 | Auto-assigned from 23 upward |

### Mod DLL Exports

| Export | Required | Signature |
|--------|----------|-----------|
| `AxoMain` | ✅ Yes | `void AxoMain(AxoAPITable* api)` |
| `AxoTick` | ❌ Optional | `void AxoTick()` |
| `AxoShutdown` | ❌ Optional | `void AxoShutdown()` |

### Block Callbacks (Current)

- `onDestroyed(x, y, z, Level*, Player*, ItemInstance*)` — fires when block is broken

### Item Callbacks (Current)

- `onUse()` / `onUseOn()` — **fields exist in AxoItemDef but are NOT wired** in AxoItemImpl.cpp

### Name Resolution

- 158 vanilla block names → tile IDs
- 165 vanilla item names → item IDs

---

## 3. Registry System

### Forge (`GameRegistry` + `FMLControlledNamespacedRegistry`)

Forge provides a comprehensive, string-namespaced registry system:

| Feature | Description |
|---------|-------------|
| `GameRegistry.registerBlock(Block, name)` | Register block with string ID |
| `GameRegistry.registerItem(Item, name)` | Register item with string ID |
| `GameRegistry.registerTileEntity(Class, name)` | Register tile entity class |
| `GameRegistry.registerWorldGenerator(IWorldGenerator, weight)` | Register world generator with priority |
| `EntityRegistry.registerModEntity(Class, name, id, mod, trackingRange, updateFreq, sendsVelocity)` | Full entity registration |
| `GameRegistry.findBlock(modId, name)` / `findItem(modId, name)` | Lookup by `modid:name` |
| `@ObjectHolder` / `@ItemStackHolder` | Automatic field injection from registry |
| `RegistryDelegate<T>` | Safe alias-aware references |
| Namespacing: `modid:blockname` | Prevents cross-mod collisions |

### Axo (Current)

| Feature | Description |
|---------|-------------|
| `RegisterBlock(AxoBlockDef*)` | Numeric ID (auto or explicit), name for internal lookup |
| `RegisterItem(AxoItemDef*)` | Numeric ID (auto or explicit), name for internal lookup |
| `RegisterRecipe`, `RegisterBiome`, `RegisterCrop` | Numeric ID based |
| No namespacing | Names are flat strings, collision risk |
| No `findBlock`/`findItem` from mod code | Mods cannot look up other mods' blocks |

### Required Changes

1. **Add string-based registry keys** with `modid:name` namespacing
   - Extend `AxoBlockDef.name` to support `"mymod:my_block"` format
   - Store a `std::unordered_map<std::string, int>` for name → ID lookups
   - Expose `FindBlockByName(const char* fullName) → int` and `FindItemByName(const char* fullName) → int` in the API table

2. **Add `RegisterWorldGenerator` to API table**
   - Currently world gen is baked into `AxoBlockDef.spawn` — good for ore/surface but insufficient for structures
   - New function: `RegisterWorldGenerator(AxoWorldGenCallback, int priority)`

3. **Add `RegisterTileEntity` to API table** (see §8)

4. **Add `RegisterEntity` to API table** (see §7)

### Engine Modifications

- **AxoAPI.h**: Add `FindBlockByName`, `FindItemByName`, `RegisterWorldGenerator`, `RegisterEntity`, `RegisterTileEntity` to `AxoAPITable`
- **AxoAPI.cpp**: Implement name → ID reverse-lookup maps, expose to API table
- **AxoModLoader.cpp**: Add `modId` prefix to all registrations during mod loading

---

## 4. Event System

### Forge (`EventBus`)

Forge's event system is its **most critical feature** — it allows mods to intercept and modify virtually every game behavior without touching engine code.

#### Core Architecture

```java
MinecraftForge.EVENT_BUS.register(handler);

@SubscribeEvent(priority = EventPriority.NORMAL)
public void onEvent(SomeEvent event) { ... }
```

- `Event` base class with `Result` (ALLOW / DEFAULT / DENY) and `@Cancelable`
- `EventPriority` (HIGHEST → LOWEST)
- `EventBus.post(event)` returns true if cancelled

#### Entity Events (13 types)

| Event | Description |
|-------|-------------|
| `EntityEvent.EntityConstructing` | Entity being created |
| `EntityEvent.CanUpdate` | Can entity update this tick |
| `EntityEvent.EnteringChunk` | Entity entering new chunk |
| `EntityJoinWorldEvent` | Entity added to world (cancelable) |
| `EntityMountEvent` | Entity mount/dismount |
| `EntityStruckByLightningEvent` | Lightning strike |
| `EntityTravelToDimensionEvent` | Dimension travel |
| `PlaySoundAtEntityEvent` | Sound played at entity |

#### Living Entity Events (14 types)

| Event | Description |
|-------|-------------|
| `LivingAttackEvent` | Living entity attacked (cancelable) |
| `LivingHurtEvent` | Living entity taking damage (modifiable) |
| `LivingDeathEvent` | Entity dies (cancelable) |
| `LivingDropsEvent` | Death drops (modifiable) |
| `LivingFallEvent` | Fall damage (modifiable) |
| `LivingHealEvent` | Healing (modifiable) |
| `LivingSetAttackTargetEvent` | AI target change |
| `LivingSpawnEvent.CheckSpawn` | Mob spawn attempt |
| `LivingSpawnEvent.SpecialSpawn` | Spawner spawn |
| `LivingSpawnEvent.AllowDespawn` | Despawn check |
| `LivingEvent.LivingUpdateEvent` | Per-tick update |
| `LivingEvent.LivingJumpEvent` | Entity jumps |
| `LivingExperienceDropEvent` | XP drop amount |
| `EnderTeleportEvent` | Ender teleport |

#### Player Events (25+ types)

| Event | Description |
|-------|-------------|
| `PlayerInteractEvent` | Right-click block/air |
| `AttackEntityEvent` | Left-click entity |
| `EntityInteractEvent` | Right-click entity |
| `PlayerEvent.BreakSpeed` | Mining speed modifier |
| `PlayerEvent.HarvestCheck` | Can harvest block |
| `PlayerUseItemEvent.Start/Tick/Stop/Finish` | Item use lifecycle |
| `PlayerDestroyItemEvent` | Tool breaks |
| `PlayerDropsEvent` | Death inventory drops |
| `EntityItemPickupEvent` | Item pickup |
| `FillBucketEvent` | Bucket fill |
| `BonemealEvent` | Bone meal use |
| `UseHoeEvent` | Hoe use |
| `ArrowNockEvent` / `ArrowLooseEvent` | Bow use |
| `AchievementEvent` | Achievement earned |
| `PlayerSleepInBedEvent` / `PlayerWakeUpEvent` | Sleep cycle |
| `PlayerSetSpawnEvent` | Spawn point change |
| `PlayerOpenContainerEvent` | Container open |
| `PlayerPickupXpEvent` | XP orb pickup |
| `PlayerFlyableFallEvent` | Flying fall |
| `PlayerEvent.Clone` | Respawn data copy |
| `PlayerEvent.LoadFromFile` / `SaveToFile` | Player data persistence |
| `ItemTooltipEvent` | Tooltip text |

#### World Events (12 types)

| Event | Description |
|-------|-------------|
| `WorldEvent.Load` / `Save` / `Unload` | World lifecycle |
| `WorldEvent.CreateSpawnPosition` | Spawn point generation |
| `WorldEvent.PotentialSpawns` | Mob spawn list modification |
| `BlockEvent.BreakEvent` | Block broken (cancelable) |
| `BlockEvent.PlaceEvent` | Block placed (cancelable) |
| `BlockEvent.MultiPlaceEvent` | Multi-block placement |
| `BlockEvent.HarvestDropsEvent` | Drop list modification |
| `BlockEvent.NeighborNotifyEvent` | Block update |
| `ExplosionEvent.Start` / `Detonate` | Explosion phases |
| `ChunkEvent.Load` / `Unload` | Chunk lifecycle |
| `NoteBlockEvent.Play` / `Change` | Note block |

### Axo (Current)

**No event system exists.** The only "callback" is `AxoBlockDef.onDestroyed`.

### Required Changes

1. **Design a C-compatible event bus**

   Since Axo uses a C-stable ABI, the event system must use function pointers rather than Java annotations:

   ```c
   // Event IDs
   enum AxoEventType {
       AXO_EVENT_BLOCK_BREAK,
       AXO_EVENT_BLOCK_PLACE,
       AXO_EVENT_ENTITY_HURT,
       AXO_EVENT_ENTITY_DEATH,
       AXO_EVENT_PLAYER_INTERACT,
       AXO_EVENT_PLAYER_ATTACK,
       AXO_EVENT_PLAYER_TICK,
       AXO_EVENT_WORLD_LOAD,
       AXO_EVENT_WORLD_TICK,
       AXO_EVENT_EXPLOSION,
       AXO_EVENT_ENTITY_SPAWN,
       AXO_EVENT_ITEM_PICKUP,
       AXO_EVENT_LIVING_JUMP,
       AXO_EVENT_LIVING_FALL,
       // ...
   };

   // Generic event data
   struct AxoEvent {
       int  eventType;
       bool cancelled;
       // Union or tagged struct with event-specific data
   };

   // Registration
   typedef bool (*AxoEventHandler)(AxoEvent* event);
   bool RegisterEventHandler(int eventType, AxoEventHandler handler, int priority);
   ```

2. **Priority-based dispatch**: Handlers sorted by priority, early-out on cancel

3. **Phase 1 events** (most impactful, lowest engine changes):
   - `BLOCK_BREAK` — hook into `Tile::playerDestroy` / `Level::removeTile`
   - `BLOCK_PLACE` — hook into `Level::setTile` from player actions
   - `ENTITY_HURT` — hook into `Entity::hurt`
   - `ENTITY_DEATH` — hook into `Mob::die`
   - `PLAYER_INTERACT` — hook into player right-click handler
   - `PLAYER_ATTACK` — hook into player left-click handler
   - `WORLD_TICK` — hook into `Level::tick`

4. **Phase 2 events** (deeper engine integration):
   - `ENTITY_SPAWN`, `ENTITY_JOIN_WORLD`
   - `LIVING_UPDATE`, `LIVING_JUMP`, `LIVING_FALL`
   - `PLAYER_USE_ITEM` lifecycle
   - `EXPLOSION_START`, `EXPLOSION_DETONATE`
   - `BLOCK_HARVEST_DROPS`, `BLOCK_NEIGHBOR_NOTIFY`
   - `CHUNK_LOAD`, `CHUNK_UNLOAD`
   - `WORLD_LOAD`, `WORLD_SAVE`

### Engine Modifications

- **LCE Engine**: Add event dispatch calls at ~20 key points in the game loop
- **AxoAPI.h**: Add `AxoEventType` enum, `AxoEvent` struct hierarchy, `RegisterEventHandler` to API table
- **AxoAPI.cpp**: Implement event bus with priority queues per event type
- **AxoModLoader.cpp**: Initialize event bus before mod loading, dispatch lifecycle events

---

## 5. Block System

### Forge `Block` (100+ overridable methods)

#### Block States & Metadata

| Feature | Forge Method | Description |
|---------|-------------|-------------|
| Block State | `IBlockState getStateFromMeta(int)` | Convert 0-15 metadata to state |
| Metadata | `int getMetaFromState(IBlockState)` | Convert state to 0-15 for saving |
| Actual State | `IBlockState getActualState(IBlockState, IBlockAccess, BlockPos)` | Context-dependent state (e.g. fence connections) |
| State Properties | `BlockState createBlockState()` | Define properties (facing, powered, etc.) |
| Default State | `IBlockState getDefaultState()` | Initial block state |

#### Block Behavior Callbacks

| Category | Forge Methods | Axo Status |
|----------|--------------|------------|
| **Placement** | `canPlaceBlockAt`, `canPlaceBlockOnSide`, `onBlockPlacedBy`, `onBlockAdded` | Only `canBePlacedOnlyOn` (string) |
| **Breaking** | `onBlockDestroyedByPlayer`, `onBlockDestroyedByExplosion`, `onBlockHarvested`, `harvestBlock` | Only `onDestroyed` callback |
| **Interaction** | `onBlockActivated` (right-click), `onBlockClicked` (left-click) | ❌ Missing |
| **Ticking** | `updateTick` (scheduled), `randomTick` (random), `tickRate` | ❌ Missing |
| **Neighbor Updates** | `onNeighborBlockChange`, `onNeighborChange` | ❌ Missing |
| **Entity Collision** | `onEntityCollidedWithBlock`, `onEntityWalk`, `onFallenUpon` | ❌ Missing |
| **Redstone** | `canProvidePower`, `getWeakPower`, `getStrongPower`, `canConnectRedstone`, `shouldCheckWeakPower` | ❌ Missing |
| **Fire** | `isFlammable`, `getFlammability`, `getFireSpreadSpeed`, `isFireSource`, `isBurning` | ❌ Missing |
| **Plants** | `canSustainPlant`, `onPlantGrow`, `isFertile` | Crops only via `AxoCropDef` |
| **Harvesting** | `canHarvestBlock`, `canSilkHarvest`, `getDrops`, `getSilkTouchDrop`, `getExpDrop`, `setHarvestLevel`, `getHarvestTool` | Only `dropItemName` / `dropCount` |
| **Rendering** | `canRenderInLayer`, `getRenderType`, `getBlockLayer`, `isOpaqueCube`, `isFullCube`, `isVisuallyOpaque`, `doesSideBlockRendering` | Basic `renderShape` enum |
| **Beds** | `isBed`, `setBedOccupied`, `getBedSpawnPosition`, `getBedDirection` | ❌ Missing |
| **Ladders** | `isLadder` | ❌ Missing |
| **Beacons** | `isBeaconBase` | ❌ Missing |
| **Enchanting** | `getEnchantPowerBonus` | ❌ Missing |
| **Rotation** | `rotateBlock`, `getBlockRotation`, `getBlockMirror` | ❌ Missing |
| **Explosion** | `getExplosionResistance`, `canDropFromExplosion`, `onBlockExploded` | Only `resistance` field |
| **Light** | `getLightValue`, `getLightOpacity` | ❌ Missing |
| **Comparator** | `hasComparatorInputOverride`, `getComparatorInputOverride` | ❌ Missing |
| **Tile Entity** | `hasTileEntity`, `createTileEntity` | ❌ Missing |

### Axo `AxoBlockDef` (Current)

```c
struct AxoBlockDef {
    id, iconName, name, material, hardness, resistance,
    creativeTab, dropItemName, dropCount, renderShape,
    noCollision, canBeBrokenByHand, canBePlacedOnlyOn,
    customModel, hasDifferentSides,
    iconTop/Bottom/North/South/East/West,
    spawn (AxoBlockSpawnDef),
    onDestroyed  // only callback
};
```

### Required Changes

#### Phase 1 — Essential Block Callbacks

Add to `AxoBlockDef`:

```c
// Interaction
void (*onActivated)(int x, int y, int z, Level*, Player*);     // right-click
void (*onClicked)(int x, int y, int z, Level*, Player*);       // left-click
void (*onPlaced)(int x, int y, int z, Level*, Player*);        // after placement
void (*onNeighborChanged)(int x, int y, int z, Level*, int nx, int ny, int nz);

// Ticking
bool  ticksRandomly;
int   tickRate;            // scheduled tick interval
void (*onTick)(int x, int y, int z, Level*);          // scheduled tick
void (*onRandomTick)(int x, int y, int z, Level*);    // random tick

// Entity collision
void (*onEntityWalkOn)(int x, int y, int z, Level*, void* entity);
void (*onFallenUpon)(int x, int y, int z, Level*, void* entity, float distance);

// Light
int   lightEmission;       // 0-15
int   lightOpacity;        // 0-15
```

#### Phase 2 — Block States

```c
// Simplified block state system for C ABI
struct AxoBlockStateDef {
    const char* propertyName;       // e.g. "facing", "powered"
    int         propertyType;       // BOOL, INT_RANGE, ENUM
    int         minValue;
    int         maxValue;
    const char* enumValues[16];     // for ENUM type
};

// Add to AxoBlockDef:
int                statePropertyCount;
AxoBlockStateDef   stateProperties[4];   // max 4 properties (fits in 4-bit metadata)
int              (*getMetaFromState)(int propertyValues[4]);
void             (*getStateFromMeta)(int meta, int propertyValues[4]);
```

#### Phase 3 — Advanced Block Features

```c
// Redstone
bool  canProvidePower;
int (*getWeakPower)(int x, int y, int z, Level*, int side);
int (*getStrongPower)(int x, int y, int z, Level*, int side);

// Fire
bool  isFlammable;
int   flammability;        // 0-300
int   fireSpreadSpeed;     // 0-300

// Harvesting
const char* harvestTool;   // "pickaxe", "axe", "shovel"
int         harvestLevel;  // 0=hand, 1=wood, 2=stone, 3=iron, 4=diamond
bool        canSilkHarvest;
int       (*getExpDrop)(int x, int y, int z, Level*, int fortune);

// Special blocks
bool  isLadder;
bool  isBed;
bool  isBeaconBase;
float enchantPowerBonus;

// Explosion
void (*onExploded)(int x, int y, int z, Level*);
```

### Engine Modifications

- **`Tile` class (LCE)**: Override `use()`, `attack()`, `tick()`, `neighborChanged()`, `entityInside()`, `stepOn()`, `fallOn()` to dispatch to Axo callbacks
- **`Level` class**: Hook `setTile()` for placement callbacks, hook `removeTile()` for break callbacks
- **Block state storage**: LCE uses 4-bit data values per block (same as Java 1.8) — map `AxoBlockStateDef` properties to these bits
- **Random tick system**: Add custom blocks to the random-tickable set when `ticksRandomly = true`
- **Light engine**: Set custom `lightEmission` and `lightOpacity` during block registration

---

## 6. Item System

### Forge `Item` (80+ overridable methods)

#### Tool System

| Feature | Forge Methods |
|---------|--------------|
| Harvest Level | `setHarvestLevel(tool, level)`, `getHarvestLevel(stack, tool)` |
| Dig Speed | `getDigSpeed(stack, state)`, `canHarvestBlock(state)` |
| Tool Classes | `getToolClasses(stack)` — multiple tool types per item |
| Damage | `hitEntity(stack, target, attacker)` — returns true if damaged |
| Durability | `getMaxDamage()`, `getDamage()`, `setDamage()`, `isDamageable()`, `isDamaged()` |
| Durability Bar | `showDurabilityBar()`, `getDurabilityForDisplay()` |

#### Armor System

| Feature | Forge Methods |
|---------|--------------|
| Armor Check | `isValidArmor(stack, armorType, entity)` |
| Armor Model | `getArmorModel(entity, stack, slot, default)` |
| Armor Texture | `getArmorTexture(stack, entity, slot, type)` |
| Armor Tick | `onArmorTick(world, player, stack)` — per-tick armor logic |

#### Item Use Lifecycle

| Feature | Forge Methods |
|---------|--------------|
| Right-click Air | `onItemRightClick(stack, world, player)` |
| Right-click Block | `onItemUse(stack, player, world, pos, side, hitX, hitY, hitZ)` |
| Use Duration | `getMaxItemUseDuration(stack)` |
| Use Action | `getItemUseAction(stack)` — EAT, DRINK, BLOCK, BOW |
| Use Tick | `onUsingTick(stack, player, count)` |
| Use Finish | `onPlayerStoppedUsing(stack, world, player, timeLeft)` |
| Use Complete | `onItemUseFinish(stack, world, player)` |

#### Enchantment System

| Feature | Forge Methods |
|---------|--------------|
| Enchantability | `getItemEnchantability()`, `getItemEnchantability(stack)` |
| Bookshelf Enchant | `isBookEnchantable(stack, book)` |
| Enchant Effect | `hasEffect(stack)` — glint overlay |

#### Other

| Feature | Forge Methods |
|---------|--------------|
| Creative Tabs | `getCreativeTabs()` — multiple tabs per item |
| Entity Lifetime | `getEntityLifespan(stack, world)` |
| Container Item | `getContainerItem()`, `hasContainerItem()` — e.g. bucket → empty bucket |
| NBT Share Tag | `getNBTShareTag(stack)` — data synced to client |
| Update in Inventory | `onUpdate(stack, world, entity, slot, isSelected)` |
| Capabilities | `initCapabilities(stack, nbt)` |

### Axo `AxoItemDef` (Current)

```c
struct AxoItemDef {
    id, iconName, name, maxStackSize, creativeTab,
    onUse (NOT WIRED), onUseOn (NOT WIRED),
    attackDamage, miningSpeed,
    isPickaxe, isAxe, isShovel, isHandheld,
    isEdible, food (AxoFoodDef)
};
```

### Required Changes

#### Phase 1 — Wire Existing + Essential Callbacks

```c
// FIX: Wire onUse and onUseOn in AxoItemImpl.cpp
// These fields exist but are never called from the engine

// Add to AxoItemDef:
void (*onItemUse)(Level*, Player*, int x, int y, int z, int side);  // right-click block
void (*onItemRightClick)(Level*, Player*);                           // right-click air
void (*onHitEntity)(Level*, Player*, void* target);                  // left-click entity
int    maxDurability;          // 0 = unbreakable
bool   showDurabilityBar;
```

#### Phase 2 — Armor System

```c
struct AxoArmorDef {
    int   armorSlot;           // 0=helmet, 1=chest, 2=legs, 3=boots
    int   damageReduction;     // armor points
    int   durability;
    const wchar_t* armorTexture;
    void (*onArmorTick)(Level*, Player*);
};

// Add to AxoItemDef:
bool         isArmor;
AxoArmorDef  armor;
```

#### Phase 3 — Advanced Item Features

```c
// Enchanting
int   enchantability;      // 0 = not enchantable
bool  hasGlintEffect;      // enchantment glint

// Use duration (for bows, food, shields)
int   maxUseDuration;      // ticks
int   useAction;           // EAT=0, DRINK=1, BLOCK=2, BOW=3
void (*onUsingTick)(Level*, Player*, int ticksRemaining);
void (*onStoppedUsing)(Level*, Player*, int ticksUsed);
void (*onUseFinished)(Level*, Player*);

// Container item
const char* containerItem; // returned after crafting (e.g. "bucket")

// Inventory tick
void (*onInventoryTick)(Level*, Player*, int slot, bool isSelected);
```

### Engine Modifications

- **`Item` class (LCE)**: Override `use()`, `useOn()`, `hurtEnemy()` to call Axo callbacks
- **AxoItemImpl.cpp**: Wire `onUse` and `onUseOn` callbacks to `Item::use()` and `Item::useOn()` overrides
- **Armor registration**: Create `AxoArmorItem` subclass of `ArmorItem`
- **Durability system**: Override `getMaxDamage()`, `getDamageValue()` etc.
- **Enchantment hooks**: Override `getEnchantmentValue()` for custom enchantability

---

## 7. Entity System

### Forge (`EntityRegistry`)

```java
EntityRegistry.registerModEntity(
    EntityClass.class,
    "entity_name",
    modEntityId,
    modInstance,
    trackingRange,
    updateFrequency,
    sendsVelocityUpdates
);
```

| Feature | Description |
|---------|-------------|
| Custom Entity Classes | Full entity with AI, rendering, data watchers |
| Entity AI | `EntityAITasks` with priority-based task system |
| Spawn Eggs | `EntityRegistry.registerEgg(name, primaryColor, secondaryColor)` |
| Natural Spawning | `EntityRegistry.addSpawn(Class, weight, min, max, spawnType, biomes...)` |
| Entity Data | `DataWatcher` for client-server sync |
| NBT Persistence | `writeEntityToNBT` / `readEntityFromNBT` |
| Entity Rendering | Custom renderer registration |
| `IEntityAdditionalSpawnData` | Extra spawn packet data |
| `IThrowableEntity` | Projectile interface |

### Axo (Current)

- `SpawnEntity(Level*, entityId, x, y, z)` — **vanilla entities only**, by numeric ID
- No custom entity registration
- No entity AI
- No entity rendering
- No entity data persistence

### Required Changes

#### Phase 1 — Basic Custom Entities

```c
struct AxoEntityDef {
    int              id;               // AXO_ID_AUTO
    const char*      name;
    float            width;            // hitbox
    float            height;
    float            maxHealth;
    float            movementSpeed;
    float            attackDamage;
    bool             isHostile;
    bool             isAnimal;
    const wchar_t*   textureName;

    // Behavior callbacks
    void (*onTick)(void* entity, Level*);
    void (*onHurt)(void* entity, Level*, float damage);
    void (*onDeath)(void* entity, Level*);
    void (*onInteract)(void* entity, Level*, Player*);

    // Spawning
    int    spawnWeight;
    int    spawnMinGroup;
    int    spawnMaxGroup;
    int    spawnBiomes[16];
};
```

#### Phase 2 — Entity AI System

```c
enum AxoAITaskType {
    AXO_AI_WANDER,
    AXO_AI_FOLLOW_PLAYER,
    AXO_AI_FLEE_FROM,
    AXO_AI_ATTACK_MELEE,
    AXO_AI_ATTACK_RANGED,
    AXO_AI_LOOK_AT_PLAYER,
    AXO_AI_SWIM,
    AXO_AI_CUSTOM,
};

struct AxoAITask {
    int  type;
    int  priority;
    float speed;
    float range;
    void (*customTick)(void* entity, Level*);   // for AXO_AI_CUSTOM
};

// Add to AxoEntityDef:
int         aiTaskCount;
AxoAITask   aiTasks[16];
```

#### Phase 3 — Entity Data & Rendering

- Entity data sync (simplified DataWatcher)
- Custom model/renderer support
- NBT persistence callbacks
- Spawn egg registration

### Engine Modifications

- **Entity factory**: Register a factory function in `EntityFactory` that creates `AxoEntity` instances
- **`Mob` / `Animal` / `Monster` classes**: Create `AxoMob` / `AxoAnimal` / `AxoMonster` subclasses with callback dispatch
- **Entity rendering**: Register custom `EntityRenderer` keyed by entity type ID
- **Spawn tables**: Add custom entities to biome spawn lists
- **Entity ID allocation**: Extend entity type registry (LCE may have limited IDs)

---

## 8. Tile Entity System

### Forge (`TileEntity`)

| Feature | Description |
|---------|-------------|
| Registration | `GameRegistry.registerTileEntity(Class, name)` |
| Per-tick Logic | `ITickable.update()` — called every game tick |
| NBT Persistence | `writeToNBT(NBTTagCompound)` / `readFromNBT(NBTTagCompound)` |
| Client Sync | `getDescriptionPacket()` → sends data to client |
| Inventory | Implement `IInventory` for item storage |
| Capabilities | `getCapability(Capability, EnumFacing)` |
| Rendering | `TileEntitySpecialRenderer` for custom rendering |
| Chunk Load/Unload | `onChunkUnload()`, `validate()`, `invalidate()` |

### Axo (Current)

**No tile entity support.** Blocks are purely static.

### Required Changes

```c
struct AxoTileEntityDef {
    const char*  name;
    const char*  blockName;          // associated block

    // Lifecycle
    void (*onCreate)(int x, int y, int z, Level*);
    void (*onDestroy)(int x, int y, int z, Level*);
    void (*onTick)(int x, int y, int z, Level*);
    void (*onLoad)(int x, int y, int z, Level*);

    // Data persistence (simplified NBT)
    int  dataSize;                   // bytes of custom data
    void (*onSave)(int x, int y, int z, Level*, void* dataBuffer, int bufferSize);
    void (*onLoad)(int x, int y, int z, Level*, const void* dataBuffer, int bufferSize);

    // Inventory (optional)
    int  inventorySlots;             // 0 = no inventory
};

// API table addition:
bool (*RegisterTileEntity)(const AxoTileEntityDef* def);
```

### Engine Modifications

- **`TileEntity` class (LCE)**: Create `AxoTileEntity` subclass with callback dispatch
- **`Level` class**: Hook tile entity creation when Axo blocks are placed
- **Chunk save/load**: Serialize/deserialize custom tile entity data alongside chunk data
- **Tick loop**: Add `AxoTileEntity` instances to the tile entity tick list
- **Inventory system**: If `inventorySlots > 0`, create a `Container` for the tile entity

**This is one of the most complex additions** — it requires deep integration with LCE's chunk serialization and container/inventory system.

---

## 9. World Generation

### Forge (`IWorldGenerator`)

```java
GameRegistry.registerWorldGenerator(IWorldGenerator gen, int modGenerationWeight);

public interface IWorldGenerator {
    void generate(Random random, int chunkX, int chunkZ,
                  World world, IChunkProvider chunkProvider,
                  IChunkProvider chunkGenerator);
}
```

Also: Terrain gen events for fine control:

| Event | Description |
|-------|-------------|
| `OreGenEvent.Pre` / `Post` | Before/after ore generation |
| `OreGenEvent.GenerateMinable` | Per-ore-type generation |
| `DecorateBiomeEvent.Pre` / `Post` | Before/after decoration |
| `DecorateBiomeEvent.Decorate` | Per-decorator-type |
| `PopulateChunkEvent.Pre` / `Post` | Before/after population |
| `PopulateChunkEvent.Populate` | Per-feature-type |
| `SaplingGrowTreeEvent` | Sapling → tree growth |
| `InitMapGenEvent` | Map generator initialization |
| `BiomeEvent.GetGrassColor` / `GetFoliageColor` / `GetWaterColor` | Biome color overrides |
| `BiomeEvent.CreateDecorator` | Custom biome decorator |
| `BiomeEvent.GetVillageBlockID` | Village block selection |

### Axo (Current)

- `AxoBlockSpawnDef` embedded in `AxoBlockDef.spawn` — ore veins and surface patches only
- `AxoWorldGen.cpp` handles ore gen (`WorldGenerator::addOre`) and surface gen (`WorldGenerator::addSurface`)
- No arbitrary world gen callbacks
- No structure generation
- No terrain gen event hooks

### Required Changes

1. **Add `RegisterWorldGenerator` to API table**:

   ```c
   typedef void (*AxoWorldGenFunc)(Level* level, int chunkX, int chunkZ, int seed);

   struct AxoWorldGenDef {
       const char*     name;
       int             weight;        // generation order priority
       AxoWorldGenFunc generate;
   };

   bool (*RegisterWorldGenerator)(const AxoWorldGenDef* def);
   ```

2. **Provide utility functions** for world gen:

   ```c
   // Block placement helpers for world gen
   bool (*SetBlockAt)(Level*, int x, int y, int z, int tileId, int data);
   int  (*GetBlockAt)(Level*, int x, int y, int z);
   int  (*GetBiomeAt)(Level*, int x, int z);
   int  (*GetHeightAt)(Level*, int x, int z);
   ```

3. **Structure generation**: Beyond simple ore/surface — allow mods to place multi-block structures during chunk population

### Engine Modifications

- **Chunk population**: Hook into the chunk generation pipeline to call registered `AxoWorldGenFunc` callbacks after vanilla generation
- **AxoAPI.h**: Add `RegisterWorldGenerator`, `SetBlockAt`, `GetBlockAt`, `GetBiomeAt`, `GetHeightAt` to API table
- **AxoWorldGen.cpp**: Extend to call arbitrary world gen functions with proper priority ordering

---

## 10. Recipe System

### Forge (`GameRegistry` recipes)

| Feature | Forge | Axo |
|---------|-------|-----|
| Shaped Recipes | `addShapedRecipe(result, pattern, ingredients)` | ✅ `AxoRecipeDef` with `isShaped=true` |
| Shapeless Recipes | `addShapelessRecipe(result, ingredients)` | ✅ `AxoRecipeDef` with `isShaped=false` |
| Furnace Recipes | `addSmelting(input, output, xp)` | ✅ `AxoRecipeDef` with `isFurnace=true` |
| Ore Dictionary Recipes | `ShapedOreRecipe`, `ShapelessOreRecipe` | ❌ Missing (no ore dict) |
| Recipe Sorting | `RecipeSorter.register(name, class, category, dependencies)` | ❌ Missing |
| Fuel Handlers | `registerFuelHandler(IFuelHandler)` | ❌ Missing |
| Anvil Recipes | `AnvilUpdateEvent` | ❌ Missing |
| Brewing Recipes | `BrewingRecipeRegistry` | ❌ Missing |

### Required Changes

1. **Ore Dictionary Recipes**: After implementing Ore Dictionary (§13), allow recipe ingredients to specify ore dict keys instead of specific items

2. **Custom Fuel Handlers**:
   ```c
   typedef int (*AxoFuelHandler)(const char* itemName);  // returns burn time
   bool (*RegisterFuelHandler)(AxoFuelHandler handler);
   ```

3. **Brewing Recipes** (low priority):
   ```c
   struct AxoBrewingRecipe {
       const char* inputPotion;
       const char* ingredient;
       const char* outputPotion;
   };
   ```

### Engine Modifications

- **Furnace tile entity**: Hook fuel burn time calculation for custom fuel handlers
- **Crafting system**: Extend recipe matching to support ore dictionary wildcards
- **Brewing stand**: Hook recipe lookup for custom brewing recipes

---

## 11. Capability System

### Forge (`ICapabilityProvider`)

```java
public interface ICapabilityProvider {
    boolean hasCapability(Capability<?> capability, EnumFacing facing);
    <T> T getCapability(Capability<?> capability, EnumFacing facing);
}
```

| Feature | Description |
|---------|-------------|
| `Capability<T>` | Core holder with `IStorage<T>` for serialization |
| `CapabilityManager` | Registration singleton |
| `@CapabilityInject` | Automatic field injection |
| `CapabilityDispatcher` | High-speed dispatch for multiple capabilities |
| `ICapabilitySerializable<T>` | Combined provider + serialization |
| `AttachCapabilitiesEvent` | Attach caps to Entity/Item/TileEntity |

Used for: item inventories, fluid tanks, energy storage, custom data on entities, etc.

### Axo (Current)

**No capability system.**

### Assessment

The Forge capability system is **extremely powerful** but also very complex. For LCE modding, a **simplified version** is recommended:

```c
// Simplified "extra data" system instead of full capabilities
struct AxoExtraData {
    const char* key;           // "mymod:energy", "mymod:inventory"
    int         dataSize;      // bytes
    void*       data;
};

// Attach extra data to blocks/items/entities
bool (*AttachBlockData)(int x, int y, int z, const char* key, void* data, int size);
void*(*GetBlockData)(int x, int y, int z, const char* key);
bool (*AttachEntityData)(void* entity, const char* key, void* data, int size);
void*(*GetEntityData)(void* entity, const char* key);
```

### Engine Modifications

- **Requires tile entity system first** (§8) for block data persistence
- **Entity data**: Add a `std::unordered_map<std::string, std::vector<uint8_t>>` to entity base class
- **Serialization**: Include in chunk save/load for blocks, entity save/load for entities

---

## 12. Fluid System

### Forge (`net.minecraftforge.fluids`)

| Class/Interface | Description |
|----------------|-------------|
| `Fluid` | Fluid definition (name, density, viscosity, temperature, color) |
| `FluidStack` | Fluid amount (like ItemStack for fluids) |
| `FluidRegistry` | Registration and lookup |
| `IFluidHandler` | TileEntity that stores/transfers fluid |
| `IFluidTank` | Single tank with capacity |
| `IFluidBlock` | World-placeable fluid block |
| `BlockFluidClassic` | Vanilla-style fluid flow |
| `BlockFluidFinite` | Finite fluid simulation |
| `FluidContainerRegistry` | Items that hold fluids (buckets) |
| `IFluidContainerItem` | Item with internal fluid storage |
| `FluidUtil` | Transfer utilities |
| `UniversalBucket` | Universal fluid bucket |
| `TileFluidHandler` | Reference tile entity implementation |

### Axo (Current)

**No fluid system.** Only vanilla water/lava exist.

### Assessment

Full fluid system is **very complex** and likely **low-priority** for LCE modding. Recommended approach:

#### Phase 1 — Simple Custom Fluids (block-based only)

```c
struct AxoFluidDef {
    const char*    name;
    int            color;          // ARGB
    int            density;        // water=1000, lava=3000
    int            viscosity;      // water=1000, lava=6000
    int            luminosity;     // 0-15
    int            temperature;    // Kelvin (water=300, lava=1300)
    const wchar_t* stillTexture;
    const wchar_t* flowTexture;
    bool           isGaseous;
};

bool (*RegisterFluid)(const AxoFluidDef* def);
```

#### Phase 2 — Fluid Handling (requires tile entities)

- Tank blocks, pipes, fluid transfer between tile entities

### Engine Modifications

- **Block registration**: Create fluid blocks with custom flow behavior (extending LCE's `LiquidBlock`)
- **Texture registration**: Register still/flow textures for custom fluids
- **Bucket interaction**: Override bucket right-click to handle custom fluids
- **Rendering**: Custom liquid rendering with color tinting

---

## 13. Ore Dictionary

### Forge (`OreDictionary`)

```java
OreDictionary.registerOre("ingotIron", new ItemStack(Items.iron_ingot));
OreDictionary.registerOre("oreGold", new ItemStack(Blocks.gold_ore));
List<ItemStack> copperIngots = OreDictionary.getOres("ingotCopper");
```

| Feature | Description |
|---------|-------------|
| `registerOre(name, ItemStack)` | Register item under ore dict key |
| `getOres(name)` | Get all items matching key |
| `getOreIDs(ItemStack)` | Get all keys for an item |
| `getOreID(name)` | Get numeric ID for string key |
| `ShapedOreRecipe` | Recipe using ore dict ingredients |
| `ShapelessOreRecipe` | Shapeless recipe with ore dict |

### Axo (Current)

**No ore dictionary.** Recipes reference specific item names only.

### Required Changes

```c
// Register an item name under an ore dictionary key
bool (*RegisterOreDict)(const char* oreDictKey, const char* itemName);

// Look up all items matching an ore dictionary key
// Returns count, fills itemNames array
int (*GetOreDictEntries)(const char* oreDictKey, const char** itemNames, int maxEntries);

// Allow recipe ingredients to use ore dict keys
// In AxoRecipeDef, prefix ingredient with "#" for ore dict:
// e.g. "#ingotIron" instead of "iron_ingot"
```

### Engine Modifications

- **AxoAPI.cpp**: Implement `std::unordered_map<std::string, std::vector<int>>` for ore dict key → item IDs
- **AxoRecipeImpl.cpp**: Extend recipe matching to resolve `#oreKey` ingredients against the ore dictionary
- **AxoAPI.h**: Add `RegisterOreDict`, `GetOreDictEntries` to API table

---

## 14. Networking

### Forge (`SimpleNetworkWrapper`)

```java
SimpleNetworkWrapper network = NetworkRegistry.INSTANCE.newSimpleChannel("modid");
network.registerMessage(Handler.class, Message.class, id, Side.CLIENT);
network.registerMessage(Handler.class, Message.class, id, Side.SERVER);
network.sendToServer(message);
network.sendTo(message, player);
network.sendToAll(message);
```

| Feature | Description |
|---------|-------------|
| Custom Packets | Mod-defined message classes |
| Client → Server | Player actions, GUI interactions |
| Server → Client | Block data sync, entity data, effects |
| Targeted Sending | To specific player, all players, all in dimension, all near point |
| `IEntityAdditionalSpawnData` | Extra data when entity spawns on client |

### Axo (Current)

**No networking support.** Mods run entirely server-side (or single-player). No way to sync custom data to clients.

### Assessment

Networking is **critical for multiplayer** but **complex in LCE** due to its proprietary networking stack. Recommended approach:

#### Phase 1 — Data Sync (simplified)

```c
// Sync arbitrary data to all players (e.g., tile entity state)
bool (*SyncBlockData)(Level*, int x, int y, int z, const void* data, int size);

// Sync data to specific player
bool (*SyncPlayerData)(Player*, const char* channel, const void* data, int size);
```

#### Phase 2 — Custom Packet System

```c
struct AxoPacketDef {
    const char* channel;       // "mymod:my_packet"
    int         direction;     // CLIENT_TO_SERVER=0, SERVER_TO_CLIENT=1
    void (*onReceive)(Player*, const void* data, int size);
};

bool (*RegisterPacket)(const AxoPacketDef* def);
bool (*SendPacket)(const char* channel, Player* target, const void* data, int size);
```

### Engine Modifications

- **LCE Network Layer**: Hook into the packet dispatch system to intercept/inject custom packets
- **Custom packet registration**: Allocate packet IDs for mod channels
- **Data serialization**: Provide helpers for encoding/decoding common types (int, float, string, position)
- **This is one of the hardest features** — LCE's networking is very different from Java Edition's

---

## 15. Creative Tabs

### Forge

```java
public class ModTab extends CreativeTabs {
    public ModTab(String label) { super(label); }
    public Item getTabIconItem() { return ModItems.myItem; }
}
```

| Feature | Forge | Axo |
|---------|-------|-----|
| Custom Tabs | Full custom tab classes | ❌ Only preset enum values |
| Tab Icons | Custom item icons | ❌ Not supported |
| Multiple Tabs per Item | `getCreativeTabs()` returns array | ❌ Single tab |
| Tab Sorting | Custom sort order | ❌ Not supported |
| Search Tab | Special search tab | ❌ Not applicable |

### Axo (Current)

```c
enum AxoCreativeTab {
    AxoTab_BuildingBlocks = 0,
    AxoTab_Decoration     = 1,
    AxoTab_Redstone       = 2,
    AxoTab_Transport      = 3,
    AxoTab_Materials      = 4,
    AxoTab_Food           = 5,
    AxoTab_ToolsArmor     = 6,
    AxoTab_Brewing        = 7,
    AxoTab_Misc           = 12,
};
```

Items are assigned to a single existing tab via integer.

### Required Changes

```c
struct AxoCreativeTabDef {
    const char*    name;
    const char*    label;        // display name
    const char*    iconItem;     // item name for tab icon
    int            sortOrder;    // position in tab bar
};

bool (*RegisterCreativeTab)(const AxoCreativeTabDef* def);
```

### Engine Modifications

- **Creative inventory UI**: Add new tab buttons for mod tabs
- **Item sorting**: Extend creative item list to include mod tab membership
- **Tab rendering**: Render custom tab icons
- **Note**: LCE's creative inventory UI is **different from Java Edition** — this may require significant UI work

---

## 16. Sound & Particles

### Forge

| Feature | Forge |
|---------|-------|
| Custom Sounds | `SoundEvent` registration + `sounds.json` |
| Sound Events | `PlaySoundAtEntityEvent` (cancelable/modifiable) |
| Custom Particles | `EnumParticleTypes`, `EntityFX` registration |
| Particle Factory | `IParticleFactory` for custom particles |

### Axo (Current)

- `AxoBlockDef` has no sound configuration
- Blocks get default sounds from `AxoMaterial`
- No custom sound registration
- No custom particle support

### Required Changes

```c
// Sound System
enum AxoSoundEvent {
    AXO_SOUND_BLOCK_BREAK,
    AXO_SOUND_BLOCK_PLACE,
    AXO_SOUND_BLOCK_STEP,
    AXO_SOUND_ITEM_USE,
    AXO_SOUND_ENTITY_HURT,
    AXO_SOUND_ENTITY_DEATH,
    AXO_SOUND_CUSTOM,
};

struct AxoSoundDef {
    const char* name;           // "mymod:my_sound"
    const char* filePath;       // relative path to .ogg file in mod archive
    float       defaultVolume;
    float       defaultPitch;
};

bool (*RegisterSound)(const AxoSoundDef* def);
bool (*PlaySound)(Level*, const char* soundName, double x, double y, double z, float volume, float pitch);

// Add to AxoBlockDef:
const char* soundBreak;
const char* soundPlace;
const char* soundStep;

// Particle System (Phase 2)
bool (*SpawnParticle)(Level*, int particleType, double x, double y, double z,
                      double velX, double velY, double velZ);
```

### Engine Modifications

- **Sound engine**: Load custom .ogg files from mod archives, register as sound events
- **Sound playback**: Extend `Level::playSound` to handle custom sound names
- **Particle system**: Extend particle spawning to accept custom particle IDs (very complex for LCE)

---

## 17. GUI / Container System

### Forge

```java
NetworkRegistry.INSTANCE.registerGuiHandler(modInstance, guiHandler);

public interface IGuiHandler {
    Object getServerGuiElement(int id, EntityPlayer player, World world, int x, int y, int z);
    Object getClientGuiElement(int id, EntityPlayer player, World world, int x, int y, int z);
}
```

| Feature | Description |
|---------|-------------|
| Custom GUIs | `GuiContainer` / `GuiScreen` subclasses |
| Container Logic | `Container` with `Slot` definitions |
| Slot Types | Input, output, armor, hotbar, custom |
| Inventory Sync | Automatic server→client sync of container contents |
| GUI Opening | `player.openGui(mod, guiId, world, x, y, z)` |

### Axo (Current)

**No GUI/Container support.** No way to create custom inventory screens.

### Assessment

Custom GUIs are **very important** for modded gameplay (machines, storage, crafting stations) but **extremely complex** to implement across the C ABI boundary. This requires:

1. A container definition system (slot layout, item transfer logic)
2. GUI rendering (texture, slot positions, buttons, text)
3. Networking for inventory sync
4. Input handling

### Recommended Approach

```c
struct AxoContainerSlot {
    int  x, y;                 // pixel position
    int  slotType;             // INPUT=0, OUTPUT=1, FUEL=2
    bool allowPlayerInsert;
    bool allowPlayerExtract;
};

struct AxoContainerDef {
    const char*       name;
    const char*       title;
    const wchar_t*    backgroundTexture;   // GUI background
    int               textureWidth;
    int               textureHeight;
    int               slotCount;
    AxoContainerSlot  slots[54];           // max slots
    void            (*onSlotChanged)(int slotIndex, Level*, Player*);
    void            (*onContainerTick)(Level*, Player*);
};

bool (*RegisterContainer)(const AxoContainerDef* def);
bool (*OpenContainer)(Player*, const char* containerName, int x, int y, int z);
```

### Engine Modifications

- **Container system**: Create `AxoContainer` subclass with dynamic slot layout
- **GUI rendering**: Create `AxoGuiContainer` that renders from texture + slot definitions
- **Tile entity integration**: Link containers to tile entities for persistent inventory
- **Networking**: Sync container state between server and client
- **Input handling**: Forward clicks, drags, shift-clicks to container logic
- **Note**: LCE uses a **completely different GUI system** from Java Edition — this requires extensive reverse engineering

---

## 18. Terrain Generation Events

### Forge (`net.minecraftforge.event.terraingen`)

| Event | Description |
|-------|-------------|
| `OreGenEvent.Pre` / `Post` | Before/after ore generation |
| `OreGenEvent.GenerateMinable` | Per-ore generation (cancelable) |
| `DecorateBiomeEvent.Pre` / `Post` | Before/after biome decoration |
| `DecorateBiomeEvent.Decorate` | Per-decorator type (cancelable) |
| `PopulateChunkEvent.Pre` / `Post` | Before/after chunk population |
| `PopulateChunkEvent.Populate` | Per-feature population (cancelable) |
| `SaplingGrowTreeEvent` | Sapling growth (cancelable) |
| `InitMapGenEvent` | Map generator init |
| `InitNoiseGensEvent` | Noise generator init |
| `BiomeEvent.CreateDecorator` | Biome decorator creation |
| `BiomeEvent.GetGrassColor` / `GetFoliageColor` / `GetWaterColor` | Color overrides |
| `BiomeEvent.GetVillageBlockID` | Village block selection |
| `ChunkProviderEvent.ReplaceBiomeBlocks` | Biome surface replacement |

### Axo (Current)

- Basic ore and surface generation only
- No terrain gen events
- No way to modify vanilla generation

### Required Changes

These would be implemented as part of the Event System (§4):

```c
// Terrain gen event types added to AxoEventType enum:
AXO_EVENT_ORE_GEN_PRE,
AXO_EVENT_ORE_GEN_POST,
AXO_EVENT_DECORATE_BIOME_PRE,
AXO_EVENT_DECORATE_BIOME_POST,
AXO_EVENT_POPULATE_CHUNK_PRE,
AXO_EVENT_POPULATE_CHUNK_POST,
AXO_EVENT_SAPLING_GROW_TREE,

// Event data for terrain gen:
struct AxoTerrainGenEventData {
    Level* level;
    int    chunkX;
    int    chunkZ;
    int    seed;
    int    generationType;   // ore type, decorator type, etc.
};
```

### Engine Modifications

- **Chunk generation pipeline**: Insert event dispatch hooks before/after each generation phase
- **These hooks must NOT slow down chunk generation significantly** — performance-critical path

---

## 19. Engine Modifications Required

### Summary of All Engine Changes

#### Tier 1 — Minimal (AxoModLoader / AxoAPI only)

These changes are **contained within the Axo mod loader** and don't touch the LCE engine:

| Change | Files Affected |
|--------|---------------|
| String-based registry with namespacing | AxoAPI.h, AxoAPI.cpp |
| Ore Dictionary implementation | AxoAPI.h, AxoAPI.cpp, AxoRecipeImpl.cpp |
| Fuel handler registration | AxoAPI.h, AxoAPI.cpp |
| Event bus implementation (dispatch logic) | New: AxoEventBus.cpp/h |
| World gen block placement helpers | AxoAPI.h, AxoAPI.cpp |

#### Tier 2 — Light Engine Hooks

These require **small hooks** at specific points in the LCE engine:

| Change | LCE Engine Location | Hook Type |
|--------|---------------------|-----------|
| Wire `onUse`/`onUseOn` item callbacks | `Item::use()`, `Item::useOn()` | Virtual override |
| Block `onActivated` (right-click) | `Tile::use()` | Virtual override |
| Block `onClicked` (left-click) | `Tile::attack()` | Virtual override |
| Block `onPlaced` callback | `Level::setTile()` call sites | Post-call hook |
| Block `onNeighborChanged` | `Tile::neighborChanged()` | Virtual override |
| Block random/scheduled ticking | Tick dispatcher | Registration + virtual override |
| Block light emission/opacity | Block registration | Property setting |
| Block entity collision | `Tile::entityInside()`, `Tile::stepOn()` | Virtual override |
| Custom sounds | `Level::playSound()` | Lookup extension |
| Event dispatch: block break | `Tile::playerDestroy()` or `Level::removeTile()` | Pre-call hook |
| Event dispatch: block place | `Level::setTile()` player paths | Pre-call hook |
| Event dispatch: entity hurt | `Entity::hurt()` | Pre-call hook |

#### Tier 3 — Moderate Engine Changes

These require **new systems** or **significant modifications**:

| Change | Description | Complexity |
|--------|-------------|------------|
| Block states / metadata | Map Axo state properties to 4-bit block data | Medium |
| Custom entity registration | Entity factory, rendering, AI | High |
| Tile entity system | Chunk serialization, tick loop, container | High |
| Custom armor items | ArmorItem subclass, rendering | Medium |
| Custom creative tabs | Creative inventory UI modification | Medium |
| Redstone integration | Power provider/receiver callbacks | Medium |
| Fluid blocks | LiquidBlock subclass, flow behavior | Medium |
| Enchantment hooks | Enchantment value, glint rendering | Low |

#### Tier 4 — Deep Engine Changes

These require **extensive reverse engineering** and **core system modifications**:

| Change | Description | Complexity |
|--------|-------------|------------|
| Custom networking / packets | LCE proprietary networking stack | Very High |
| GUI / Container system | LCE-specific UI framework | Very High |
| Custom particles | Particle system extension | High |
| Entity data sync (DataWatcher) | Client-server entity data | High |
| Terrain gen event hooks | Chunk generation pipeline | High |
| Biome color overrides | Biome rendering system | Medium |

---

## 20. Implementation Priority & Roadmap

### Phase 1 — Foundation (Weeks 1-4)

> **Goal**: Wire existing callbacks, add essential block/item interactions

| Task | Effort | Impact |
|------|--------|--------|
| Wire `onUse`/`onUseOn` in AxoItemImpl.cpp | 1 day | 🔴 High — unlocks item interactions |
| Add `onActivated` (right-click) to blocks | 2 days | 🔴 High — unlocks interactive blocks |
| Add `onPlaced` callback to blocks | 1 day | 🟡 Medium |
| Add `onNeighborChanged` callback | 1 day | 🟡 Medium |
| Add block light emission/opacity | 1 day | 🟡 Medium — glowing blocks |
| Add random tick + scheduled tick to blocks | 3 days | 🔴 High — unlocks crop-like behavior |
| Add `onEntityWalkOn` / `onFallenUpon` | 1 day | 🟡 Medium |
| Add `onClicked` (left-click) to blocks | 1 day | 🟡 Medium |
| String-based registry with namespacing | 3 days | 🟠 Medium — future-proofing |

### Phase 2 — Event System (Weeks 5-8)

> **Goal**: Build the event bus and wire the most critical events

| Task | Effort | Impact |
|------|--------|--------|
| Design event bus (AxoEventBus.cpp/h) | 3 days | 🔴 Critical — foundation for all events |
| Wire `BLOCK_BREAK` event | 1 day | 🔴 High |
| Wire `BLOCK_PLACE` event | 1 day | 🔴 High |
| Wire `ENTITY_HURT` / `ENTITY_DEATH` events | 2 days | 🔴 High |
| Wire `PLAYER_INTERACT` / `PLAYER_ATTACK` events | 2 days | 🔴 High |
| Wire `WORLD_TICK` event | 1 day | 🟡 Medium |
| Wire `ENTITY_SPAWN` event | 1 day | 🟡 Medium |
| Wire `ITEM_PICKUP` event | 1 day | 🟡 Medium |

### Phase 3 — Block States & Tile Entities (Weeks 9-16)

> **Goal**: Enable stateful blocks with persistent data

| Task | Effort | Impact |
|------|--------|--------|
| Block states / metadata system | 1 week | 🔴 High — directional blocks, multi-state blocks |
| Tile entity foundation | 2 weeks | 🔴 Critical — enables machines, storage |
| Tile entity NBT persistence | 1 week | 🔴 High — data survives chunk reload |
| Tile entity ticking | 2 days | 🔴 High — enables furnaces, machines |
| Tile entity inventory (basic) | 1 week | 🟡 High — storage blocks |

### Phase 4 — Custom Entities & Armor (Weeks 17-24)

> **Goal**: Custom mobs and equipment

| Task | Effort | Impact |
|------|--------|--------|
| Entity registration system | 1 week | 🔴 High |
| Basic entity AI (wander, follow, attack) | 2 weeks | 🟡 High |
| Entity rendering (textured box model) | 1 week | 🟡 High |
| Custom armor items | 1 week | 🟡 Medium |
| Durability system for tools/armor | 3 days | 🟡 Medium |
| Enchantment hooks | 3 days | 🟠 Medium |

### Phase 5 — Advanced Systems (Weeks 25-36)

> **Goal**: Feature parity with most common Forge mods

| Task | Effort | Impact |
|------|--------|--------|
| Ore Dictionary | 3 days | 🟠 Medium |
| Custom fuel handlers | 1 day | 🟠 Low |
| Custom sounds | 1 week | 🟠 Medium |
| Custom world gen callbacks | 1 week | 🟠 Medium |
| Custom creative tabs | 3 days | 🟠 Low |
| Fire properties on blocks | 2 days | 🟠 Low |
| Harvest levels / tool types | 2 days | 🟠 Medium |
| Redstone integration | 1 week | 🟠 Medium |

### Phase 6 — Complex Systems (Weeks 37+)

> **Goal**: Deep modding capabilities

| Task | Effort | Impact |
|------|--------|--------|
| Fluid system (basic) | 2 weeks | 🟠 Medium |
| GUI / Container system | 3-4 weeks | 🔴 High (for tech mods) |
| Custom networking | 3-4 weeks | 🔴 High (for multiplayer) |
| Terrain gen events | 2 weeks | 🟠 Medium |
| Custom particles | 2 weeks | 🔵 Low |
| Entity data sync (DataWatcher equivalent) | 2 weeks | 🟡 High (for multiplayer entities) |
| Capability system (simplified) | 1 week | 🟡 Medium |

---

## Appendix A — API Table Growth Projection

### Current (11 functions)

```
Log, RegisterItem, RegisterBlock, RegisterRecipe, RegisterBiome, RegisterCrop,
SpawnEntity, DropItem, StrikeLightning, SpawnTnt, SpawnFallingBlock
```

### Projected (after all phases, ~35-40 functions)

```
// Existing
Log, RegisterItem, RegisterBlock, RegisterRecipe, RegisterBiome, RegisterCrop,
SpawnEntity, DropItem, StrikeLightning, SpawnTnt, SpawnFallingBlock,

// Registry (Phase 1)
FindBlockByName, FindItemByName,

// Event System (Phase 2)
RegisterEventHandler, UnregisterEventHandler,

// Tile Entities (Phase 3)
RegisterTileEntity,

// Custom Entities (Phase 4)
RegisterEntity,

// World Gen (Phase 5)
RegisterWorldGenerator, SetBlockAt, GetBlockAt, GetBiomeAt, GetHeightAt,

// Ore Dictionary (Phase 5)
RegisterOreDict, GetOreDictEntries,

// Sound (Phase 5)
RegisterSound, PlaySound,

// Creative Tabs (Phase 5)
RegisterCreativeTab,

// Fuel (Phase 5)
RegisterFuelHandler,

// Networking (Phase 6)
RegisterPacket, SendPacket, SyncBlockData,

// GUI (Phase 6)
RegisterContainer, OpenContainer,

// Fluid (Phase 6)
RegisterFluid,

// Capability (Phase 6)
AttachBlockData, GetBlockData, AttachEntityData, GetEntityData,

// Particles (Phase 6)
SpawnParticle,
```

### ABI Versioning Strategy

To maintain backward compatibility as the API table grows:

```c
struct AxoAPITable {
    int version;  // ABI version number

    // v1 functions (always present)
    void (*Log)(...);
    bool (*RegisterItem)(...);
    // ...

    // v2 functions (added in version 2)
    bool (*RegisterEventHandler)(...);
    // ...

    // v3 functions (added in version 3)
    bool (*RegisterTileEntity)(...);
    // ...
};

// Mods check version before calling new functions:
if (gAxoAPI->version >= 2) {
    gAxoAPI->RegisterEventHandler(...);
}
```

---

## Appendix B — Feature Parity Matrix

| Forge Feature | Axo Equivalent | Gap | Phase |
|---------------|---------------|-----|-------|
| `GameRegistry.registerBlock` | `RegisterBlock` | ✅ Basic parity | - |
| `GameRegistry.registerItem` | `RegisterItem` | ✅ Basic parity | - |
| `GameRegistry.addRecipe` | `RegisterRecipe` | ✅ Basic parity | - |
| `GameRegistry.addSmelting` | `RegisterRecipe` (furnace) | ✅ Basic parity | - |
| `GameRegistry.registerWorldGenerator` | `AxoBlockSpawnDef` only | ⚠️ Ore/surface only | 5 |
| `GameRegistry.registerTileEntity` | ❌ | 🔴 Missing | 3 |
| `GameRegistry.registerFuelHandler` | ❌ | 🟠 Missing | 5 |
| `GameRegistry.findBlock/findItem` | ❌ | 🟡 Missing | 1 |
| `EntityRegistry.registerModEntity` | ❌ | 🔴 Missing | 4 |
| `MinecraftForge.EVENT_BUS` | ❌ | 🔴 Missing | 2 |
| `Block.onBlockActivated` | ❌ | 🔴 Missing | 1 |
| `Block.updateTick` | ❌ | 🔴 Missing | 1 |
| `Block.randomTick` | ❌ | 🔴 Missing | 1 |
| `Block IBlockState` | ❌ | 🔴 Missing | 3 |
| `Block.hasTileEntity` | ❌ | 🔴 Missing | 3 |
| `Block redstone methods` | ❌ | 🟠 Missing | 5 |
| `Block fire methods` | ❌ | 🟠 Missing | 5 |
| `Item.onItemUse` | Fields exist, NOT WIRED | ⚠️ Broken | 1 |
| `Item.onItemRightClick` | Fields exist, NOT WIRED | ⚠️ Broken | 1 |
| `Item armor system` | ❌ | 🔴 Missing | 4 |
| `Item durability` | ❌ | 🟡 Missing | 4 |
| `Item enchantability` | ❌ | 🟠 Missing | 4 |
| `CreativeTabs (custom)` | Enum only | ⚠️ Partial | 5 |
| `Capability system` | ❌ | 🟡 Missing | 6 |
| `Fluid system` | ❌ | 🟠 Missing | 6 |
| `OreDictionary` | ❌ | 🟠 Missing | 5 |
| `SimpleNetworkWrapper` | ❌ | 🔴 Missing | 6 |
| `GuiHandler` | ❌ | 🔴 Missing | 6 |
| `Custom sounds` | ❌ | 🟠 Missing | 5 |
| `Custom particles` | ❌ | 🔵 Missing | 6 |
| `TileEntitySpecialRenderer` | ❌ | 🟡 Missing | 6 |
| `IWorldGenerator (arbitrary)` | ❌ | 🟡 Missing | 5 |

---

## Appendix C — Risks & Constraints

### LCE-Specific Limitations

1. **Block ID limit**: Only 82 custom block IDs available (174-255). Forge Java Edition has no practical limit. This is a **hard LCE engine constraint** that cannot easily be changed.

2. **4-bit metadata**: Block metadata is limited to 4 bits (0-15), same as Java 1.8. Forge works within this too, but mods needing more states rely on tile entities.

3. **Proprietary networking**: LCE uses Xbox/PlayStation networking libraries, not the Netty-based system Java Edition uses. Custom packets require reverse engineering the network protocol.

4. **GUI framework**: LCE's UI is built for controllers, not mouse. Custom GUIs must work with gamepad navigation.

5. **No reflection/classloading**: Forge relies heavily on Java reflection (`@SubscribeEvent`, `@ObjectHolder`, etc.). Axo must use explicit function pointer registration — conceptually similar but more verbose for mod authors.

6. **Memory constraints**: Console platforms have less RAM than PCs. Custom entities, tile entities, and fluid systems must be carefully memory-managed.

7. **Single-threaded rendering**: LCE's render pipeline is different from Java Edition. Custom entity/tile entity renderers must integrate with LCE's specific render system.

### C-ABI Constraints

1. **No inheritance in API**: Mods cannot subclass engine types across the DLL boundary. All customization must go through callbacks and data structs.

2. **No exceptions**: Errors must be returned via return codes or logged via `Log()`.

3. **String lifetime**: C-strings (`const char*`) passed to the API must remain valid during the registration call. The engine copies them internally.

4. **No templates/generics**: The capability system and event system cannot use C++ templates in the public API.

---

*Document generated from comparison of [Forge 1.8.9-11.15.1.2318 JavaDocs](https://skmedix.github.io/ForgeJavaDocs/javadoc/forge/1.8.9-11.15.1.2318/) against the Axo Mod Loader codebase (main branch, June 2025).*
