#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Soldier.h"

class Archer : public Soldier
{
public:

    static Archer* create();
    virtual std::string GetIconName() override;
    virtual std::string GetAttackSound() override { return "Audio/arrow.mp3"; }
};

#endif