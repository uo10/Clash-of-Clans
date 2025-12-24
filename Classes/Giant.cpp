#include "Giant.h"

USING_NS_CC;

Soldier::SoldierStats Giant::getStats() {
    // 血量: 600 (非常厚)
    // 伤害: 25
    // 移速: 40 (很慢)
    // 射程: 40 (近战)
    // 攻速: 2.0秒/次 (攻击慢)
    return { 600, 25, 40.0f, 40.0f, 2.0f };
}

std::string Giant::getIconName() {
    return "Giant.png";
}

BuildingType Giant::getPreferredTargetType() {
    // 返回 CANNON，在 Soldier::findTarget 中，我们约定这代表“所有防御建筑”
    return BuildingType::kCannon;
}