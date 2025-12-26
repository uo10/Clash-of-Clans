#include "Giant.h"

USING_NS_CC;

// 创建对象
Giant* Giant::Create() {
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

// 获取图标名称
std::string Giant::GetIconName() {
    return "Giant.png";
}
