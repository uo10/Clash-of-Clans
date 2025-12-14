#ifndef __GIANT_H__
#define __GIANT_H__

#include "Soldier.h"

class Giant : public Soldier
{
public:
    CREATE_FUNC(Giant);

    virtual SoldierStats getStats() override;
    virtual std::string getIconName() override;

    // 重写偏好：告诉基类我想打防御塔
    virtual BuildingType getPreferredTargetType() override;
};

#endif