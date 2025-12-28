#include "archer_tower.h"

USING_NS_CC;

// 创建对象
ArcherTower* ArcherTower::Create(int level)
{
    ArcherTower* pRet = new (std::nothrow) ArcherTower();
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 初始化对象
bool ArcherTower::init(int level)
{
    if (!BaseBuilding::Init(BuildingType::kArcherTower, level)) return false;
    return true;
}