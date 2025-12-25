#include "loading_scene.h"
#include "main_village.h" 

USING_NS_CC;
using namespace cocos2d::ui;

Scene* LoadingScene::createScene() {
    return LoadingScene::create();
}

bool LoadingScene::init() {
    if (!Scene::init()) return false;

    auto visible_size = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visible_size.width / 2, visible_size.height / 2);

    // 1. 背景图 
    auto bg = Sprite::create("LoadingScene/Loading_BG.png");
    if (!bg) {
        auto layer = LayerColor::create(Color4B::BLACK);
        this->addChild(layer);
    }
    else {
        bg->setPosition(center);
        bg->setScaleX(visible_size.width / bg->getContentSize().width);
        bg->setScaleY(visible_size.height / bg->getContentSize().height);
        this->addChild(bg);
    }

    // 2. 游戏 Logo
    auto label_logo = Label::createWithTTF("Clash of Clans", "fonts/GROBOLD.ttf", 100);
    label_logo->setColor(Color3B(255, 220, 100)); // 金色字体
    label_logo->enableOutline(Color4B::BLACK, 6); // 黑色描边
    label_logo->enableShadow(Color4B(0, 0, 0, 150), Size(5, -5), 0); // 黑色阴影
    label_logo->setPosition(Vec2(center.x, visible_size.height * 0.80f));
    this->addChild(label_logo);

    auto scale_up = ScaleTo::create(1.5f, 1.05f); // 放大
    auto scale_down = ScaleTo::create(1.5f, 1.0f); // 缩小
    auto seq = Sequence::create(scale_up, scale_down, nullptr);
    label_logo->runAction(RepeatForever::create(seq));

    // 3. 进度条组件 
    auto loading_node = Node::create();

    // 父节点定位
    loading_node->setPosition(Vec2(center.x, visible_size.height * 0.15f));

    this->addChild(loading_node);

    // --- 外框 ---
    auto bar_bg = Sprite::create();
    bar_bg->setTextureRect(Rect(0, 0, 604, 44));
    bar_bg->setColor(Color3B(30, 30, 30));
    bar_bg->setPosition(0, 0); // 居中
    loading_node->addChild(bar_bg);

    // --- 内槽 ---
    auto bar_inner = Sprite::create();
    bar_inner->setTextureRect(Rect(0, 0, 600, 40));
    bar_inner->setColor(Color3B(60, 40, 20));
    bar_inner->setPosition(0, 0);

    loading_node->addChild(bar_inner);

    // 4. 创建 LoadingBar
    loading_bar_ = LoadingBar::create("LoadingScene/Loading_Bar.png");

    if (loading_bar_->getContentSize().width <= 0) {
        auto s = Sprite::create();
        s->setTextureRect(Rect(0, 0, 600, 40));
        loading_bar_->loadTexture(s->getTexture()->getPath());
    }
    loading_bar_->setScale9Enabled(true);
    loading_bar_->setContentSize(Size(600, 40));
    loading_bar_->setDirection(LoadingBar::Direction::LEFT);
    loading_bar_->setPercent(0);
    loading_bar_->setColor(Color3B(255, 50, 255));
    loading_bar_->setPosition(Vec2(0, 0)); // 以前是 -350

    loading_node->addChild(loading_bar_);

    // 5. 百分比文字
    percent_label_ = Label::createWithTTF("0%", "fonts/GROBOLD.ttf", 20);
    percent_label_->setColor(Color3B::WHITE);
    percent_label_->enableOutline(Color4B::BLACK, 1);
    percent_label_->setPosition(Vec2(0, 0)); 

    loading_node->addChild(percent_label_, 10);

   // 6. 进度条上方的提示文字
    auto label_info = Label::createWithTTF("Loading...", "fonts/GROBOLD.ttf", 28);

    // 样式设置：白色字 + 黑色描边
    label_info->setColor(Color3B::WHITE);
    label_info->enableOutline(Color4B::BLACK, 2);
    label_info->enableShadow(Color4B(0, 0, 0, 100), Size(2, -2), 0);

    // 位置设置
    label_info->setPosition(Vec2(0, 45));

    loading_node->addChild(label_info);

    // 7. 开启 Update
    this->scheduleUpdate();

    return true;
}

void LoadingScene::update(float dt) {

    // 模拟加载速度：每秒增加 25% (即4秒加载完)
    current_percent_ += dt * 25.0f;

    if (current_percent_ > 100.0f) {
        current_percent_ = 100.0f;

        // 加载完成，切换场景
        this->unscheduleUpdate(); // 停止更新

        // 切换到 MainVillage
        auto scene = MainVillage::createScene();
        // 淡入淡出效果
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }

    // 更新 UI
    loading_bar_->setPercent(current_percent_);  //更新进度条
    percent_label_->setString(StringUtils::format("%d%%", static_cast<int>(current_percent_)));  // 更新进度条文字
}