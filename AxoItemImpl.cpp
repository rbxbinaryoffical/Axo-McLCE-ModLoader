#include "stdafx.h"

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <cstdio>

#include "..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.ai.attributes.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\Minecraft.World\net.minecraft.locale.h"
#include "..\..\Minecraft.World\net.minecraft.world.h"
#include "..\..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\..\Minecraft.World\HtmlString.h"

#include "..\..\Minecraft.World\Item.h"
#include "..\..\Minecraft.World\FoodItem.h"
#include "..\..\Minecraft.World\IconRegister.h"
#include "..\..\Minecraft.World\Icon.h"
#include "..\..\Minecraft.World\MobEffectInstance.h"
#include "..\..\Minecraft.World\ArmorItem.h"
#include "..\..\Minecraft.World\com.mojang.nbt.h"

#include "..\Common\UI\IUIScene_CreativeMenu.h"

#include "AxoAPI.h"

extern int AxoAPI_ResolveItemName(const std::string& name);

static int ResolveEffectName(const std::string& name) {
    static const std::unordered_map<std::string, int> kMap = {
        {"speed",           1},
        {"slowness",        2},
        {"haste",           3},
        {"mining_fatigue",  4},
        {"strength",        5},
        {"instant_health",  6},
        {"instant_damage",  7},
        {"jump_boost",      8},
        {"nausea",          9},
        {"regeneration",   10},
        {"resistance",     11},
        {"fire_resistance",12},
        {"water_breathing",13},
        {"invisibility",   14},
        {"blindness",      15},
        {"night_vision",   16},
        {"hunger",         17},
        {"weakness",       18},
        {"poison",         19},
        {"wither",         20},
        {"health_boost",   21},
        {"absorption",     22},
        {"saturation",     23},
    };
    auto it = kMap.find(name);
    if (it != kMap.end()) return it->second;
    printf("[AxoLoader] ResolveEffectName: unknown effect \"%s\"\n", name.c_str());
    return -1;
}

class AxoItem : public Item {
public:
    std::wstring           mIconName;
    std::string            mDisplayName;
    std::function<void()>  mOnUse;
    std::function<void()>  mOnUseOn;
    int                    mAttackDamage;
    float                  mMiningSpeed;
    bool                   mIsPickaxe;
    bool                   mIsAxe;
    bool                   mIsShovel;

    explicit AxoItem(const AxoItemDefI& def)
        : Item(def.id - 256)
        , mIconName(def.iconName)
        , mDisplayName(def.name)
        , mOnUse(def.onUse)
        , mOnUseOn(def.onUseOn)
        , mAttackDamage(def.attackDamage)
        , mMiningSpeed(def.miningSpeed)
        , mIsPickaxe(def.isPickaxe)
        , mIsAxe(def.isAxe)
        , mIsShovel(def.isShovel)
    {
        maxStackSize = def.maxStackSize;
        setIconName(mIconName);
    }

    void registerIcons(IconRegister* iconRegister) override {
        icon = iconRegister->registerIcon(mIconName);
    }

    std::wstring getName() override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    std::wstring getHoverName(shared_ptr<ItemInstance>) override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    attrAttrModMap* getDefaultAttributeModifiers() override {
        attrAttrModMap* result = Item::getDefaultAttributeModifiers();
        if (mAttackDamage > 1) {
            result->insert(attrAttrModMap::value_type(
                SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
                new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, (float)mAttackDamage, AttributeModifier::OPERATION_ADDITION)
            ));
        }
        return result;
    }

    float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile* tile) override {
        if (tile == nullptr) return 1.0f;
        Material* mat = tile->material;
        if (mIsPickaxe && (mat == Material::stone || mat == Material::metal || mat == Material::heavyMetal))
            return mMiningSpeed;
        if (mIsAxe && (mat == Material::wood || mat == Material::plant || mat == Material::leaves))
            return mMiningSpeed;
        if (mIsShovel && (mat == Material::dirt || mat == Material::sand || mat == Material::clay || mat == Material::snow))
            return mMiningSpeed;
        if (!mIsPickaxe && !mIsAxe && !mIsShovel && mMiningSpeed != 1.0f)
            return mMiningSpeed;
        return 1.0f;
    }

    bool canDestroySpecial(Tile* tile) override {
        if (tile == nullptr) return false;
        Material* mat = tile->material;
        if (mIsPickaxe && (mat == Material::stone || mat == Material::metal || mat == Material::heavyMetal))
            return true;
        if (mIsAxe && mat == Material::wood)
            return true;
        if (mIsShovel && (mat == Material::dirt || mat == Material::sand))
            return true;
        return false;
    }
};

class AxoFoodItem : public FoodItem {
public:
    std::wstring           mIconName;
    std::string            mDisplayName;
    std::function<void()>  mOnUse;
    std::function<void()>  mOnUseOn;
    int                    mAttackDamage;
    float                  mMiningSpeed;
    bool                   mIsPickaxe;
    bool                   mIsAxe;
    bool                   mIsShovel;
    std::string            mEffectName;
    int                    mEffectDuration;
    int                    mEffectAmplifier;

    explicit AxoFoodItem(const AxoItemDefI& def)
        : FoodItem(def.id - 256, def.food.nutrition, def.food.saturation, def.food.isMeat)
        , mIconName(def.iconName)
        , mDisplayName(def.name)
        , mOnUse(def.onUse)
        , mOnUseOn(def.onUseOn)
        , mAttackDamage(def.attackDamage)
        , mMiningSpeed(def.miningSpeed)
        , mIsPickaxe(def.isPickaxe)
        , mIsAxe(def.isAxe)
        , mIsShovel(def.isShovel)
        , mEffectName(def.food.effect.effectName)
        , mEffectDuration(def.food.effect.duration)
        , mEffectAmplifier(def.food.effect.amplifier)
    {
        maxStackSize = def.maxStackSize;
        setIconName(mIconName);
        if (def.food.canAlwaysEat)
            setCanAlwaysEat();
    }

    void registerIcons(IconRegister* iconRegister) override {
        icon = iconRegister->registerIcon(mIconName);
    }

    std::wstring getName() override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    std::wstring getHoverName(shared_ptr<ItemInstance>) override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level* level, shared_ptr<Player> player) override {
        shared_ptr<ItemInstance> result = FoodItem::useTimeDepleted(instance, level, player);
        if (!mEffectName.empty() && mEffectDuration > 0 && !level->isClientSide) {
            int effectId = ResolveEffectName(mEffectName);
            if (effectId >= 0)
                player->addEffect(new MobEffectInstance(effectId, mEffectDuration, mEffectAmplifier));
        }
        return result;
    }

    attrAttrModMap* getDefaultAttributeModifiers() override {
        attrAttrModMap* result = Item::getDefaultAttributeModifiers();
        if (mAttackDamage > 1) {
            result->insert(attrAttrModMap::value_type(
                SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
                new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, (float)mAttackDamage, AttributeModifier::OPERATION_ADDITION)
            ));
        }
        return result;
    }

    float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile* tile) override {
        if (tile == nullptr) return 1.0f;
        Material* mat = tile->material;
        if (mIsPickaxe && (mat == Material::stone || mat == Material::metal || mat == Material::heavyMetal))
            return mMiningSpeed;
        if (mIsAxe && (mat == Material::wood || mat == Material::plant || mat == Material::leaves))
            return mMiningSpeed;
        if (mIsShovel && (mat == Material::dirt || mat == Material::sand || mat == Material::clay || mat == Material::snow))
            return mMiningSpeed;
        if (!mIsPickaxe && !mIsAxe && !mIsShovel && mMiningSpeed != 1.0f)
            return mMiningSpeed;
        return 1.0f;
    }

    bool canDestroySpecial(Tile* tile) override {
        if (tile == nullptr) return false;
        Material* mat = tile->material;
        if (mIsPickaxe && (mat == Material::stone || mat == Material::metal || mat == Material::heavyMetal))
            return true;
        if (mIsAxe && mat == Material::wood)
            return true;
        if (mIsShovel && (mat == Material::dirt || mat == Material::sand))
            return true;
        return false;
    }
};


class AxoArmorItem : public ArmorItem {
public:
    std::wstring  mIconName;
    std::string   mDisplayName;
    std::string   mRepairItemName;
    bool          mIsDyeable;
    int           mDefaultColor;
    Icon*         mOverlayIcon;
    std::function<void(Level*, Player*, ItemInstance*)> mOnArmorTick;

    explicit AxoArmorItem(const AxoItemDefI& def, ArmorItem::ArmorMaterial* mat)
        : ArmorItem(def.id - 256, mat, def.armor.modelIndex, def.armor.slot)
        , mIconName(def.iconName)
        , mDisplayName(def.name)
        , mRepairItemName(def.armor.material.repairItemName)
        , mIsDyeable(def.armor.isDyeable)
        , mDefaultColor(def.armor.defaultColor)
        , mOverlayIcon(nullptr)
        , mOnArmorTick(def.armor.onArmorTick)
    {
        setIconName(mIconName);
    }


    void registerIcons(IconRegister* iconRegister) override {
        icon = iconRegister->registerIcon(mIconName);
        if (mIsDyeable) {
            std::wstring overlayName = mIconName + L"_overlay";
            mOverlayIcon = iconRegister->registerIcon(overlayName);
        }
    }

    // ── Display name ─────────────────────────────────────────────────────────

    std::wstring getName() override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    std::wstring getHoverName(shared_ptr<ItemInstance>) override {
        return std::wstring(mDisplayName.begin(), mDisplayName.end());
    }

    // ── Dyeable color support (Forge-compatible) ─────────────────────────────

    bool hasMultipleSpriteLayers() override {
        return mIsDyeable;
    }

    int getColor(shared_ptr<ItemInstance> item, int spriteLayer) override {
        if (!mIsDyeable) return 0xFFFFFF;
        if (spriteLayer > 0) return 0xFFFFFF;   // overlay layer is untinted
        int color = getColor(item);
        if (color < 0) color = 0xFFFFFF;
        return color;
    }

    bool hasCustomColor(shared_ptr<ItemInstance> item) override {
        if (!mIsDyeable) return false;
        if (!item->hasTag()) return false;
        if (!item->getTag()->contains(L"display")) return false;
        if (!item->getTag()->getCompound(L"display")->contains(L"color")) return false;
        return true;
    }

    int getColor(shared_ptr<ItemInstance> item) override {
        if (!mIsDyeable) return -1;
        CompoundTag* tag = item->getTag();
        if (tag == nullptr) return mDefaultColor;
        CompoundTag* display = tag->getCompound(L"display");
        if (display == nullptr) return mDefaultColor;
        if (display->contains(L"color")) return display->getInt(L"color");
        return mDefaultColor;
    }

    void setColor(shared_ptr<ItemInstance> item, int color) override {
        if (!mIsDyeable) return;
        CompoundTag* tag = item->getTag();
        if (tag == nullptr) {
            tag = new CompoundTag();
            item->setTag(tag);
        }
        CompoundTag* display = tag->getCompound(L"display");
        if (!tag->contains(L"display")) tag->putCompound(L"display", display);
        display->putInt(L"color", color);
    }

    void clearColor(shared_ptr<ItemInstance> item) override {
        if (!mIsDyeable) return;
        CompoundTag* tag = item->getTag();
        if (tag == nullptr) return;
        CompoundTag* display = tag->getCompound(L"display");
        if (display != nullptr && display->contains(L"color"))
            display->remove(L"color");
    }

    Icon* getLayerIcon(int auxValue, int spriteLayer) override {
        if (mIsDyeable && spriteLayer == 1 && mOverlayIcon)
            return mOverlayIcon;
        return Item::getLayerIcon(auxValue, spriteLayer);
    }

    // ── Repair (custom material support) ─────────────────────────────────────

    bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem) override {
        if (!mRepairItemName.empty()) {
            int repairId = AxoAPI_ResolveItemName(mRepairItemName);
            if (repairId >= 0 && repairItem->id == repairId)
                return true;
        }
        return Item::isValidRepairItem(source, repairItem);
    }
};

bool AxoItem_CreateFromDef(const AxoItemDefI& def) {
    printf("[AxoLoader] CreateFromDef id=%d edible=%d nutrition=%d sat=%.2f isMeat=%d\n",
        def.id, (int)def.isEdible, def.food.nutrition, def.food.saturation, (int)def.food.isMeat);
    fflush(stdout);
    if (Item::items[def.id] != NULL) {
        printf("[AxoLoader] id %d already taken, skipping \"%s\".\n", def.id, def.name.c_str());
        return false;
    }
    if (def.isArmor) {
        printf("[AxoLoader] Creating ArmorItem slot=%d model=%d dur=%d ench=%d dyeable=%d...\n",
            def.armor.slot, def.armor.modelIndex,
            def.armor.material.durabilityMultiplier,
            def.armor.material.enchantability,
            (int)def.armor.isDyeable);
        fflush(stdout);
        int* protections = new int[4];
        for (int i = 0; i < 4; i++)
            protections[i] = def.armor.material.slotProtections[i];
        auto* mat = new ArmorItem::ArmorMaterial(
            def.armor.material.durabilityMultiplier, protections,
            def.armor.material.enchantability);
        new AxoArmorItem(def, mat);
        printf("[AxoLoader] ArmorItem created OK\n"); fflush(stdout);
    } else if (def.isEdible) {
        printf("[AxoLoader] Creating FoodItem...\n"); fflush(stdout);
        new AxoFoodItem(def);
        printf("[AxoLoader] FoodItem created OK\n"); fflush(stdout);
    } else {
        printf("[AxoLoader] Creating Item...\n"); fflush(stdout);
        new AxoItem(def);
        printf("[AxoLoader] Item created OK\n"); fflush(stdout);
    }
    printf("[AxoLoader] Created AxoItem id=%d \"%s\"%s%s\n",
           def.id, def.name.c_str(),
           def.isEdible ? " (edible)" : "",
           def.isArmor  ? " (armor)"  : "");
    return true;
}

void AxoItem_AddToCreativeMenu(int itemId, int creativeTab) {
    IUIScene_CreativeMenu::ECreative_Inventory_Groups group = IUIScene_CreativeMenu::eCreativeInventory_Misc;
    if (creativeTab >= 0 && creativeTab < IUIScene_CreativeMenu::eCreativeInventoryGroupsCount)
        group = (IUIScene_CreativeMenu::ECreative_Inventory_Groups)creativeTab;
    IUIScene_CreativeMenu::AxoAddToGroup(group, shared_ptr<ItemInstance>(new ItemInstance(itemId, 1, 0)));
    printf("[AxoLoader] Added item id=%d to creative group %d\n", itemId, (int)group);
}