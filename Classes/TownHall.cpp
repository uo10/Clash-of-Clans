#include "TownHall.h"

USING_NS_CC;

TownHall* TownHall::create(int level)
{
    TownHall* pRet = new (std::nothrow) TownHall();
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool TownHall::init(int level)
{
    // 初始化父类 (会自动加载 TownHall1.png 等)
    if (!BaseBuilding::Init(BuildingType::kTownHall, level)) return false;

    // 大本营不需要额外的 update 逻辑，除非你有特殊的防守武器
    return true;
}

// =========================================================
// 逻辑迁移：数量限制
// =========================================================
int TownHall::getMaxBuildingCount(BuildingType type, int townHallLevel)
{
    // 基础保护
    if (townHallLevel < 1&&type!= BuildingType::kTownHall) return 0;

    // 大本营永远只有 1 个
    if (type == BuildingType::kTownHall) return 1;

    // 兵营
    if (type == BuildingType::kBarracks) return 1;

    // --- 基于大本营等级的限制 ---
    switch (townHallLevel) {
    case 1:
        if (type == BuildingType::kGoldMine) return 1;
        if (type == BuildingType::kElixirPump) return 1;
        if (type == BuildingType::kGoldStorage) return 1;
        if (type == BuildingType::kElixirStorage) return 1;
        if (type == BuildingType::kCannon) return 1;
        if (type == BuildingType::kArcherTower) return 0;
        if (type == BuildingType::kWall) return 0;
        break;
    case 2:
        if (type == BuildingType::kGoldMine) return 2;
        if (type == BuildingType::kElixirPump) return 2;
        if (type == BuildingType::kGoldStorage) return 1;
        if (type == BuildingType::kElixirStorage) return 1;
        if (type == BuildingType::kCannon) return 2;
        if (type == BuildingType::kArcherTower) return 1;
        if (type == BuildingType::kWall) return 25;
        break;
    case 3:
    default: // 3级及以上
        if (type == BuildingType::kGoldMine) return 3;
        if (type == BuildingType::kElixirPump) return 3;
        if (type == BuildingType::kGoldStorage) return 2;
        if (type == BuildingType::kElixirStorage) return 2;
        if (type == BuildingType::kCannon) return 2;
        if (type == BuildingType::kArcherTower) return 1;
        if (type == BuildingType::kWall) return 50;
        break;
    }

    // 如果上面的 switch 没覆盖到
    // 返回 0 或 1，视需求而定
    return 0;
}

// =========================================================
// 逻辑迁移：升级限制
// =========================================================
bool TownHall::isUpgradeAllowed(BuildingType buildingType, int targetLevel, int townHallLevel)
{
    // 如果没有大本营，默认禁止高级升级
    if (townHallLevel <= 0) return false;

    switch (buildingType)
    {
    case BuildingType::kGoldMine:
        // 金矿升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;
    case BuildingType::kElixirPump:
        // 圣水收集器升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;
    case BuildingType::kGoldStorage:
        // 金库升到2,3级需要大本营2级
        if (targetLevel >= 2 && townHallLevel < 2) return false;
        break;
    case BuildingType::kElixirStorage:
        // 圣水瓶升到2,3级需要大本营2级
        if (targetLevel >= 2 && townHallLevel < 2) return false;
        break;
    case BuildingType::kWall:
        // 墙升到3级需要大本营3级
        if (targetLevel >= 3 && townHallLevel < 3) return false;
        break;

    case BuildingType::kCannon:
        // 炮升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;

    case BuildingType::kArcherTower:
        // 塔升到1级(建造)需要大本营2级
        if (targetLevel == 1 && townHallLevel < 2) return false;
        // 塔升到3级需要大本营3级
        if (targetLevel >= 3 && townHallLevel < 3) return false;
        break;

    case BuildingType::kTownHall:
        // 大本营自己升级不受限制 (只要钱够)
        return true;
    }

    return true; // 默认允许
}