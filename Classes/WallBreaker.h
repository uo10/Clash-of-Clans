#ifndef __WALL_BREAKER_H__
#define __WALL_BREAKER_H__

#include "Soldier.h"

class WallBreaker : public Soldier
{
public:
    static WallBreaker* create();
    virtual std::string getIconName() override;
    virtual BuildingType getPreferredTargetType() override;
    /*
    // 重写攻击逻辑：因为炸弹人攻击完要自杀
    virtual void attackTarget(float dt) override;

    // 重写寻路逻辑，让它只找墙
    virtual void findTarget() override;*/
};

#endif