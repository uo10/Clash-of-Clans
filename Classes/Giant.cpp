#include "Giant.h"

USING_NS_CC;

Giant* Giant::create() {
    Giant* pRet = new(std::nothrow) Giant();

    // 调用 GameUnit 的初始化函数
    // 填入数值: 图片, HP, 速度, 伤害, 射程, 类型
    if (pRet && pRet->InitUnit("Soldier/Giant.png", 300, 50.0f, 11.0f, 40.0f, UnitType::kSoldier)) {

        pRet->SetUnitName("Giant"); // 设置名字
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

std::string Giant::getIconName() {
    return "Giant.png";
}

BuildingType Giant::getPreferredTargetType() {
    // 返回 CANNON，在 Soldier::findTarget 中，我们约定这代表“所有防御建筑”
    return BuildingType::kCannon;
}