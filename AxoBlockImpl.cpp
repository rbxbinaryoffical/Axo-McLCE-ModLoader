#include "stdafx.h"

#include <memory>
#include <string>
#include <functional>
#include <cstdio>

#include "..\..\Minecraft.World\net.minecraft.locale.h"
#include "..\..\Minecraft.World\net.minecraft.world.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\Minecraft.World\HtmlString.h"

#include "..\..\Minecraft.World\Tile.h"
#include "..\..\Minecraft.World\Material.h"
#include "..\..\Minecraft.World\IconRegister.h"
#include "..\..\Minecraft.World\Icon.h"
#include "..\..\Minecraft.World\Item.h"
#include "..\..\Minecraft.World\Player.h"
#include "..\..\Minecraft.World\Inventory.h"

#include "..\Common\UI\IUIScene_CreativeMenu.h"
#include "..\..\Minecraft.World\ItemInstance.h"
#include "..\..\Minecraft.World\TileItem.h"

#include "AxoAPI.h"
#include "AxoModLoader.h"

bool AxoModLoader_IsTerrainIconQueued(const std::wstring& name);
int  AxoAPI_ResolveItemName(const std::string& name);

class AxoBlock : public Tile {
    std::string    mDisplayName;
    int            mDropItemId;
    int            mDropCount;
    AxoRenderShape mRenderShape;
    bool           mNoCollision;
    bool           mCanBeBrokenByHand;
    int            mPlaceOnTileId;
    std::function<void(int, int, int, Level*, Player*, ItemInstance*)> mOnDestroyed;

    static Player*                  sLastPlayer;
    static shared_ptr<ItemInstance> sLastItem;

public:
    explicit AxoBlock(const AxoBlockDefInternal& def)
        : Tile(def.id, Material::stone)
        , mDisplayName(def.name)
        , mDropItemId(def.dropItemId)
        , mDropCount(def.dropCount)
        , mRenderShape(def.renderShape)
        , mNoCollision(def.noCollision)
        , mCanBeBrokenByHand(def.canBeBrokenByHand)
        , mPlaceOnTileId(def.placeOnTileId)
        , mOnDestroyed(def.onDestroyed)
    {
        setIconName(def.iconName);
        setDestroyTime(def.hardness);
        setExplodeable(def.resistance);
        if (def.renderShape == AxoShape_Cross)
            setLightBlock(0);
    }

    void registerIcons(IconRegister* iconRegister) override {
        if (AxoModLoader_IsTerrainIconQueued(iconName)) {
            icon = iconRegister->registerIcon(iconName);
        } else {
            printf("[AxoLoader] WARNING: No terrain texture for \"%ls\", using stone fallback.\n", iconName.c_str());
            icon = iconRegister->registerIcon(L"stone");
        }
    }

    float getDestroyProgress(shared_ptr<Player> player, Level* level, int x, int y, int z) override {
        sLastPlayer = player.get();
        sLastItem   = player ? player->inventory->getSelected() : nullptr;
        return Tile::getDestroyProgress(player, level, x, y, z);
    }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        if (mNoCollision) return nullptr;
        return Tile::getAABB(level, x, y, z);
    }

    bool mayPlace(Level* level, int x, int y, int z) override {
        if (mPlaceOnTileId > 0) {
            int below = level->getTile(x, y - 1, z);
            return below == mPlaceOnTileId;
        }
        return Tile::mayPlace(level, x, y, z);
    }

    int getResource(int /*data*/, Random* /*random*/, int /*bonusLevel*/) override {
        return (mDropItemId > 0) ? mDropItemId : id;
    }

    int getResourceCount(Random* /*random*/) override {
        return mDropCount;
    }

    wstring getTileItemIconName() override {
        return L"";
    }

    wstring getName() override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    int getRenderShape() override {
        if (mRenderShape == AxoShape_Cross)
            return Tile::SHAPE_CROSS_TEXTURE;
        return Tile::SHAPE_BLOCK;
    }

    bool isSolidRender(bool /*isServerLevel*/) override {
        return mRenderShape == AxoShape_Cube && !mNoCollision;
    }

    bool isCubeShaped() override {
        return mRenderShape == AxoShape_Cube && !mNoCollision;
    }

    bool axoCanBeBrokenByHand() { return mCanBeBrokenByHand; }
    bool isAxoCanBeBrokenByHand() override { return mCanBeBrokenByHand; }

    void playerDestroy(Level* level, shared_ptr<Player> player, int x, int y, int z, int data) override {
        Tile::playerDestroy(level, player, x, y, z, data);
    }

    void destroy(Level* level, int x, int y, int z, int data) override {
        Tile::destroy(level, x, y, z, data);
        if (mCanBeBrokenByHand) {
            Tile::spawnResources(level, x, y, z, data, 1.0f, 0);
        }
        if (mOnDestroyed)
            mOnDestroyed(x, y, z, level, sLastPlayer, sLastItem.get());
    }
};

Player*                  AxoBlock::sLastPlayer = nullptr;
shared_ptr<ItemInstance> AxoBlock::sLastItem   = nullptr;

class AxoTileItem : public TileItem {
    std::wstring mDisplayName;
public:
    AxoTileItem(int id, const std::string& name)
        : TileItem(id)
        , mDisplayName(name.begin(), name.end())
    {}

    std::wstring getName() override {
        return mDisplayName;
    }

    std::wstring getHoverName(shared_ptr<ItemInstance>) override {
        return mDisplayName;
    }
};

bool AxoBlock_CreateFromDef(const AxoBlockDefInternal& def) {
    new AxoBlock(def);

    if (Item::items[def.id] == nullptr)
        Item::items[def.id] = new AxoTileItem(def.id - 256, def.name);

    Tile::propagate[def.id] = false;
    Tile::mipmapEnable[def.id] = false;

    printf("[AxoLoader] Created AxoBlock id=%d \"%s\"\n", def.id, def.name.c_str());
    return true;
}

void AxoBlock_AddToCreativeMenu(int blockId, int creativeTab) {
    IUIScene_CreativeMenu::ECreative_Inventory_Groups group = IUIScene_CreativeMenu::eCreativeInventory_BuildingBlocks;
    if (creativeTab >= 0 && creativeTab < IUIScene_CreativeMenu::eCreativeInventoryGroupsCount)
        group = (IUIScene_CreativeMenu::ECreative_Inventory_Groups)creativeTab;
    IUIScene_CreativeMenu::AxoAddToGroup(group, shared_ptr<ItemInstance>(new ItemInstance(blockId, 1, 0)));
    printf("[AxoLoader] Added block id=%d to creative group %d\n", blockId, (int)group);
}
