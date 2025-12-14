#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Soldier.h"

class Archer : public Soldier
{
public:
    CREATE_FUNC(Archer);

    virtual SoldierStats getStats() override;
    virtual std::string getIconName() override;
};

#endif