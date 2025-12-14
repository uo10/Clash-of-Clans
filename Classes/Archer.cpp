#include "Archer.h"

USING_NS_CC;

Soldier::SoldierStats Archer::getStats() {
    // 血量: 80 (脆皮)
    // 伤害: 8
    // 移速: 90 (稍快)
    // 射程: 200 (远程，可以隔着墙打建筑)
    // 攻速: 1.0秒/次
    return { 80, 8, 90.0f, 200.0f, 1.0f };
}

std::string Archer::getIconName() {
    return "Archer.png";
}