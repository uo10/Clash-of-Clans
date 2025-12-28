#ifndef __TRAP_H__
#define __TRAP_H__

#include "cocos2d.h"
#include "game_unit.h"

class GameTrap : public cocos2d::Node {
public:
    /**
    * 创建陷阱
    * @param pos 要创建陷阱的位置
    * @param size 地图瓦块大小
    */
    static GameTrap* CreateTrap(cocos2d::Vec2 pos,cocos2d::Size size);
    /**
    * 按帧更新陷阱状态
    * @param dt 每一帧的时间
    * @param enemies 地图上现有的士兵集合
    */
    void UpdateTrap(float dt, const cocos2d::Vector<GameUnit*>& enemies);

private:
    bool is_able_;          // 是否有效，若已触发则无效
    float trigger_range_;    // 触发范围
    float damage_range_;     // 爆炸伤害范围
    float damage_;          // 伤害值
    cocos2d::Sprite* sprite_; // 陷阱精灵
    /**
    * 激活陷阱，对爆炸范围内的敌人造成伤害
    * @param enemies 地图上现有的士兵集合
    */
    void Activate(const cocos2d::Vector<GameUnit*>& enemies); 
};
#endif
