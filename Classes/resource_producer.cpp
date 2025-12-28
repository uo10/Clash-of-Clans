#include "resource_producer.h"
#include "player_data.h" 

USING_NS_CC;

ResourceProducer* ResourceProducer::Create(BuildingType type, int level) {
    ResourceProducer* pRet = new(std::nothrow) ResourceProducer();
    if (pRet && pRet->Init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ResourceProducer::Init(BuildingType type, int level) {

    // 1. 调用父类初始化
    if (!BaseBuilding::Init(type, level)) return false;

    // 2.子类获取从父类中的自己的特殊属性
    this->production_rate_ = stats_.production_rate;
    this->max_capacity_ = stats_.capacity;

    // 初始化当前资源
    this->current_res_ = 0.0f;
    this->is_bubble_showing_ = false;

    // 3. 创建气泡图标 (一开始隐藏)
    std::string icon_name = "Icon/Gold.png";
    if (this->type_ == BuildingType::kElixirPump) {
        icon_name = "Icon/Elixir.png"; 
    }
    bubble_icon_ = Sprite::create(icon_name);

    float center_x = 0;
    float top_y = 60; // 默认高度

    if (this->main_sprite_) {
        // 获取主图片的尺寸
        Size sprite_size = this->main_sprite_->getContentSize();
        float scale_y = this->main_sprite_->getScaleY();

        // 1. 计算 X 轴中心
        center_x = this->main_sprite_->getPositionX();

        // 2. 计算 Y 轴头顶
        top_y = this->main_sprite_->getPositionY() + (sprite_size.height * scale_y / 2);
    }

    // 设置位置
    bubble_icon_->setPosition(Vec2(center_x, top_y));
    bubble_icon_->setVisible(false);

    // 气泡浮动动画
    auto moveAction = RepeatForever::create(Sequence::create(
        MoveBy::create(0.8f, Vec2(0, 10)),
        MoveBy::create(0.8f, Vec2(0, -10)),
        nullptr
    ));
    bubble_icon_->runAction(moveAction);
    this->addChild(bubble_icon_,1000); // 层级在建筑的上面

    // 4. 开启 Update
    this->scheduleUpdate();

    // 5. 触摸监听器 用来收集操作
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = CC_CALLBACK_2(ResourceProducer::OnTouchBegan, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void ResourceProducer::update(float dt) {

    // 1. 先调用父类处理建造倒计时
    BaseBuilding::update(dt); 

    //  ================== 如果不是 IDLE，强制隐藏气泡并停止生产 =================
    if (this->state_ != BuildingState::kIdle) {
        // 如果当前气泡还存在，将其关掉
        if (is_bubble_showing_) {
            bubble_icon_->setVisible(false);
            is_bubble_showing_ = false;
        }
        // 停止生产，直接返回
        return;
    }

    // ======================== IDLE 状态下的正常生产逻辑 =========================

    // 2. 生产资源
    if (current_res_ < max_capacity_) {
        current_res_ += (production_rate_ / 3600.0f) * dt;
        if (current_res_ > max_capacity_) current_res_ = max_capacity_; //如果暂存量达到最大，不再进行增加
    }

    // 3. 气泡显示逻辑 
    bool should_show = (current_res_ >= 0.01 * max_capacity_);// 设定阈值1%

    if (should_show && !is_bubble_showing_) {
        // 如果达到显示阈值 进行展示气泡
        bubble_icon_->setVisible(true);
        is_bubble_showing_ = true;
    }
    else if (!should_show && is_bubble_showing_) {
        // 资源被收走了 隐藏气泡
        bubble_icon_->setVisible(false);
        is_bubble_showing_ = false;
    }
}

void ResourceProducer::CollectResource() {
    // 1. 获取可收集的数量
    int amount_to_collect = static_cast<int>(current_res_);
    if (amount_to_collect <= 0) return;

    int actual_added = 0;

    // 2. 根据建筑类型选择增加的数据类型
    if (this->type_ == BuildingType::kGoldMine) {
        actual_added = PlayerData::GetInstance()->AddGold(amount_to_collect);
    }
    else if (this->type_ == BuildingType::kElixirPump) {
        actual_added = PlayerData::GetInstance()->AddElixir(amount_to_collect);
    }

    // 如果仓库满了，剩下的资源还要保留在库存里
    current_res_ -= actual_added;

    // 3. UI 反馈
    if (actual_added > 0) {
        CCLOG("收集成功: %d, 剩余未收: %f", actual_added, current_res_);
        ShowFloatText(actual_added);
    }
    else {
        // 仓库满了，没有收入
        CCLOG("仓库已满！");
        ShowFloatText(0); 
    }
}

void ResourceProducer::ShowFloatText(int amount) {

    std::string text_str; // 显示的收集的数目
    std::string icon_path; // 图片的路径
    Color3B text_color;

    // 1. 初始化数据（图形和数量）
    if (amount > 0) { // 仓库未满
        text_str = "+" + std::to_string(amount);
        if (this->type_ == BuildingType::kGoldMine) {
            icon_path = "Icon/Gold.png"; 
            text_color = Color3B(255, 230, 100); // 金黄色
        }
        else {
            icon_path = "Icon/Elixir.png";
            text_color = Color3B(Color3B::MAGENTA); // 紫色
        }
    }
    else {
        text_str = "Full!";        // 仓库满了的情况
        text_color = Color3B::RED; // 显示红色的FULL情况
        icon_path = "";
    }

    // 2. 组合图标和文字
    auto effect_node = Node::create();
    effect_node->setPosition(Vec2(0.2, 80));    // 放在建筑头顶稍微高一点的位置
    effect_node->setScale(0.0f);    // 初始缩放为0，为了做弹出的效果
    this->addChild(effect_node, 2000); // 层级设高，盖过气泡

    // 创建图标 
    float total_width = 0;
    Sprite* icon = nullptr;
    if (!icon_path.empty()) {
        icon = Sprite::create(icon_path); // 根据路径创建图标
        if (icon) {
            // 限制图标大小，高度限制在 30 像素
            float scale = 30.0f / icon->getContentSize().height;
            icon->setScale(scale);
            icon->setAnchorPoint(Vec2(0, 0.5)); // 左中对齐
            effect_node->addChild(icon);

            // 累加宽度 (图标宽 + 间距)
            total_width += (icon->getContentSize().width * scale) + 5;
        }
    }

    // 3. 创建显示的收集文字
    auto label = Label::createWithSystemFont(text_str, "Arial", 32);
    label->setColor(text_color);
    label->enableOutline(Color4B::BLACK, 2); // 黑边增加清晰度
    label->setAnchorPoint(Vec2(0, 0.5)); // 左中对齐
    effect_node->addChild(label);

    total_width += label->getContentSize().width;


    float current_x = -total_width / 2.0f;    // 居中排版
    if (icon) {
        icon->setPosition(current_x, 0);
        current_x += (icon->getContentSize().width * icon->getScale()) + 5;
    }
    label->setPosition(current_x + 40, 0);

    // 4. 展示漂浮动画 
    // A: 弹出 (0.3秒内，从小变大，带弹性)
    auto appear_action = EaseBackOut::create(ScaleTo::create(0.3f, 1.2f));

    // B: 飘动并消失 (向上移动并变透明)
    auto float_action = Spawn::create(
        MoveBy::create(0.8f, Vec2(0, 80)), // 向上飘 80 像素
        FadeOut::create(0.8f),             // 同时变透明
        nullptr
    );

    effect_node->setCascadeOpacityEnabled(true);

    auto seq = Sequence::create(
        appear_action,
        float_action,
        RemoveSelf::create(), // 动画做完自我销毁
        nullptr
    );

    effect_node->runAction(seq); // 进行漂浮动画

    // 5. 播放音效
    if (amount > 0) {
        if (this->type_ == BuildingType::kGoldMine) {
            PlayerData::GetInstance()->PlayEffect("Audio/coins.mp3");
        }
        else {
            PlayerData::GetInstance()->PlayEffect("Audio/water.mp3");
        }
    }
    else {
         PlayerData::GetInstance()->PlayEffect("Audio/error.mp3");
    }
}

void ResourceProducer::UpdateSpecialProperties() {
    // 从父类已更新的 _stats 中获取新值
    this->production_rate_ = stats_.production_rate;
    this->max_capacity_ = stats_.capacity;
}

bool ResourceProducer::OnTouchBegan(Touch* touch, Event* event) {
    // 1. 获取点击位置 
    Vec2 p = this->convertTouchToNodeSpace(touch);

    // 2. 获取主图片的包围盒
    Rect rect;
    if (this->main_sprite_) {
        rect = this->main_sprite_->getBoundingBox();
    }
    // 3. 判断点击点是否在图片矩形内
    if (rect.containsPoint(p)) {
        // 如果处于可收集状态 (资源 >= 1)
        if (this->current_res_ >= 1.0f) {
            this->CollectResource();
        }
        // 返回 true  吞噬这次点击，不再向下传递
        return true;
    }

    // 点击在建筑外面，返回 false，让点击事件继续传递给地图或其他物体
    return false;
}