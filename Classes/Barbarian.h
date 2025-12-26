#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Soldier.h"

class Barbarian : public Soldier
{
public:

    static Barbarian* create();
    virtual std::string GetIconName() override;
    virtual std::string GetAttackSound() override { return "Audio/sword_hit1.mp3"; }
};

#endif