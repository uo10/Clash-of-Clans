#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "base_building.h"

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
    virtual BuildingType getPreferredTargetType() { return BuildingType::kNone; }

    //设置是否为家乡模式（只游走不攻击）
    void setHomeMode(bool isHome);
    bool getIsHomeMode() const { return isHomeMode; }

    void setHomePosition(cocos2d::Vec2 pos) { _homePosition = pos; }

protected:
    // 成员变量
    SoldierStats stats;
    float currentHP;
    float attackTimer;
    State state;

    BaseBuilding* target; // 当前目标
    cocos2d::DrawNode* hpBar;

    cocos2d::Vec2 _homePosition; // 记录兵营位置

    bool isValidTarget(BaseBuilding* building, BuildingType preference);

    // 行为
    virtual void findTarget();
    void moveTowardTarget(float dt);
    virtual void attackTarget(float dt);
    void updateHPBar();
    void die();

    bool isHomeMode = false;    // 开关
    float wanderTimer = 0.0f;   // 计时器
    float wanderWaitTime = 0.0f;// 随机的发呆时间
    cocos2d::Vec2 wanderTarget; // 随机的移动目标点

    // 游走逻辑
    void updateWander(float dt);
    void pickNewWanderTarget();

};

#endif