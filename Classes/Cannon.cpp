#include "cannon.h"

USING_NS_CC;

// 创建对象
Cannon* Cannon::Create(int level)
{
    Cannon* pRet = new (std::nothrow) Cannon();
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 初始化对象
bool Cannon::init(int level)
{
    if (!BaseBuilding::Init(BuildingType::kCannon, level)) return false;
    return true;
}