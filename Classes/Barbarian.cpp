#include "Barbarian.h"

USING_NS_CC;

// 创建对象
Barbarian* Barbarian::Create() {
    Barbarian* pRet = new(std::nothrow) Barbarian();

    // 调用 GameUnit 的初始化函数
    // 填入数值: 图片, HP, 速度, 伤害, 射程, 类型
    if (pRet && pRet->InitUnit("Soldier/Barbarian.png", 45, 70.0f, 8.0f, 40.0f, UnitType::kSoldier)) {

        pRet->SetUnitName("Barbarian"); // 设置名字
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 获取图标名称
std::string Barbarian::GetIconName() {
    return "Barbarian.png";
}