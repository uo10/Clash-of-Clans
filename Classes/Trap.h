#ifndef __TRAP_H__
#define __TRAP_H__

#include "cocos2d.h"
#include "GameUnit.h"

class GameTrap : public cocos2d::Node {
public:
    /**
    * 创建陷阱
    * @param pos 要创建陷阱的位置
    * @param size 地图瓦块大小
    */
    static GameTrap* createTrap(cocos2d::Vec2 pos,cocos2d::Size size);
    /**
    * 按帧更新陷阱状态
    * @param dt 每一帧的时间
    * @param enemies 地图上现有的士兵集合
    */
    void updateTrap(float dt, const cocos2d::Vector<GameUnit*>& enemies);

private:
    bool _isAble;          // 是否有效，若已触发则无效
    float _triggerRange;    // 触发范围
    float _damageRange;     // 爆炸伤害范围
    float _damage;          // 伤害值
    cocos2d::Sprite* _sprite; // 陷阱精灵
    /**
    * 激活陷阱，对爆炸范围内的敌人造成伤害
    * @param enemies 地图上现有的士兵集合
    */
    void activate(const cocos2d::Vector<GameUnit*>& enemies); 
};
#endif
