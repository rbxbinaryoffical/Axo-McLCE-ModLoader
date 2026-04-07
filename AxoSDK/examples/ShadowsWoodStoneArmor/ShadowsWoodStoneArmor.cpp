// ═══════════════════════════════════════════════════════════════════════════════
//  Shadow's Wooden And Stone Armor — Axo Port
//
//  Original Forge mod by Shadow3654 (1.8.x, v1.3.2)
//  Ported to Axo Mod Loader for Minecraft LCE
//
//  Adds 7 armor materials (6 wood variants + stone) with 28 armor pieces
//  and full crafting recipes.
// ═══════════════════════════════════════════════════════════════════════════════
#ifndef AXO_MOD
#define AXO_MOD
#endif
#define MOD_ID "shadows_wood_stone_armor"
#include "AxoAPI.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Armor Material Definitions (from Forge EnumHelper.addArmorMaterial)
//
//  Forge signature: (name, textureName, durabilityMultiplier, reductions[], enchantability)
//  Wood:  durabilityMultiplier=4, protections={1,2,1,1}, enchantability=6
//  Stone: durabilityMultiplier=10, protections={2,5,4,1}, enchantability=5
//
//  Forge vanilla reference:
//    LEATHER: dur=5,  prot={1,3,2,1}, ench=15
//    CHAIN:   dur=15, prot={2,5,4,1}, ench=12
//    IRON:    dur=15, prot={2,6,5,2}, ench=9
//
//  So wood is weaker than leather, stone matches chainmail protection.
// ─────────────────────────────────────────────────────────────────────────────

static AxoArmorMaterialDef MakeWoodMaterial(const char* name) {
    AxoArmorMaterialDef mat = {};
    mat.name                 = name;
    mat.durabilityMultiplier = 4;
    mat.slotProtections[0]   = 1;  // head
    mat.slotProtections[1]   = 2;  // torso
    mat.slotProtections[2]   = 1;  // legs
    mat.slotProtections[3]   = 1;  // feet
    mat.enchantability       = 6;
    mat.repairItemName       = "stick";
    return mat;
}

static AxoArmorMaterialDef MakeStoneMaterial() {
    AxoArmorMaterialDef mat = {};
    mat.name                 = "stone";
    mat.durabilityMultiplier = 10;
    mat.slotProtections[0]   = 2;  // head
    mat.slotProtections[1]   = 5;  // torso
    mat.slotProtections[2]   = 4;  // legs
    mat.slotProtections[3]   = 1;  // feet
    mat.enchantability       = 5;
    mat.repairItemName       = "cobblestone";
    return mat;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: register one full armor set (helmet + chestplate + leggings + boots)
// ─────────────────────────────────────────────────────────────────────────────

struct ArmorSetNames {
    const char*    helmetName;
    const char*    chestName;
    const char*    legsName;
    const char*    bootsName;
    const wchar_t* helmetIcon;
    const wchar_t* chestIcon;
    const wchar_t* legsIcon;
    const wchar_t* bootsIcon;
};

static void RegisterArmorSet(
    const ArmorSetNames& names,
    const AxoArmorMaterialDef& material,
    int modelIndex,
    const char* armorTextureName = nullptr)
{
    // Helmet
    {
        AxoItemDef item = {};
        item.name         = names.helmetName;
        item.iconName     = names.helmetIcon;
        item.maxStackSize = 1;
        item.creativeTab  = AxoTab_ToolsArmor;
        item.isArmor      = true;
        item.armor.slot       = AXO_ARMOR_HEAD;
        item.armor.modelIndex = modelIndex;
        item.armor.material   = material;
        if (armorTextureName) item.armor.armorTextureName = armorTextureName;
        AxoAPI_RegisterItem(&item);
    }
    // Chestplate
    {
        AxoItemDef item = {};
        item.name         = names.chestName;
        item.iconName     = names.chestIcon;
        item.maxStackSize = 1;
        item.creativeTab  = AxoTab_ToolsArmor;
        item.isArmor      = true;
        item.armor.slot       = AXO_ARMOR_TORSO;
        item.armor.modelIndex = modelIndex;
        item.armor.material   = material;
        if (armorTextureName) item.armor.armorTextureName = armorTextureName;
        AxoAPI_RegisterItem(&item);
    }
    // Leggings
    {
        AxoItemDef item = {};
        item.name         = names.legsName;
        item.iconName     = names.legsIcon;
        item.maxStackSize = 1;
        item.creativeTab  = AxoTab_ToolsArmor;
        item.isArmor      = true;
        item.armor.slot       = AXO_ARMOR_LEGS;
        item.armor.modelIndex = modelIndex;
        item.armor.material   = material;
        if (armorTextureName) item.armor.armorTextureName = armorTextureName;
        AxoAPI_RegisterItem(&item);
    }
    // Boots
    {
        AxoItemDef item = {};
        item.name         = names.bootsName;
        item.iconName     = names.bootsIcon;
        item.maxStackSize = 1;
        item.creativeTab  = AxoTab_ToolsArmor;
        item.isArmor      = true;
        item.armor.slot       = AXO_ARMOR_FEET;
        item.armor.modelIndex = modelIndex;
        item.armor.material   = material;
        if (armorTextureName) item.armor.armorTextureName = armorTextureName;
        AxoAPI_RegisterItem(&item);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: register standard armor crafting recipes for one set
//
//  Helmet:      WWW / W W
//  Chestplate:  W W / WWW / WWW
//  Leggings:    WWW / W W / W W
//  Boots:       W W / W W
// ─────────────────────────────────────────────────────────────────────────────

static void RegisterArmorRecipes(
    const char* helmetResult,
    const char* chestResult,
    const char* legsResult,
    const char* bootsResult,
    const char* ingredient)
{
    // Helmet: top row=3, middle row: left+right (shaped 2×3)
    {
        AxoRecipeDef r = {};
        r.resultItemName = helmetResult;
        r.resultCount    = 1;
        r.isShaped       = true;
        r.recipeGroup    = AxoRecipe_Armor;
        // Row 1: W W W
        r.grid[0].itemName = ingredient; r.grid[0].count = 1;
        r.grid[1].itemName = ingredient; r.grid[1].count = 1;
        r.grid[2].itemName = ingredient; r.grid[2].count = 1;
        // Row 2: W _ W
        r.grid[3].itemName = ingredient; r.grid[3].count = 1;
        r.grid[4].itemName = "";
        r.grid[5].itemName = ingredient; r.grid[5].count = 1;
        // Row 3: empty
        r.grid[6].itemName = "";
        r.grid[7].itemName = "";
        r.grid[8].itemName = "";
        AxoAPI_RegisterRecipe(&r);
    }
    // Chestplate: W _ W / W W W / W W W
    {
        AxoRecipeDef r = {};
        r.resultItemName = chestResult;
        r.resultCount    = 1;
        r.isShaped       = true;
        r.recipeGroup    = AxoRecipe_Armor;
        // Row 1: W _ W
        r.grid[0].itemName = ingredient; r.grid[0].count = 1;
        r.grid[1].itemName = "";
        r.grid[2].itemName = ingredient; r.grid[2].count = 1;
        // Row 2: W W W
        r.grid[3].itemName = ingredient; r.grid[3].count = 1;
        r.grid[4].itemName = ingredient; r.grid[4].count = 1;
        r.grid[5].itemName = ingredient; r.grid[5].count = 1;
        // Row 3: W W W
        r.grid[6].itemName = ingredient; r.grid[6].count = 1;
        r.grid[7].itemName = ingredient; r.grid[7].count = 1;
        r.grid[8].itemName = ingredient; r.grid[8].count = 1;
        AxoAPI_RegisterRecipe(&r);
    }
    // Leggings: W W W / W _ W / W _ W
    {
        AxoRecipeDef r = {};
        r.resultItemName = legsResult;
        r.resultCount    = 1;
        r.isShaped       = true;
        r.recipeGroup    = AxoRecipe_Armor;
        // Row 1: W W W
        r.grid[0].itemName = ingredient; r.grid[0].count = 1;
        r.grid[1].itemName = ingredient; r.grid[1].count = 1;
        r.grid[2].itemName = ingredient; r.grid[2].count = 1;
        // Row 2: W _ W
        r.grid[3].itemName = ingredient; r.grid[3].count = 1;
        r.grid[4].itemName = "";
        r.grid[5].itemName = ingredient; r.grid[5].count = 1;
        // Row 3: W _ W
        r.grid[6].itemName = ingredient; r.grid[6].count = 1;
        r.grid[7].itemName = "";
        r.grid[8].itemName = ingredient; r.grid[8].count = 1;
        AxoAPI_RegisterRecipe(&r);
    }
    // Boots: W _ W / W _ W
    {
        AxoRecipeDef r = {};
        r.resultItemName = bootsResult;
        r.resultCount    = 1;
        r.isShaped       = true;
        r.recipeGroup    = AxoRecipe_Armor;
        // Row 1: W _ W
        r.grid[0].itemName = ingredient; r.grid[0].count = 1;
        r.grid[1].itemName = "";
        r.grid[2].itemName = ingredient; r.grid[2].count = 1;
        // Row 2: W _ W
        r.grid[3].itemName = ingredient; r.grid[3].count = 1;
        r.grid[4].itemName = "";
        r.grid[5].itemName = ingredient; r.grid[5].count = 1;
        // Row 3: empty
        r.grid[6].itemName = "";
        r.grid[7].itemName = "";
        r.grid[8].itemName = "";
        AxoAPI_RegisterRecipe(&r);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Mod Entry Point
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" __declspec(dllexport)
void ModEntry(AxoMod* mod, AxoAPITable* api) {
    AxoMod_SetAPI(api);
    AxoAPI_Log("Shadow's Wooden And Stone Armor v1.3.2 — Axo Port loading...");

    // ─── Armor Materials ─────────────────────────────────────────────────
    AxoArmorMaterialDef oakWoodMat  = MakeWoodMaterial("oak_wood");
    AxoArmorMaterialDef sprWoodMat  = MakeWoodMaterial("spruce_wood");
    AxoArmorMaterialDef birWoodMat  = MakeWoodMaterial("birch_wood");
    AxoArmorMaterialDef junWoodMat  = MakeWoodMaterial("jungle_wood");
    AxoArmorMaterialDef acaWoodMat  = MakeWoodMaterial("acacia_wood");
    AxoArmorMaterialDef darWoodMat  = MakeWoodMaterial("dark_oak_wood");
    AxoArmorMaterialDef stoneMat    = MakeStoneMaterial();

    // ─── Oak Wood Armor ──────────────────────────────────────────────────
    ArmorSetNames oakNames = {
        "Oak Wood Helmet",      "Oak Wood Chestplate",
        "Oak Wood Leggings",    "Oak Wood Boots",
        L"OakWoodHel",  L"OakWoodChe",
        L"OakWoodLeg",  L"OakWoodBoo"
    };
    RegisterArmorSet(oakNames, oakWoodMat, 1, "OakWoodArmor");
    RegisterArmorRecipes("Oak Wood Helmet", "Oak Wood Chestplate",
                         "Oak Wood Leggings", "Oak Wood Boots", "wood");

    // ─── Spruce Wood Armor ───────────────────────────────────────────────
    ArmorSetNames sprNames = {
        "Spruce Wood Helmet",   "Spruce Wood Chestplate",
        "Spruce Wood Leggings", "Spruce Wood Boots",
        L"SprWoodHel",  L"SprWoodChe",
        L"SprWoodLeg",  L"SprWoodBoo"
    };
    RegisterArmorSet(sprNames, sprWoodMat, 1, "SprWoodArmor");
    RegisterArmorRecipes("Spruce Wood Helmet", "Spruce Wood Chestplate",
                         "Spruce Wood Leggings", "Spruce Wood Boots", "wood");

    // ─── Birch Wood Armor ────────────────────────────────────────────────
    ArmorSetNames birNames = {
        "Birch Wood Helmet",    "Birch Wood Chestplate",
        "Birch Wood Leggings",  "Birch Wood Boots",
        L"BirWoodHel",  L"BirWoodChe",
        L"BirWoodLeg",  L"BirWoodBoo"
    };
    RegisterArmorSet(birNames, birWoodMat, 1, "BirWoodArmor");
    RegisterArmorRecipes("Birch Wood Helmet", "Birch Wood Chestplate",
                         "Birch Wood Leggings", "Birch Wood Boots", "wood");

    // ─── Jungle Wood Armor ───────────────────────────────────────────────
    ArmorSetNames junNames = {
        "Jungle Wood Helmet",   "Jungle Wood Chestplate",
        "Jungle Wood Leggings", "Jungle Wood Boots",
        L"JunWoodHel",  L"JunWoodChe",
        L"JunWoodLeg",  L"JunWoodBoo"
    };
    RegisterArmorSet(junNames, junWoodMat, 1, "JunWoodArmor");
    RegisterArmorRecipes("Jungle Wood Helmet", "Jungle Wood Chestplate",
                         "Jungle Wood Leggings", "Jungle Wood Boots", "wood");

    // ─── Acacia Wood Armor ───────────────────────────────────────────────
    ArmorSetNames acaNames = {
        "Acacia Wood Helmet",   "Acacia Wood Chestplate",
        "Acacia Wood Leggings", "Acacia Wood Boots",
        L"AcaWoodHel",  L"AcaWoodChe",
        L"AcaWoodLeg",  L"AcaWoodBoo"
    };
    RegisterArmorSet(acaNames, acaWoodMat, 1, "AcaWoodArmor");
    RegisterArmorRecipes("Acacia Wood Helmet", "Acacia Wood Chestplate",
                         "Acacia Wood Leggings", "Acacia Wood Boots", "wood");

    // ─── Dark Oak Wood Armor ─────────────────────────────────────────────
    ArmorSetNames darNames = {
        "Dark Oak Wood Helmet",    "Dark Oak Wood Chestplate",
        "Dark Oak Wood Leggings",  "Dark Oak Wood Boots",
        L"DarWoodHel",  L"DarWoodChe",
        L"DarWoodLeg",  L"DarWoodBoo"
    };
    RegisterArmorSet(darNames, darWoodMat, 1, "DarWoodArmor");
    RegisterArmorRecipes("Dark Oak Wood Helmet", "Dark Oak Wood Chestplate",
                         "Dark Oak Wood Leggings", "Dark Oak Wood Boots", "wood");

    // ─── Stone Armor ─────────────────────────────────────────────────────
    ArmorSetNames stoNames = {
        "Stone Helmet",      "Stone Chestplate",
        "Stone Leggings",    "Stone Boots",
        L"StoHel",   L"StoChe",
        L"StoLeg",   L"StoBoo"
    };
    RegisterArmorSet(stoNames, stoneMat, 1, "StoneArmor");
    RegisterArmorRecipes("Stone Helmet", "Stone Chestplate",
                         "Stone Leggings", "Stone Boots", "cobblestone");

    AxoAPI_Log("Shadow's Wooden And Stone Armor loaded! 28 armor pieces, 28 recipes registered.");
}

extern "C" __declspec(dllexport)
void OnTick() {
    // No per-tick logic needed
}

extern "C" __declspec(dllexport)
void OnShutdown() {
}
