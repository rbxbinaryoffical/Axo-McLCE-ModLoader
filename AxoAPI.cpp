#include "stdafx.h"

#include "AxoAPI.h"
#include "AxoModLoader.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdio>

extern bool AxoItem_CreateFromDef(const AxoItemDef& def);
extern void AxoItem_AddToCreativeMenu(int itemId, int creativeTab);
extern bool AxoBlock_CreateFromDef(const AxoBlockDefInternal& def);
extern void AxoBlock_AddToCreativeMenu(int blockId, int creativeTab);
extern bool AxoRecipe_CreateFromDef(const AxoRecipeDef& def);

AxoAPITable* gAxoAPI = nullptr;

static std::vector<AxoItemDef>   sPendingItems;
static std::vector<AxoBlockDef>  sPendingBlocks;
static std::vector<AxoRecipeDef> sPendingRecipes;

static int sNextItemId  = 422;
static int sNextBlockId = 174;

struct RegisteredItem  { int id; int creativeTab; std::string name; };
struct RegisteredBlock { int id; int creativeTab; };
static std::vector<RegisteredItem>  sRegisteredItems;
static std::vector<RegisteredBlock> sRegisteredBlocks;

static const std::unordered_map<std::string, int>& GetVanillaItemMap() {
    static const std::unordered_map<std::string, int> kMap = {
        {"shovel_iron",          256}, {"pickaxe_iron",        257}, {"axe_iron",            258},
        {"flint_and_steel",      259}, {"apple",               260}, {"bow",                 261},
        {"arrow",                262}, {"coal",                263}, {"diamond",              264},
        {"iron_ingot",           265}, {"gold_ingot",          266}, {"sword_iron",           267},
        {"sword_wood",           268}, {"shovel_wood",         269}, {"pickaxe_wood",         270},
        {"axe_wood",             271}, {"sword_stone",         272}, {"shovel_stone",         273},
        {"pickaxe_stone",        274}, {"axe_stone",           275}, {"sword_diamond",        276},
        {"shovel_diamond",       277}, {"pickaxe_diamond",     278}, {"axe_diamond",          279},
        {"stick",                280}, {"bowl",                281}, {"mushroom_stew",        282},
        {"sword_gold",           283}, {"shovel_gold",         284}, {"pickaxe_gold",         285},
        {"axe_gold",             286}, {"string",              287}, {"feather",              288},
        {"gunpowder",            289}, {"hoe_wood",            290}, {"hoe_stone",            291},
        {"hoe_iron",             292}, {"hoe_diamond",         293}, {"hoe_gold",             294},
        {"seeds_wheat",          295}, {"wheat",               296}, {"bread",                297},
        {"helmet_leather",       298}, {"chestplate_leather",  299}, {"leggings_leather",     300},
        {"boots_leather",        301}, {"helmet_chain",        302}, {"chestplate_chain",     303},
        {"leggings_chain",       304}, {"boots_chain",         305}, {"helmet_iron",          306},
        {"chestplate_iron",      307}, {"leggings_iron",       308}, {"boots_iron",           309},
        {"helmet_diamond",       310}, {"chestplate_diamond",  311}, {"leggings_diamond",     312},
        {"boots_diamond",        313}, {"helmet_gold",         314}, {"chestplate_gold",      315},
        {"leggings_gold",        316}, {"boots_gold",          317}, {"flint",                318},
        {"porkchop_raw",         319}, {"porkchop_cooked",     320}, {"painting",             321},
        {"apple_gold",           322}, {"sign",                323}, {"door_wood",            324},
        {"bucket_empty",         325}, {"bucket_water",        326}, {"bucket_lava",          327},
        {"minecart",             328}, {"saddle",              329}, {"door_iron",            330},
        {"redstone",             331}, {"snowball",            332}, {"boat",                 333},
        {"leather",              334}, {"bucket_milk",         335}, {"brick",                336},
        {"clay",                 337}, {"reeds",               338}, {"paper",                339},
        {"book",                 340}, {"slimeball",           341}, {"minecart_chest",       342},
        {"minecart_furnace",     343}, {"egg",                 344}, {"compass",              345},
        {"fishing_rod",          346}, {"clock",               347}, {"glowstone_dust",       348},
        {"fish_raw",             349}, {"fish_cooked",         350}, {"dye",                  351},
        {"bone",                 352}, {"sugar",               353}, {"cake",                 354},
        {"bed",                  355}, {"repeater",            356}, {"cookie",               357},
        {"map",                  358}, {"shears",              359}, {"melon",                360},
        {"seeds_pumpkin",        361}, {"seeds_melon",         362}, {"beef_raw",             363},
        {"beef_cooked",          364}, {"chicken_raw",         365}, {"chicken_cooked",       366},
        {"rotten_flesh",         367}, {"ender_pearl",         368}, {"blaze_rod",            369},
        {"ghast_tear",           370}, {"gold_nugget",         371}, {"nether_wart",          372},
        {"potion",               373}, {"glass_bottle",        374}, {"spider_eye",           375},
        {"fermented_spider_eye", 376}, {"blaze_powder",        377}, {"magma_cream",          378},
        {"brewing_stand",        379}, {"cauldron",            380}, {"eye_of_ender",         381},
        {"glistering_melon",     382}, {"spawn_egg",           383}, {"exp_bottle",           384},
        {"fireball",             385}, {"emerald",             388}, {"item_frame",           389},
        {"flower_pot",           390}, {"carrots",             391}, {"potato",               392},
        {"potato_baked",         393}, {"potato_poisonous",    394}, {"empty_map",            395},
        {"carrot_golden",        396}, {"skull",               397}, {"carrot_on_stick",      398},
        {"nether_star",          399}, {"pumpkin_pie",         400}, {"fireworks",            401},
        {"fireworks_charge",     402}, {"enchanted_book",      403}, {"comparator",           404},
        {"nether_brick",         405}, {"nether_quartz",       406}, {"minecart_tnt",         407},
        {"minecart_hopper",      408}, {"horse_armor_iron",    417}, {"horse_armor_gold",     418},
        {"horse_armor_diamond",  419}, {"lead",                420}, {"name_tag",             421},
    };
    return kMap;
}

static int ResolveDropItemName(const std::string& name, int blockId) {
    if (name.empty()) return 0;
    const auto& map = GetVanillaItemMap();
    auto it = map.find(name);
    if (it != map.end()) return it->second;
    for (const auto& r : sRegisteredItems) {
        if (r.name == name) return r.id;
    }
    printf("[AxoLoader] RegisterBlock id=%d: unknown dropItemName \"%s\", dropping self.\n",
           blockId, name.c_str());
    return 0;
}

static void Impl_Log(const char* modId, const char* msg) {
    printf("[AxoLoader][%s] %s\n", modId, msg);
    fflush(stdout);
}

static bool Impl_RegisterItem(const AxoItemDef* def) {
    if (!def) return false;
    AxoItemDef resolved = *def;
    if (resolved.id == AXO_ID_AUTO) {
        if (sNextItemId > 31999) {
            printf("[AxoLoader] RegisterItem: no free item IDs left.\n");
            return false;
        }
        resolved.id = sNextItemId++;
    } else if (resolved.id < 422 || resolved.id > 31999) {
        printf("[AxoLoader] RegisterItem: id %d out of range (422-31999).\n", resolved.id);
        return false;
    }
    sPendingItems.push_back(resolved);
    printf("[AxoLoader] Queued item id=%d \"%s\"\n", resolved.id, resolved.name.c_str());
    return true;
}

static bool Impl_RegisterBlock(const AxoBlockDef* def) {
    if (!def) return false;
    AxoBlockDef resolved = *def;
    if (resolved.id == AXO_ID_AUTO) {
        if (sNextBlockId > 255) {
            printf("[AxoLoader] RegisterBlock: no free block IDs left.\n");
            return false;
        }
        resolved.id = sNextBlockId++;
    } else if (resolved.id < 174 || resolved.id > 255) {
        printf("[AxoLoader] RegisterBlock: id %d out of range (174-255).\n", resolved.id);
        return false;
    }
    sPendingBlocks.push_back(resolved);
    printf("[AxoLoader] Queued block id=%d \"%s\" drop=\"%s\"\n",
           resolved.id, resolved.name.c_str(), resolved.dropItemName.c_str());
    return true;
}

static bool Impl_RegisterRecipe(const AxoRecipeDef* def) {
    if (!def) return false;
    sPendingRecipes.push_back(*def);
    printf("[AxoLoader] Queued recipe -> \"%s\" (x%d)\n",
           def->resultItemName.c_str(), def->resultCount);
    return true;
}

static AxoAPITable sAPITable = {
    Impl_Log,
    Impl_RegisterItem,
    Impl_RegisterBlock,
    Impl_RegisterRecipe
};

AxoAPITable* AxoAPI_GetTable() {
    return &sAPITable;
}

void AxoAPI_FlushRegistrations() {
    printf("[AxoLoader] FlushRegistrations: %u item(s)...\n", (unsigned)sPendingItems.size());
    for (const auto& def : sPendingItems) {
        if (AxoItem_CreateFromDef(def))
            sRegisteredItems.push_back({def.id, def.creativeTab, def.name});
    }
    sPendingItems.clear();
}

void AxoAPI_FlushBlockRegistrations() {
    printf("[AxoLoader] FlushBlockRegistrations: %u block(s)...\n", (unsigned)sPendingBlocks.size());
    for (const auto& def : sPendingBlocks) {
        AxoBlockDefInternal internal;
        internal.id          = def.id;
        internal.iconName    = def.iconName;
        internal.name        = def.name;
        internal.material    = def.material;
        internal.hardness    = def.hardness;
        internal.resistance  = def.resistance;
        internal.creativeTab = def.creativeTab;
        internal.dropItemId  = ResolveDropItemName(def.dropItemName, def.id);
        internal.dropCount   = def.dropCount;
        if (AxoBlock_CreateFromDef(internal))
            sRegisteredBlocks.push_back({def.id, def.creativeTab});
    }
    sPendingBlocks.clear();
}

void AxoAPI_FlushCreativeMenu() {
    printf("[AxoLoader] FlushCreativeMenu: %u item(s), %u block(s)...\n",
           (unsigned)sRegisteredItems.size(), (unsigned)sRegisteredBlocks.size());
    for (const auto& r : sRegisteredItems)
        AxoItem_AddToCreativeMenu(r.id, r.creativeTab);
    for (const auto& r : sRegisteredBlocks)
        AxoBlock_AddToCreativeMenu(r.id, r.creativeTab);
}

void AxoAPI_FlushRecipeRegistrations() {
    printf("[AxoLoader] FlushRecipeRegistrations: %u recipe(s)...\n", (unsigned)sPendingRecipes.size());
    for (const auto& def : sPendingRecipes)
        AxoRecipe_CreateFromDef(def);
    sPendingRecipes.clear();
}

int AxoAPI_ResolveItemName(const std::string& name) {
    const auto& map = GetVanillaItemMap();
    auto it = map.find(name);
    if (it != map.end()) return it->second;
    for (const auto& r : sRegisteredItems) {
        if (r.name == name) return r.id;
    }
    return -1;
}
