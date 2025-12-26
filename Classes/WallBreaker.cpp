#include "WallBreaker.h"

USING_NS_CC;

WallBreaker* WallBreaker::create() {
    WallBreaker* pRet = new(std::nothrow) WallBreaker();

    // 调用 GameUnit 的初始化函数
    // 填入数值: 图片, HP, 速度, 伤害, 射程, 类型
    if (pRet && pRet->InitUnit("Soldier/Wall_Breaker.png", 20, 100.0f, 7.0f, 80.0f, UnitType::kSoldier)) {

        pRet->SetUnitName("WallBreaker"); // 设置名字
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

std::string WallBreaker::GetIconName() {
    return "Wall_Breaker.png";
}

// 重写攻击逻辑：自爆
void WallBreaker::UpdateUnit(float dt) {
    if (!target_ || !target_->IsAlive()) return;
    attack_speed_ += dt;
    if (attack_speed_ >= 0.3f) {
        attack_speed_ = 0; 

        // 目标是墙
        if (target_->GetType() == UnitType::kWall || target_->GetUnitName() == "Fence") {
            //PlayerData::GetInstance()->PlayEffect("Audio/explode.mp3");

            // 造成巨额伤害
            float final_damage = damage_ * 40.0f;
            target_->GetDamage(final_damage);

            CCLOG("WallBreaker DETONATED on Wall! Damage: %.0f", final_damage);
            this->GetDamage(999999.0f);
        }
        // 没墙了
        else {
            PlayerData::GetInstance()->PlayEffect("Audio/punch.mp3");
            target_->GetDamage(damage_);
            auto scale_up = ScaleBy::create(0.1f, 1.2f);
            auto scale_down = scale_up->reverse();
            this->runAction(Sequence::create(scale_up, scale_down, nullptr));

            CCLOG("WallBreaker punched building (No Suicide).");
        }
    }
}