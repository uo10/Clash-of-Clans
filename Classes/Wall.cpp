#include "wall.h"

USING_NS_CC;

// 创建对象
Wall* Wall::Create(int level)
{
    Wall* pRet = new (std::nothrow) Wall();
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 初始化对象
bool Wall::init(int level)
{
    if (!BaseBuilding::Init(BuildingType::kWall, level)) return false;
    return true;
}