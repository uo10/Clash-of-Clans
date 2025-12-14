#include "Soldier.h"

USING_NS_CC;

bool Soldier::init()
{
    // 1. 获取子类定义的图片
    if (!Sprite::initWithFile(getIconName())) {
        // 容错：没图就画个色块
        setTextureRect(Rect(0, 0, 32, 32));
        setColor(Color3B::MAGENTA);
    }

    // 2. 获取子类定义的数值
    stats = getStats();
    currentHP = stats.maxHP;
    attackTimer = 0;
    state = State::IDLE;
    target = nullptr;

    // 3. 血条
    hpBar = DrawNode::create();
    this->addChild(hpBar, 10);
    updateHPBar();

    // 4. 开启逻辑循环
    this->scheduleUpdate();

    return true;
}

void Soldier::update(float dt)
{
    if (state == State::DEAD) return;

    // 1. 校验目标是否还存在
    if (target) {
        if (!target->getParent() || target->state == BuildingState::DESTROYED) {
            target = nullptr; // 目标没了
            state = State::IDLE;
        }
    }

    // 2. 如果没有目标，去寻找
    if (!target) {
        findTarget();
    }

    // 3. 如果还是没有目标 (全图空了)，发呆
    if (!target) {
        state = State::IDLE;
        return;
    }

    // 4. 计算距离
    float dist = this->getPosition().distance(target->getPosition());

    // 判定范围：射程 + 建筑半径(估算40)
    if (dist <= stats.attackRange + 40.0f) {
        state = State::ATTACK;
        attackTarget(dt);
    }
    else {
        state = State::MOVE;
        moveTowardTarget(dt);
    }
}

bool Soldier::isValidTarget(BaseBuilding* building, BuildingType pref)
{
    // 1. 基础检查：建筑必须存在且活着
    if (!building || building->state == BuildingState::DESTROYED) return false;

    // 2. 如果偏好是 NONE，说明什么都吃，只排除障碍物或特殊物体
    if (pref == BuildingType::NONE) return true;

    // 3. === 巨人逻辑 (CANNON 代表防御类) ===
    if (pref == BuildingType::CANNON) {
        // 只要是防御塔都算
        if (building->type == BuildingType::CANNON ||
            building->type == BuildingType::ARCHER_TOWER) {
            return true;
        }
        return false;
    }

    // 4. === 炸弹人逻辑 ===
    if (pref == BuildingType::WALL) {
        return (building->type == BuildingType::WALL);
    }

    // 默认不匹配
    return false;
}

// 寻路逻辑
void Soldier::findTarget()
{
    Node* map = this->getParent();
    if (!map) return;

    BaseBuilding* bestTarget = nullptr;
    float minDst = 999999.0f;
    Vec2 myPos = this->getPosition();

    // 1. 获取子类的偏好
    BuildingType pref = getPreferredTargetType();

    // 2. 第一轮扫描：只找偏好的目标
    if (pref != BuildingType::NONE) {
        for (auto child : map->getChildren()) {
            if (child->getTag() == 999) {
                auto b = dynamic_cast<BaseBuilding*>(child);

                // 【调用辅助函数】核心逻辑在这里
                if (isValidTarget(b, pref)) {
                    float d = myPos.distance(b->getPosition());
                    if (d < minDst) {
                        minDst = d;
                        bestTarget = b;
                    }
                }
            }
        }
    }

    // 3. 第二轮扫描：如果第一轮没找到（或没偏好），找最近的任意目标
    if (!bestTarget) {
        minDst = 999999.0f;
        for (auto child : map->getChildren()) {
            if (child->getTag() == 999) {
                auto b = dynamic_cast<BaseBuilding*>(child);
                // 这里 pref 传 NONE，表示任意活着的建筑都行
                if (isValidTarget(b, BuildingType::NONE)) {
                    float d = myPos.distance(b->getPosition());
                    if (d < minDst) {
                        minDst = d;
                        bestTarget = b;
                    }
                }
            }
        }
    }

    target = bestTarget;
}

void Soldier::moveTowardTarget(float dt)
{
    if (!target) return;

    Vec2 dir = target->getPosition() - this->getPosition();
    dir.normalize();

    Vec2 newPos = this->getPosition() + (dir * stats.moveSpeed * dt);
    this->setPosition(newPos);

    // 动态 ZOrder (遮挡关系)
    this->setLocalZOrder(3000 - (int)newPos.y);

    // 翻转朝向
    if (dir.x > 0) setFlippedX(true); // 假设原图朝左
    else setFlippedX(false);
}

void Soldier::attackTarget(float dt)
{
    attackTimer += dt;
    if (attackTimer >= stats.attackSpeed) {
        attackTimer = 0;

        if (target) {
            target->takeDamage(stats.damage);

            auto scaleUp = ScaleBy::create(0.1f, 1.2f);
            auto scaleDown = scaleUp->reverse(); // 自动计算反向动作 (除以 1.2)


            // 简单攻击动画
            this->runAction(Sequence::create(scaleUp, scaleDown, nullptr));

        }
    }
}

void Soldier::updateHPBar()
{
    if (!hpBar) return;
    hpBar->clear();

    // 血条尺寸
    Vec2 size(30, 4);
    // 放在头顶上方
    Vec2 pos(-size.x / 2, 35);

    // 必须强转 float，否则 int/int 会变成 0
    float percent = currentHP / (float)stats.maxHP;
    if (percent < 0) percent = 0;

    // 绘制背景（灰色）
    hpBar->drawSolidRect(pos, pos + size, Color4F::GRAY);
    // 绘制血量（红色）
    hpBar->drawSolidRect(pos, pos + Vec2(size.x * percent, size.y), Color4F::RED);
}

void Soldier::takeDamage(float dmg)
{
    // 如果已经死了，就不要再鞭尸了
    if (state == State::DEAD) return;

    // 1. 扣血
    currentHP -= dmg;
    if (currentHP < 0) currentHP = 0;

    // 2. 刷新头顶血条
    updateHPBar();

    // 3. 受击反馈 (变红闪烁一下)
    auto tintRed = TintTo::create(0.1f, 255, 100, 100);
    auto tintBack = TintTo::create(0.1f, 255, 255, 255);
    this->runAction(Sequence::create(tintRed, tintBack, nullptr));

    CCLOG("Soldier took %.0f damage, HP: %.0f", dmg, currentHP);

    // 4. 死亡判定
    if (currentHP <= 0) {
        die();
    }
}

void Soldier::die()
{
    state = State::DEAD;

    CCLOG("Soldier Died!");

    // 播放死亡动画（可选），这里直接移除
    // 稍微延迟一丢丢移除，让“变红”效果能显示出来，也防止逻辑崩溃
    this->runAction(Sequence::create(
        FadeOut::create(0.2f),
        RemoveSelf::create(),
        nullptr
    ));
}