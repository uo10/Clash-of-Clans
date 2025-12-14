#include "Barbarian.h"

USING_NS_CC;

Soldier::SoldierStats Barbarian::getStats() {
    // 血量: 150 (中等)
    // 伤害: 20
    // 移速: 80
    // 射程: 2 (近战，大概是一个格子的距离)
    // 攻速: 1.0秒/次
    return { 150, 20, 80.0f, 2.0f, 1.0f };
}

std::string Barbarian::getIconName() {
    return "Barbarian.png";
}