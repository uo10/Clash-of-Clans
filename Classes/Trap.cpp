#include "Trap.h"

USING_NS_CC;

GameTrap* GameTrap::createTrap( Vec2 pos,Size tilesize) {
    GameTrap* trap = new GameTrap();
    if (trap && trap->init()) {
        trap->setPosition(pos);
        trap->_isAble = true; //以此状态标记陷阱已布设

        // 设置属性
            trap->_triggerRange = 30.0f; // 触发半径
            trap->_damageRange = 50.0f; // 爆炸半径
            trap->_damage = 20.0f;
            trap->_sprite = Sprite::create("map/trapBomb.png");
            Size spriteSize = trap->_sprite->getContentSize();
            trap->_sprite->setScaleX(tilesize.width / spriteSize.width);
            trap->_sprite->setScaleY(tilesize.height / spriteSize.height);
            trap->addChild(trap->_sprite);          
            // 陷阱没有触发时是隐藏状态
            trap->setVisible(false);                 

        trap->autorelease();
        return trap;
    }
    CC_SAFE_DELETE(trap);
    return nullptr;
}
void GameTrap::updateTrap(float dt, const Vector<GameUnit*>& enemies) {
    if (!_isAble) return; // 炸过了就不再检测

    for (auto enemy : enemies) {
        // 必须是活着的敌人
        if (enemy->isAlive()) {

            float dist = this->getPosition().distance(enemy->getPosition());

            // 只要有一个敌人走进触发范围
            if (dist <= _triggerRange) {
                this->activate(enemies); // 传入所有敌人是为了计算AOE爆炸
                break; // 触发一次就够了，跳出循环
            }
        }
    }
}
void GameTrap::activate(const Vector<GameUnit*>& enemies) {
    _isAble = false; // 标记为已失效

    //踩到了显示炸弹
    this->setVisible(true);

    // 让炸弹跳出来
    auto sequence = Sequence::create(
        ScaleTo::create(0.2f, 1.2f), // 弹起动画
        CallFunc::create([this, enemies]() {

            // 计算范围伤害
            // 遍历所有敌人，对爆炸范围内的敌人造成伤害
            for (auto enemy : enemies) {
                if (enemy->isAlive()) {
                    float dist = this->getPosition().distance(enemy->getPosition());
                    if (dist <= _damageRange) {
                        enemy->getDamage(_damage);
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