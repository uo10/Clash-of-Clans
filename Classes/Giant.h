#ifndef __GIANT_H__
#define __GIANT_H__

#include "Soldier.h"

class Giant : public Soldier
{
public:

    static Giant* create();

    virtual std::string GetIconName() override;
    virtual std::string GetAttackSound() override { return "Audio/punch.mp3"; }
    virtual UnitType GetPreferredTargetType() override { return UnitType::kBuildingDefence; }
};

#endif