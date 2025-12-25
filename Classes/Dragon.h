#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Soldier.h"

class Dragon : public Soldier
{
private:
    cocos2d::Sprite* shadow_sprite = nullptr;
public:
    virtual bool InitUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type) override;
    virtual void update(float dt) override;
    static Dragon* create();
    virtual std::string getIconName() override;
};

#endif