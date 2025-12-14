#include "WallBreaker.h"

USING_NS_CC;

Soldier::SoldierStats WallBreaker::getStats() {
    // 血量: 40 (极脆，还没跑到可能就被塔打死了)
    // 伤害: 400 (非常高，用于秒杀墙)
    // 移速: 120 (非常快)
    // 射程: 30 (贴脸爆炸)
    // 攻速: 1.0秒/次 (其实只攻击一次)
    return { 40, 400, 120.0f, 30.0f, 1.0f };
}

std::string WallBreaker::getIconName() {
    return "Wall_Breaker.png";
}

BuildingType WallBreaker::getPreferredTargetType() {
    // 优先找墙
    return BuildingType::WALL;
}

void WallBreaker::attackTarget(float dt) {
    // 只有当目标存在时才自爆
    if (target) {
        // 1. 造成伤害
        target->takeDamage(stats.damage);

        CCLOG("WallBreaker BOOM!");

        // 2. 将状态设为 DEAD，停止一切逻辑
        state = State::DEAD;

        // 3. 播放自爆特效并移除
        this->runAction(Sequence::create(
            // 如果有爆炸动画，在这里播放
            // DelayTime::create(0.1f), 
            RemoveSelf::create(),
            nullptr
        ));
    }
}

void WallBreaker::findTarget()
{
    Node* map = this->getParent();
    if (!map) return;

    BaseBuilding* bestTarget = nullptr;
    float minDst = 999999.0f;
    Vec2 myPos = this->getPosition();

    // 遍历地图所有子节点
    for (auto child : map->getChildren()) {
        if (child->getTag() == 999) {
            auto b = dynamic_cast<BaseBuilding*>(child);

            // 1. 必须是活着的建筑
            if (b && b->state != BuildingState::DESTROYED) {

                // 2. 只允许 WALL 类型通过
                // 如果不是墙，直接跳过 (continue)
                if (b->type != BuildingType::WALL) {
                    continue;
                }

                // 3. 计算距离，找最近的墙
                float d = myPos.distance(b->getPosition());
                if (d < minDst) {
                    minDst = d;
                    bestTarget = b;
                }
            }
        }
    }

    // 更新目标 (如果没有墙，bestTarget 就是 nullptr，炸弹人就会待机)
    target = bestTarget;
}