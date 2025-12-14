#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Soldier.h"

class Barbarian : public Soldier
{
public:
    CREATE_FUNC(Barbarian);

    // 重写基类纯虚函数
    virtual SoldierStats getStats() override;
    virtual std::string getIconName() override;
};

#endif