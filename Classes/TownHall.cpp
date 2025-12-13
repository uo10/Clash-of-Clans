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
    if (!BaseBuilding::init(BuildingType::TOWN_HALL, level)) return false;

    // 大本营不需要额外的 update 逻辑，除非你有特殊的防守武器
    return true;
}

// =========================================================
// 逻辑迁移：数量限制
// =========================================================
int TownHall::getMaxBuildingCount(BuildingType type, int townHallLevel)
{
    // 基础保护
    if (townHallLevel < 1&&type!= BuildingType::TOWN_HALL) return 0;

    // 大本营永远只有 1 个
    if (type == BuildingType::TOWN_HALL) return 1;

    // 兵营
    if (type == BuildingType::BARRACKS) return 1;

    // --- 基于大本营等级的限制 ---
    switch (townHallLevel) {
    case 1:
        if (type == BuildingType::GOLD_MINE) return 1;
        if (type == BuildingType::ELIXIR_PUMP) return 1;
        if (type == BuildingType::GOLD_STORAGE) return 1;
        if (type == BuildingType::ELIXIR_STORAGE) return 1;
        if (type == BuildingType::CANNON) return 1;
        if (type == BuildingType::ARCHER_TOWER) return 0;
        if (type == BuildingType::WALL) return 0;
        break;
    case 2:
        if (type == BuildingType::GOLD_MINE) return 2;
        if (type == BuildingType::ELIXIR_PUMP) return 2;
        if (type == BuildingType::GOLD_STORAGE) return 1;
        if (type == BuildingType::ELIXIR_STORAGE) return 1;
        if (type == BuildingType::CANNON) return 2;
        if (type == BuildingType::ARCHER_TOWER) return 1;
        if (type == BuildingType::WALL) return 25;
        break;
    case 3:
    default: // 3级及以上
        if (type == BuildingType::GOLD_MINE) return 3;
        if (type == BuildingType::ELIXIR_PUMP) return 3;
        if (type == BuildingType::GOLD_STORAGE) return 2;
        if (type == BuildingType::ELIXIR_STORAGE) return 2;
        if (type == BuildingType::CANNON) return 2;
        if (type == BuildingType::ARCHER_TOWER) return 1;
        if (type == BuildingType::WALL) return 50;
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
    case BuildingType::GOLD_MINE:
        // 金矿升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;
    case BuildingType::ELIXIR_PUMP:
        // 圣水收集器升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;
    case BuildingType::GOLD_STORAGE:
        // 金库升到2,3级需要大本营2级
        if (targetLevel >= 2 && townHallLevel < 2) return false;
        break;
    case BuildingType::ELIXIR_STORAGE:
        // 圣水瓶升到2,3级需要大本营2级
        if (targetLevel >= 2 && townHallLevel < 2) return false;
        break;
    case BuildingType::WALL:
        // 墙升到3级需要大本营3级
        if (targetLevel >= 3 && townHallLevel < 3) return false;
        break;

    case BuildingType::CANNON:
        // 炮升到3级需要大本营2级
        if (targetLevel >= 3 && townHallLevel < 2) return false;
        break;

    case BuildingType::ARCHER_TOWER:
        // 塔升到1级(建造)需要大本营2级
        if (targetLevel == 1 && townHallLevel < 2) return false;
        // 塔升到3级需要大本营3级
        if (targetLevel >= 3 && townHallLevel < 3) return false;
        break;

    case BuildingType::TOWN_HALL:
        // 大本营自己升级不受限制 (只要钱够)
        return true;
    }

    return true; // 默认允许
}