#include "Soldier.h"

USING_NS_CC;

// 开关函数
void Soldier::SetHomeMode(bool is_home) {
    this->is_home_mode_ = is_home;
    if (is_home) {
        // 隐藏血条
        if (hp_bg_)  hp_bg_->setVisible(false);
        if (hp_bar_) hp_bar_->setVisible(false);
        // 开启游走
        PickNewWanderTarget();
    }
    else {
        if (hp_bg_)  hp_bg_->setVisible(true);
        if (hp_bar_) hp_bar_->setVisible(true);
    }
}

void Soldier::update(float dt)
{
    // 如果死了，直接返回 (GameUnit 提供了 IsAlive)
    if (!IsAlive()) return;

    // 分流逻辑
    if (is_home_mode_) {
        // === 情况 A：在大本营，执行游走 ===
        UpdateWander(dt);
    }
    else {
        // === 情况 B：在战斗地图 ===
        // 调用父类的 update (如果有) 或者什么都不做
        // 因为你的 GameMap.cpp 里的 update 已经在手动调用 UpdateUnit 和 CalculatePath 了
        // 所以这里留空即可，或者调用 GameUnit::update(dt) 如果父类有逻辑的话
    }
}

// 实现游走
void Soldier::UpdateWander(float dt)
{
    // 状态机：要么在走，要么在发呆
    if (wander_wait_time_ > 0) {
        wander_wait_time_ -= dt;
        if (wander_wait_time_ <= 0) {
            PickNewWanderTarget();
        }
    }
    else {
        // 移动向目标点
        Vec2 current_pos = this->getPosition();
        float dist = current_pos.distance(wander_target_);

        // 如果到了 (距离小于 5 像素)
        if (dist < 5.0f) {
            // 随机发呆 2 到 5 秒
            wander_wait_time_ = 2.0f + CCRANDOM_0_1() * 3.0f;
            wander_timer_ = 0;
        }
        else {
            // 继续走
            Vec2 dir = wander_target_ - current_pos;
            dir.normalize();

            // 移动 (速度稍微慢一点，闲庭信步)
            float move_speed = this->GetSpeed() * 0.5f;
            Vec2 new_pos = current_pos + (dir * move_speed * dt);
            this->setPosition(new_pos);

            // 调整 ZOrder 和朝向
            this->setLocalZOrder(3000 - (int)new_pos.y);
            if (dir.x > 0) setFlippedX(true);
            else setFlippedX(false);
        }
    }
}

// 随机选点
void Soldier::PickNewWanderTarget()
{
    Vec2 current_pos = home_position_;

    // 随机半径
    float radius = 100.0f * CCRANDOM_0_1();

    // 随机角度
    float angle = CCRANDOM_0_1() * 2.0f * 3.14159f; // 0 ~ 2PI

    float offsetX = cos(angle) * radius;
    float offsetY = sin(angle) * radius;

    Vec2 potential_target = current_pos + Vec2(offsetX, offsetY);

    auto map = dynamic_cast<TMXTiledMap*>(this->getParent());

    // 边界检查
    if (map) {
        Size map_size = map->getMapSize();
        Size tile_size = map->getTileSize();

        float minX = 0;
        float minY = 0;
        float maxX = map_size.width * tile_size.width;
        float maxY = map_size.height * tile_size.height;

        // 设置一个内缩边距，防止兵贴着边缘走
        float padding = 50.0f;

        // 限制 X 轴
        if (potential_target.x < minX + padding) potential_target.x = minX + padding;
        if (potential_target.x > maxX - padding) potential_target.x = maxX - padding;

        // 限制 Y 轴
        if (potential_target.y < minY + padding) potential_target.y = minY + padding;
        if (potential_target.y > maxY - padding) potential_target.y = maxY - padding;
    }

    wander_target_ = potential_target;

}
