#include "ResourceProducer.h"
#include "PlayerData.h" 

USING_NS_CC;

ResourceProducer* ResourceProducer::create(BuildingType type, int level) {
    ResourceProducer* pRet = new(std::nothrow) ResourceProducer();
    if (pRet && pRet->init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ResourceProducer::init(BuildingType type, int level) {
    // 1. 调用父类初始化
    if (!BaseBuilding::init(type, level)) return false;

    //2.子类获取从父类中的自己的特殊属性
    this->productionRate = _stats.productionRate;
    this->maxCapacity = _stats.capacity;

    // 初始化当前资源
    this->currentRes = 0.0f;
    this->isBubbleShowing = false;

    // 3. 创建气泡图标 (一开始隐藏)
    std::string iconName = "Gold.png";
    if (this->type == BuildingType::ELIXIR_PUMP) {
        iconName = "Elixir.png"; 
    }
    bubbleIcon = Sprite::create(iconName);

    float centerX = 0;
    float topY = 60; // 默认高度

    if (this->mainSprite) {
        // 获取主图片的尺寸
        Size spriteSize = this->mainSprite->getContentSize();
        float scaleY = this->mainSprite->getScaleY();

        // 1. 计算 X 轴中心
        centerX = this->mainSprite->getPositionX();

        // 2. 计算 Y 轴头顶
        topY = this->mainSprite->getPositionY() + (spriteSize.height * scaleY / 2);
    }

    // 设置位置
    bubbleIcon->setPosition(Vec2(centerX, topY));
    bubbleIcon->setVisible(false);

    // 气泡浮动动画
    auto moveAction = RepeatForever::create(Sequence::create(
        MoveBy::create(0.8f, Vec2(0, 10)),
        MoveBy::create(0.8f, Vec2(0, -10)),
        nullptr
    ));
    bubbleIcon->runAction(moveAction);
    this->addChild(bubbleIcon,999); // 层级要高

    // 4. 开启 Update
    this->scheduleUpdate();

    // 5. 注册触摸监听器
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = CC_CALLBACK_2(ResourceProducer::onTouchBegan, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void ResourceProducer::update(float dt) {
    // 只有在 IDLE (正常工作) 状态下才生产
    // 如果正在升级(BUILDING)或者被毁(DESTROYED)，不应该生产
    if (this->state != BuildingState::IDLE) return;

    if (currentRes < maxCapacity) {
        currentRes += (productionRate / 3600.0f) * dt;// Rate 是每秒产量

        if (currentRes > maxCapacity) currentRes = maxCapacity;
    }

    // 气泡逻辑：满 20% 就显示
    bool shouldShow = (currentRes >= 1.0f);

    if (shouldShow && !isBubbleShowing) {
        bubbleIcon->setVisible(true);
        isBubbleShowing = true;
    }
    else if (!shouldShow && isBubbleShowing) {
        // 如果资源被收光了，或者被偷了，隐藏气泡
        bubbleIcon->setVisible(false);
        isBubbleShowing = false;
    }
}

void ResourceProducer::collectResource() {
    int amountToCollect = (int)currentRes;
    if (amountToCollect <= 0) return;

    int actualAdded = 0;

    //根据建筑类型选择增加的数据类型
    if (this->type == BuildingType::GOLD_MINE) {
        actualAdded = PlayerData::getInstance()->addGold(amountToCollect);
    }
    else if (this->type == BuildingType::ELIXIR_PUMP) {
        actualAdded = PlayerData::getInstance()->addElixir(amountToCollect);
    }

    // 如果仓库满了，剩下的资源还要保留在金矿里
    currentRes -= actualAdded;

    // 3. UI 反馈
    if (actualAdded > 0) {
        CCLOG("收集成功: %d, 剩余未收: %f", actualAdded, currentRes);
        showFloatText(actualAdded);

        // 可以添加音效：
        // SimpleAudioEngine::getInstance()->playEffect("collect.wav");
    }
    else {
        // 仓库满了，没有收入
        CCLOG("仓库已满！");
        showFloatText(0); 
    }
}

void ResourceProducer::showFloatText(int amount) {
    std::string text = (amount > 0) ? "+" + std::to_string(amount) : "Full!";

    // 根据资源类型设置颜色
    Color3B color = (this->type == BuildingType::GOLD_MINE) ? Color3B::YELLOW : Color3B::MAGENTA;
    if (amount == 0) color = Color3B::RED;

    auto label = Label::createWithSystemFont(text, "Arial", 28);
    label->setColor(color);
    label->enableOutline(Color4B::BLACK, 2);

    label->setPosition(Vec2(0, 80));
    this->addChild(label, 200);

    auto move = MoveBy::create(0.8f, Vec2(0, 60));
    auto fade = FadeOut::create(0.8f);
    auto seq = Sequence::create(Spawn::create(move, fade), RemoveSelf::create(), nullptr);
    label->runAction(seq);
}

void ResourceProducer::updateSpecialProperties() {
    // 从父类已更新的 _stats 中获取新值
    this->productionRate = _stats.productionRate;
    this->maxCapacity = _stats.capacity;
}

bool ResourceProducer::onTouchBegan(Touch* touch, Event* event) {
    // 1. 获取点击位置 (转换为节点内的相对坐标)
    Vec2 p = this->convertTouchToNodeSpace(touch);

    // 2. 获取主图片的包围盒
    Rect rect;
    if (this->mainSprite) {
        rect = this->mainSprite->getBoundingBox();
    }
    // 3. 判断点击点是否在图片矩形内
    if (rect.containsPoint(p)) {
        // 如果处于可收集状态 (资源 >= 1)
        if (this->currentRes >= 1.0f) {
            this->collectResource();
        }
        // 返回 true 表示吞噬这次点击，不再向下传递
        return true;
    }

    // 点击在建筑外面，返回 false，让点击事件继续传递给地图或其他物体
    return false;
}