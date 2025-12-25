#ifndef __GIANT_H__
#define __GIANT_H__

#include "Soldier.h"

class Giant : public Soldier
{
public:

    static Giant* create();

    virtual std::string getIconName() override;

    // ÖØÐ´Æ«ºÃ
    virtual BuildingType getPreferredTargetType() override;
};

#endif