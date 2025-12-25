#include "Dragon.h"

USING_NS_CC;

bool Dragon::init()
{
    if (!Soldier::init()) return false;

    // 制作影子
    shadow_sprite = Sprite::create(getIconName());

    if (shadow_sprite) {
        // --- 样式设置 ---
        shadow_sprite->setColor(Color3B::BLACK);
        shadow_sprite->setOpacity(100); // 半透明

        // --- 变形设置 ---
        shadow_sprite->setScaleY(0.3f); // 压扁
        shadow_sprite->setSkewX(20.0f); // 倾斜

        // --- 位置设置 ---
        Size s = this->getContentSize();
        // 稍微偏移一点，看起来更有立体感
        shadow_sprite->setPosition(Vec2(s.width / 2 - 20, -100));
        shadow_sprite->setAnchorPoint(Vec2(0.5, 0));

        this->addChild(shadow_sprite, -1); // 放在身后
    }

    // 悬停动画
    // 让龙在那儿上下漂浮，增加“飞翔”的感觉
    float hoverDistance = 10.0f;
    float hoverDuration = 1.0f;

    auto moveUp = MoveBy::create(hoverDuration, Vec2(0, hoverDistance));
    auto moveDown = moveUp->reverse();

    auto hoverSeq = Sequence::create(moveUp, moveDown, nullptr);
    this->runAction(RepeatForever::create(hoverSeq));

    return true;
}

// 每帧同步影子
void Dragon::update(float dt)
{
    Soldier::update(dt);
    // 同步影子状态
    if (shadow_sprite) {
        if (this->getSpriteFrame()) {
            shadow_sprite->setSpriteFrame(this->getSpriteFrame());
        }
        shadow_sprite->setFlippedX(this->isFlippedX()); // 同步朝向
    }
}

Soldier::SoldierStats Dragon::getStats() {
    // 血量: 80 (脆皮)
    // 伤害: 8
    // 移速: 90 (稍快)
    // 射程: 200 (远程，可以隔着墙打建筑)
    // 攻速: 1.0秒/次
    return { 80, 8, 90.0f, 200.0f, 1.0f };
}

std::string Dragon::getIconName() {
    return "Dragon.png";
}