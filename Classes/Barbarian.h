#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Soldier.h"

class Barbarian : public Soldier
{
public:

    static Barbarian* create();

    virtual std::string getIconName() override;
};

#endif