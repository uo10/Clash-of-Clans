#include "Wall.h"

USING_NS_CC;

Wall* Wall::create(int level)
{
    Wall* pRet = new (std::nothrow) Wall();
    // 强制指定类型为 WALL
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Wall::init(int level)
{
    if (!BaseBuilding::init(BuildingType::WALL, level)) return false;
    return true;
}