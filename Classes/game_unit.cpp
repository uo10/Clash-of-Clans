#include "game_unit.h"
USING_NS_CC;

//创建对象
GameUnit* GameUnit::Create(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type)
{
    GameUnit* p_ret = new(std::nothrow) GameUnit();
    if (p_ret && p_ret->InitUnit(filename, maxHp, speed, damage, range, type)) {
        p_ret->autorelease();
        return p_ret;
    }
    delete p_ret;
    return nullptr;
}

//初始化对象
bool GameUnit::InitUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type) {
    if (!this->initWithFile(filename)) return false;

    // 初始化属性
    max_hp_ = maxHp;
    current_hp_ = maxHp;
    speed_ = speed;
    damage_ = damage;
    attack_range_ = range;
    type_ = type;

    attack_speed_ = 0.0f;
    target_ = nullptr;

    //创建血条
    // 添加背景
    auto bar_bg = Sprite::create("map/bar_bg.png");
    Size bar_bg_size = bar_bg->getContentSize();
    Size sprite_size = this->getContentSize();
    bar_bg->setScaleX(sprite_size.width / bar_bg_size.width);
    bar_bg->setScaleY(sprite_size.width / bar_bg_size.width);
    bar_bg->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 10));//适应尺寸
    this->addChild(bar_bg);

    //设置前景
    hp_bar_ = cocos2d::ui::LoadingBar::create("map/bar_red.png");//便于做百分比血量
	hp_bar_->setDirection(cocos2d::ui::LoadingBar::Direction::LEFT);//血条从左向右减少
    hp_bar_->setPercent(100);//初始满血
    Size bar_size = hp_bar_->getContentSize();
    hp_bar_->setScaleX(sprite_size.width / bar_size.width);
    hp_bar_->setScaleY(sprite_size.width / bar_size.width);
    hp_bar_->setPosition(bar_bg->getPosition());
    this->addChild(hp_bar_);

    return true;
}

//受伤扣血
void GameUnit::GetDamage(float damage) {
    if (!IsAlive()) return;

    current_hp_ -= damage;
	UpdateHpBar();//更新血条状态

    if (current_hp_ <= 0) {
        // 死亡逻辑
        this->runAction(Sequence::create(
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}
//更新血条状态
void GameUnit::UpdateHpBar() {
    if (hp_bar_) hp_bar_->setPercent((current_hp_ / max_hp_) * 100.0f);
}
//按帧更新对象状态
void GameUnit::UpdateUnit(float dt) {
    if (!target_ || !target_->IsAlive()) return;
    attack_speed_ += dt;
    if (attack_speed_ >= 1.0f) { // 1秒攻击一次
        attack_speed_ = 0;
        //炸弹兵特判，对围墙超强伤害
        if (target_->GetUnitName() == "Fence" && unit_name_=="WallBreaker") {
            target_->GetDamage(damage_*40);
        }
        else {
            target_->GetDamage(damage_);
        }
        CCLOG("%s attacked %s", unit_name_.c_str(), target_->GetUnitName().c_str());
    }
}
//设置路径
void GameUnit::SetPath(const std::vector<Vec2>& path) {
    path_ = path;
    path_index_ = 0;
    if (!path_.empty()) path_index_ = 1; // 跳过起点
}
//获取下一步落脚点
Vec2 GameUnit::GetNextStep() {
    if (path_.empty() || path_index_ >= path_.size()) return getPosition();
    return path_[path_index_];
}