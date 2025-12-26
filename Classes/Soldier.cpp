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


// 更新状态
void Soldier::update(float dt)
{
    // 如果死了，直接返回
    if (!IsAlive()) return;

    // 分流逻辑
    if (is_home_mode_) {
        // 执行游走
        UpdateWander(dt);
    }
    else {
        // 留空
    }
}

// 实现游走
void Soldier::UpdateWander(float dt)
{
    // 状态：走或者发呆
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

        // 如果到了
        if (dist < 5.0f) {
            // 随机发呆
            wander_wait_time_ = 2.0f + CCRANDOM_0_1() * 3.0f;
            wander_timer_ = 0;
        }
        else {
            // 继续走
            Vec2 dir = wander_target_ - current_pos;
            dir.normalize();

            // 移动
            float move_speed = this->GetSpeed() * 0.5f;
            Vec2 new_pos = current_pos + (dir * move_speed * dt);
            this->setPosition(new_pos);

            // 调整朝向
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

        float min_x = 0;
        float min_y = 0;
        float max_x = map_size.width * tile_size.width;
        float max_y = map_size.height * tile_size.height;

        // 设置一个内缩边距，防止兵贴着边缘走
        float padding = 50.0f;

        // 限制 X 轴
        if (potential_target.x < min_x + padding) potential_target.x = min_x + padding;
        if (potential_target.x > max_x - padding) potential_target.x = max_x - padding;

        // 限制 Y 轴
        if (potential_target.y < min_y + padding) potential_target.y = min_y + padding;
        if (potential_target.y > max_y - padding) potential_target.y = max_y - padding;
    }

    wander_target_ = potential_target;

}
