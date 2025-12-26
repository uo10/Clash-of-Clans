#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "game_unit.h"
#include "base_building.h"

class Soldier : public GameUnit
{
public:
    virtual void update(float dt) override;

    virtual std::string GetIconName() = 0;

    //设置是否为家乡模式（只游走不攻击）
    void SetHomeMode(bool is_home);
    bool GetIsHomeMode() const { return is_home_mode_; }
    void SetHomePosition(cocos2d::Vec2 pos) { home_position_ = pos; }

protected:
    bool is_home_mode_ = false;    // 开关
    float wander_timer_ = 0.0f;   // 计时器
    float wander_wait_time_ = 0.0f;// 随机的发呆时间
    cocos2d::Vec2 wander_target_; // 随机的移动目标点
    cocos2d::Vec2 home_position_; // 记录兵营位置

    // 游走逻辑
    void UpdateWander(float dt);
    void PickNewWanderTarget();

};

#endif