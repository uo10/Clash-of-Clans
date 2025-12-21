#include "LoadingScene.h"
#include "MainVillage.h" 

USING_NS_CC;
using namespace cocos2d::ui;

Scene* LoadingScene::createScene() {
    return LoadingScene::create();
}

bool LoadingScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 背景图
    auto bg = Sprite::create("Loading_BG.png");
    if (!bg) {
        // 没图就用黑色背景
        auto layer = LayerColor::create(Color4B::BLACK);
        this->addChild(layer);
    }
    else {
        bg->setPosition(center);
        // 适配拉伸 填满屏幕 
        bg->setScaleX(visibleSize.width / bg->getContentSize().width);
        bg->setScaleY(visibleSize.height / bg->getContentSize().height);
        this->addChild(bg);
    }

    // 2. 游戏 Logo 
    auto labelLogo = Label::createWithSystemFont("Clash of Clans", "fonts/GROBOLD.ttf", 80);
    labelLogo->enableOutline(Color4B::BLACK, 3);
    labelLogo->setPosition(center + Vec2(0, 500));
    this->addChild(labelLogo);

    // 3. 创建进度条背景 (槽)
    auto barBg = Sprite::create();
    barBg->setTextureRect(Rect(0, 0, 600, 40));
    barBg->setColor(Color3B(50, 50, 50)); // 深灰色底
    barBg->setPosition(center - Vec2(0, 500));
    this->addChild(barBg);

    // 4. 创建 LoadingBar
    _loadingBar = LoadingBar::create("Loading_Bar.png");

    //  开启九宫格模式 (Scale9Enabled)
    _loadingBar->setScale9Enabled(true);    // 将LoadingBar放缩为Loading_BG大小

    _loadingBar->setContentSize(Size(600, 40));    // 强制设为和背景一样大 (600 x 40)

    _loadingBar->setDirection(LoadingBar::Direction::LEFT);    // 设置方向和进度
    _loadingBar->setPercent(0);

    _loadingBar->setPosition(center - Vec2(0, 500));    // 设置位置 (和 barBg 一致)
    this->addChild(_loadingBar);


    // 5. 百分比文字
    _percentLabel = Label::createWithSystemFont("0%", "Kenney Future Narrow", 24);
    _percentLabel->setColor(Color3B::MAGENTA);
    _percentLabel->setPosition(center - Vec2(0, 500));
    this->addChild(_percentLabel);

    // 6. 开启 Update
    PlayerData::getInstance()->playEffect("Audio/Intro.mp3");
    this->scheduleUpdate();

    return true;
}

void LoadingScene::update(float dt) {

    // 模拟加载速度：每秒增加 20% (即5秒加载完)
    _currentPercent += dt * 20.0f;

    if (_currentPercent > 100.0f) {
        _currentPercent = 100.0f;

        // 加载完成，切换场景
        this->unscheduleUpdate(); // 停止更新

        // 切换到 MainVillage
        auto scene = MainVillage::createScene();
        // 淡入淡出效果
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
    }

    // 更新 UI
    _loadingBar->setPercent(_currentPercent);  //更新进度条
    _percentLabel->setString(StringUtils::format("%d%%", (int)_currentPercent));  // 更新进度条文字
}