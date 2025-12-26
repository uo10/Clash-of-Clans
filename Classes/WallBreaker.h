#ifndef __WALL_BREAKER_H__
#define __WALL_BREAKER_H__

#include "Soldier.h"

class WallBreaker : public Soldier
{
public:
    static WallBreaker* create();
    virtual std::string GetIconName() override;
    virtual UnitType GetPreferredTargetType() override { return UnitType::kWall; }
    virtual void UpdateUnit(float dt) override;
};

#endif