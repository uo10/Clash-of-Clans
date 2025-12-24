#include "splash_scene.h"
#include "loading_scene.h" 
#include "MainVillage.h" 

USING_NS_CC;

Scene* SplashScene::CreateScene() {
    return SplashScene::create();
}

bool SplashScene::init() {
    if (!Scene::init()) return false;

    PlayerData::GetInstance()->PlayEffect("Audio/Intro.mp3");
    auto visible_size = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visible_size.width / 2, visible_size.height / 2);

    // 1. 设置黑色底色遮蔽层
    auto bg_layer = LayerColor::create(Color4B::BLACK);
    this->addChild(bg_layer);

    // 2. 创建启动图
    std::string img_path = "LoadingScene/Splash_Image.png"; 
    auto sprite = Sprite::create(img_path);

    if (sprite) {
        sprite->setPosition(center);

        // a. 填充适配屏幕 
        float scale_x = visible_size.width / sprite->getContentSize().width;
        float scale_y = visible_size.height / sprite->getContentSize().height;
        float final_scale = std::max(scale_x, scale_y);

        sprite->setScale(final_scale);

        // b. 初始设置不可见 
        sprite->setOpacity(0);

        this->addChild(sprite);

        // 3. 执行动画序列：淡入 -> 停留 -> 跳转
        auto seq = Sequence::create(
            FadeIn::create(1.0f),       // 1.0秒内慢慢显示出来 (淡入)
            DelayTime::create(0.8f),    // 停留 0.8秒 让玩家看清楚
            CallFunc::create([=]() {
                this->GoToLoadingScene(0); // 动画做完后跳转
                }),
            nullptr
        );

        sprite->runAction(seq);
    }
    else {
        // 如果图片没找到，直接跳转，防止卡死
        CCLOG("Error: Splash image not found!");
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(SplashScene::GoToLoadingScene), 0.1f);
    }

    return true;
}

void SplashScene::GoToLoadingScene(float dt) {
    auto scene = LoadingScene::createScene();

    // TransitionFade负责“淡出”当前场景，进入 Loading
    Director::getInstance()->replaceScene(TransitionFade::create(0.8f, scene, Color3B::BLACK));
    // "淡入 -> 停留 -> 淡出" 
}