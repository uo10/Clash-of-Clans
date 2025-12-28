#include "trap.h"

USING_NS_CC;

GameTrap* GameTrap::CreateTrap( Vec2 pos,Size tilesize) {
    GameTrap* trap = new GameTrap();
    if (trap && trap->init()) {
        trap->setPosition(pos);
        trap->is_able_ = true; //以此状态标记陷阱已布设

        // 设置属性
            trap->trigger_range_ = 30.0f; // 触发半径
            trap->damage_range_ = 50.0f; // 爆炸半径
            trap->damage_ = 20.0f;
            trap->sprite_ = Sprite::create("map/trapBomb.png");
            Size sprite_size = trap->sprite_->getContentSize();
            trap->sprite_->setScaleX(tilesize.width / sprite_size.width);
            trap->sprite_->setScaleY(tilesize.height / sprite_size.height);
            trap->addChild(trap->sprite_);          
            // 陷阱没有触发时是隐藏状态
            trap->setVisible(false);                 

        trap->autorelease();
        return trap;
    }
	CC_SAFE_DELETE(trap);// 删除对象
    return nullptr;
}
void GameTrap::UpdateTrap(float dt, const Vector<GameUnit*>& enemies) {
    if (!is_able_) return; // 炸过了就不再检测

    for (auto enemy : enemies) {
        // 必须是活着的敌人
        if (enemy->IsAlive()) {

            float dist = this->getPosition().distance(enemy->getPosition());

            // 只要有一个敌人走进触发范围
            if (dist <= trigger_range_) {
                this->Activate(enemies); // 传入所有敌人是为了计算范围爆炸
                break; // 触发一次即可，跳出循环
            }
        }
    }
}
void GameTrap::Activate(const Vector<GameUnit*>& enemies) {
    is_able_ = false; // 标记为已失效

    //踩到了显示炸弹
    this->setVisible(true);

    // 让炸弹跳出来
    auto sequence = Sequence::create(
        ScaleTo::create(0.2f, 1.2f), // 弹起动画
        CallFunc::create([this, enemies]() {

            // 计算范围伤害
            // 遍历所有敌人，对爆炸范围内的敌人造成伤害
            for (auto enemy : enemies) {
                if (enemy->IsAlive()) {
                    float dist = this->getPosition().distance(enemy->getPosition());
                    if (dist <= damage_range_) {
                        enemy->GetDamage(damage_);
                    }
                }
            }         
            // 销毁陷阱
            this->removeFromParent();
            }),
        nullptr
    );

    this->runAction(sequence);
}
