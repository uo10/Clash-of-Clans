#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "BaseBuilding.h"

class Soldier : public cocos2d::Sprite
{
public:
    enum class State {
        IDLE,       // 寻找目标
        MOVE,       // 移动中
        ATTACK,     // 攻击中
        DEAD        // 死亡
    };

    // 士兵属性结构体
    struct SoldierStats {
        int maxHP;
        int damage;
        float moveSpeed;
        float attackRange;
        float attackSpeed; // 攻击间隔
    };

    virtual bool init() override;
    virtual void update(float dt) override;
    void takeDamage(float dmg);
    State getState() const { return state; }


    // 1. 获取该兵种的特有属性
    virtual SoldierStats getStats() = 0;

    // 2. 获取该兵种的图片文件名
    virtual std::string getIconName() = 0;

    // 3. 获取优先攻击目标类型 (默认返回 NONE，表示攻击最近的一切)
    virtual BuildingType getPreferredTargetType() { return BuildingType::NONE; }

protected:
    // 成员变量
    SoldierStats stats;
    float currentHP;
    float attackTimer;
    State state;

    BaseBuilding* target; // 当前目标
    cocos2d::DrawNode* hpBar;

    bool isValidTarget(BaseBuilding* building, BuildingType preference);

    // 行为
    virtual void findTarget();
    void moveTowardTarget(float dt);
    virtual void attackTarget(float dt);
    void updateHPBar();
    void die();
};

#endif