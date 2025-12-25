#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Soldier.h"

class Archer : public Soldier
{
public:

    static Archer* create();

    virtual std::string getIconName() override;
};

#endif