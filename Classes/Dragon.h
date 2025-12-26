#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Soldier.h"

class Dragon : public Soldier
{
private:
    cocos2d::Sprite* shadow_sprite = nullptr;
    cocos2d::Sprite* visual_body = nullptr; //  ”æıÃÊ…Ì
public:
    virtual bool InitUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type) override;
    virtual void update(float dt) override;
    static Dragon* create();
    virtual std::string GetIconName() override;
    virtual cocos2d::Node* GetVisualNode() { return visual_body; }
    //virtual std::string GetAttackSound() override { return "Audio/dragon_fire.mp3"; }
};

#endif