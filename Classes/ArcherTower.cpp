#include "ArcherTower.h"
#include "Soldier.h" // 引用 Soldier 头文件以便识别

USING_NS_CC;

ArcherTower* ArcherTower::create(int level)
{
    ArcherTower* pRet = new (std::nothrow) ArcherTower();
    // 强制类型为 ArcherTower
    if (pRet && pRet->init(level))
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ArcherTower::init(int level)
{
    // 1. 调用父类 BaseBuilding 初始化 (它会处理图片、血条、状态机)
    if (!BaseBuilding::Init(BuildingType::kArcherTower, level)) return false;

    // 2. 初始化战斗属性
    this->_attackTimer = 0;
    this->_targetSoldier = nullptr;

    // 根据等级初始化伤害 (可以在这里写，也可以在 updateSpecialProperties 里写)
    UpdateSpecialProperties();

    // 3. 开启 update 寻找敌人
    this->scheduleUpdate();

    return true;
}

void ArcherTower::UpdateSpecialProperties()
{
    // 这里处理父类 BuildingStats 里没有的“伤害”和“射程”属性
    if (this->level_ == 1) {
        _damage = 20.0f;
        _attackRange = 150.0f;
        _attackInterval = 1.0f;
    }
    else if (this->level_ == 2) {
        _damage = 35.0f;
        _attackRange = 160.0f;
        _attackInterval = 0.9f;
    }
    else {
        _damage = 50.0f;
        _attackRange = 170.0f;
        _attackInterval = 0.8f;
    }
    CCLOG("ArcherTower Updated: Dmg=%.0f, Range=%.0f", _damage, _attackRange);
}

void ArcherTower::update(float dt)
{
    BaseBuilding::update(dt); // 先调用父类刷新时间条
    // 只有在 IDLE 或 ATTACKING 状态才工作
    // 如果正在建造(BUILDING)或被毁(DESTROYED)，不能攻击
    if (this->state_ != BuildingState::kIdle && this->state_ != BuildingState::kAttacking) return;

    // 1. 校验目标有效性
    if (_targetSoldier) {
        // 1. 检查士兵状态是否为 DEAD (这是最安全的做法)
        // 只要士兵宣判死亡，塔就应该立刻停手，哪怕士兵尸体还在播动画
        if (_targetSoldier->getState() == Soldier::State::DEAD) {
            _targetSoldier = nullptr;
            this->state_ = BuildingState::kIdle;
        }
        // 2. 检查指针指向的对象是否还在场景树上 (防止野指针)
        else if (!_targetSoldier->getParent()) {
            _targetSoldier = nullptr;
            this->state_ = BuildingState::kIdle;
        }
    }

    // 2. 如果没有目标，寻找最近的
    if (!_targetSoldier) {
        findEnemy();
    }

    // 3. 如果有目标，攻击
    if (_targetSoldier) {
        this->state_ = BuildingState::kAttacking; // 切换状态(虽然BaseBuilding目前对Attack没特殊表现，但这符合逻辑)

        _attackTimer += dt;
        if (_attackTimer >= _attackInterval) {
            _attackTimer = 0;
            fireAtEnemy();
        }
    }
}

void ArcherTower::findEnemy()
{
    Node* map = this->getParent();
    if (!map) return;

    Soldier* closest = nullptr;
    float minDst = 999999.0f;
    Vec2 myPos = this->getPosition();

    // 遍历地图寻找 Soldier
    for (auto child : map->getChildren()) {
        // 假设 Soldier 没有特定 Tag，我们用 dynamic_cast 识别
        // 或者你可以给 Soldier 设置一个特定 Tag (比如 2000)
        Soldier* s = dynamic_cast<Soldier*>(child);

        if (s) {
            if (s->getState() == Soldier::State::DEAD) {
                continue; // 跳过这个死人
            }

            if (s->getIsHomeMode()) {
                continue; // 跳过自己人
            }

            float dist = myPos.distance(s->getPosition());
            if (dist <= _attackRange && dist < minDst) {
                minDst = dist;
                closest = s;
            }
        }
    }

    if (closest) {
        _targetSoldier = closest;
        // CCLOG("ArcherTower found target!");
    }
}

void ArcherTower::fireAtEnemy()
{
    if (!_targetSoldier) return;

    // 1. 炮口特效 (可选)
    // auto boom = ParticleExplosion::create(); ...

    // 2. 发射炮弹动画 (简易版：直接扣血)

    // 这里直接造成伤害
    _targetSoldier->takeDamage(this->_damage); // 假设伤害变量叫 _damage 或 damage

    // 模拟一下：
    CCLOG("Boom! ArcherTower hit soldier for %.0f damage", _damage);

    // 简单的缩放反馈
    this->main_sprite_->runAction(Sequence::create(
        ScaleTo::create(0.05f, 1.1f),
        ScaleTo::create(0.05f, 1.0f),
        nullptr
    ));
}