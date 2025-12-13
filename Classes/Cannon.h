#ifndef __CANNON_H__
#define __CANNON_H__

#include "BaseBuilding.h"
// 前向声明 Soldier，因为我们需要攻击它
class Soldier;

class Cannon : public BaseBuilding
{
public:
    // 修改为接受等级参数
    static Cannon* create(int level);

    // 初始化函数
    virtual bool init(int level);

    // 核心循环：用于寻找敌人并攻击
    virtual void update(float dt) override;

    // 更新特有属性 (升级时伤害增加)
    virtual void updateSpecialProperties() override;

private:
    float _attackRange;    // 攻击范围
    float _damage;         // 攻击伤害
    float _attackInterval; // 攻击间隔
    float _attackTimer;    // 计时器

    Soldier* _targetSoldier; // 当前锁定的敌人

    void findEnemy();      // 寻找最近的敌人
    void fireAtEnemy();    // 开火逻辑
};

#endif