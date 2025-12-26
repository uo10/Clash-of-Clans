#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "game_unit.h"
#include "base_building.h"

class Soldier : public GameUnit
{
public:
    /*
    * 更新状态
    * @param dt 每帧的时间
    */
    virtual void update(float dt) override;

    /*
	* 获取图标名称
	* @return 图标文件名
    */
    virtual std::string GetIconName() = 0;

    /*
    * 开关函数
    * @param is_home 是否在大本营
    */
    void SetHomeMode(bool is_home);

    /*
    * 获取状态
	* @return 当前是否在大本营
    */
    bool GetIsHomeMode() const { return is_home_mode_; }

    /*
    * 设置游走锚点
	* @param pos 要设置的锚点位置
    */
    void SetHomePosition(cocos2d::Vec2 pos) { home_position_ = pos; }

protected:
    bool is_home_mode_ = false; // 开关
    float wander_timer_ = 0.0f; // 计时器
    float wander_wait_time_ = 0.0f; // 随机的发呆时间
    cocos2d::Vec2 wander_target_; // 随机的移动目标点
    cocos2d::Vec2 home_position_; // 记录兵营位置

    void UpdateWander(float dt); // 实现游走
    void PickNewWanderTarget(); // 随机选点

};

#endif