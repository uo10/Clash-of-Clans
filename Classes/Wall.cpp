#include "Wall.h"

USING_NS_CC;

Wall* Wall::Create(int level)
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
    if (!BaseBuilding::Init(BuildingType::kWall, level)) return false;
    return true;
}