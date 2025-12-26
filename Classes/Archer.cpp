#include "Archer.h"

USING_NS_CC;

Archer* Archer::create() {
    Archer* pRet = new(std::nothrow) Archer();

    // 调用 GameUnit 的初始化函数
    // 填入数值: 图片, HP, 速度, 伤害, 射程, 类型
    if (pRet && pRet->InitUnit("Soldier/Archer.png", 20, 100.0f, 7.0f, 80.0f, UnitType::kSoldier)) {

        pRet->SetUnitName("Archer"); // 设置名字
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

std::string Archer::GetIconName() {
    return "Archer.png";
}