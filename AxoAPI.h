#pragma once
#include <string>
#include <functional>

#define AXO_ID_AUTO  0
#define AXO_DROP_SELF ""

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

struct AxoItemDef {
    int              id           = AXO_ID_AUTO;
    std::wstring     iconName;
    std::string      name;
    int              maxStackSize = 64;
    int              creativeTab  = AxoTab_Misc;
    std::function<void()> onUse   = nullptr;
    std::function<void()> onUseOn = nullptr;
};

enum AxoMaterial {
    AxoMat_Stone = 0,
    AxoMat_Wood  = 1,
    AxoMat_Grass = 2,
    AxoMat_Dirt  = 3,
    AxoMat_Metal = 4,
};

struct AxoBlockDef {
    int          id           = AXO_ID_AUTO;
    std::wstring iconName;
    std::string  name;
    AxoMaterial  material     = AxoMat_Stone;
    float        hardness     = 1.5f;
    float        resistance   = 10.0f;
    int          creativeTab  = 0;
    std::string  dropItemName = AXO_DROP_SELF; // "" = drop self, "diamond", "iron_ingot", etc.
    int          dropCount    = 1;
};

struct AxoBlockDefInternal {
    int          id;
    std::wstring iconName;
    std::string  name;
    AxoMaterial  material;
    float        hardness;
    float        resistance;
    int          creativeTab;
    int          dropItemId;
    int          dropCount;
};

struct AxoMod {
    const char* id;
};

struct AxoAPITable {
    void (*Log)(const char* modId, const char* msg);
    bool (*RegisterItem)(const AxoItemDef* def);
    bool (*RegisterBlock)(const AxoBlockDef* def);
};

#ifndef MOD_ID
#  define MOD_ID "unknown_mod"
#endif

#ifdef AXO_MOD
static AxoAPITable* gAxoAPI = nullptr;
inline void AxoMod_SetAPI(AxoAPITable* api) { gAxoAPI = api; }
#else
extern AxoAPITable* gAxoAPI;
#endif

#define AxoAPI_Log(msg)            (gAxoAPI->Log(MOD_ID, msg))
#define AxoAPI_RegisterItem(def)   (gAxoAPI->RegisterItem(def))
#define AxoAPI_RegisterBlock(def)  (gAxoAPI->RegisterBlock(def))