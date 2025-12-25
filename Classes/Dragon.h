#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Soldier.h"

class Dragon : public Soldier
{
private:
    cocos2d::Sprite* shadow_sprite = nullptr;
public:
    CREATE_FUNC(Dragon);
    virtual bool init() override; // ÊÓ¾õÌØÐ§
    virtual void update(float dt) override;
    virtual SoldierStats getStats() override;
    virtual std::string getIconName() override;
};

#endif