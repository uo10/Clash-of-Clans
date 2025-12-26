#include "Dragon.h"

USING_NS_CC;

Dragon* Dragon::create() {
    Dragon* pRet = new(std::nothrow) Dragon();

    // 调用 GameUnit 的初始化函数
    // 填入数值: 图片, HP, 速度, 伤害, 射程, 类型
    if (pRet && pRet->InitUnit("Soldier/Dragon.png", 500, 120.0f, 15.0f, 60.0f, UnitType::kSoldier)) {

        pRet->SetUnitName("Dragon"); // 设置名字
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Dragon::InitUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type)
{
    if (!GameUnit::InitUnit(filename, maxHp, speed, damage, range, type)) return false;
    
    this->setOpacity(0);
    visual_body = Sprite::create(GetIconName());
    if (visual_body) {
        Size s = this->getContentSize();
        // 放在中心
        visual_body->setPosition(Vec2(s.width / 2, s.height / 2));

        //悬浮动画
        float hover_distance = 10.0f;
        float hover_duration = 1.0f;
        auto move_up = MoveBy::create(hover_duration, Vec2(0, hover_distance));
        auto move_down = move_up->reverse();
        auto hover_seq = Sequence::create(move_up, move_down, nullptr);

        visual_body->runAction(RepeatForever::create(hover_seq));

        // 添加为子节点，Z=1 保证盖在影子上面
        this->addChild(visual_body, 1);
    }

    // 创建影子 (贴在本体 this 上，而不是 visual_body 上)
    shadow_sprite = Sprite::create(GetIconName());
    if (shadow_sprite) {
        shadow_sprite->setColor(Color3B::BLACK);
        shadow_sprite->setOpacity(100);
        shadow_sprite->setScaleY(0.3f);
        shadow_sprite->setSkewX(20.0f);

        Size s = this->getContentSize();
        // 影子位置固定在脚下
        shadow_sprite->setPosition(Vec2(s.width / 2 - 20, -100));
        shadow_sprite->setAnchorPoint(Vec2(0.5, 0));

        this->addChild(shadow_sprite, -1); // 在最底层
    }

    return true;
}

// 每帧同步影子
void Dragon::update(float dt)
{
    Soldier::update(dt);
    if (visual_body) {
        visual_body->setFlippedX(this->isFlippedX());
        // 如果有帧动画，也要同步
        if (this->getSpriteFrame()) {
            
        }
    }
    // 同步影子状态
    if (shadow_sprite) {
        shadow_sprite->setFlippedX(this->isFlippedX());
    }
}


std::string Dragon::GetIconName() {
    return "Dragon.png";
}