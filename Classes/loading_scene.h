#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" 

/**
 * @brief 加载场景 (LoadingScene)
 * 游戏启动后的第一个场景。
 * 主要功能：
 * 1. 显示游戏Logo和背景。
 * 2. 显示一个进度条和百分比文字。
 * 3. 通过 update 函数模拟加载过程。
 * 4. 加载完成后自动跳转到主场景 (MainVillage)。
 */
class LoadingScene : public cocos2d::Scene {
public:
    /**
     * @brief 创建场景的静态方法
     * 标准 Cocos2d-x 写法，用于创建并返回一个由 autorelease 管理的 Scene 对象。
     */
    static cocos2d::Scene* createScene();

    /**
     * @brief 初始化函数
     * 在这里进行 UI 的创建和布局（背景图、进度条、文字标签等）。
     * 并开启 scheduleUpdate() 以启动帧循环。
     * @return bool 初始化成功返回 true
     */
    virtual bool init();

    /**
     * @brief 帧更新函数
     * 系统每一帧都会自动调用此函数。
     * 在这里处理进度数值的增加逻辑 (_currentPercent++)。
     * 当进度达到 100% 时，执行场景切换逻辑。
     * @param dt 两帧之间的时间间隔 (Delta Time)
     */
    void update(float dt) override;

    // Cocos2d-x 宏：自动实现 create() 静态工厂方法
    CREATE_FUNC(LoadingScene);

private:

    /**
     * @brief 进度条控件
     * 使用 cocos2d::ui::LoadingBar，可以通过 setPercent() 控制显示长度。
     */
    cocos2d::ui::LoadingBar* loading_bar_;

    /**
     * @brief 百分比文本标签
     * 用于实时显示 "0%" 到 "100%" 的数字。
     */
    cocos2d::Label* percent_label_;

    /**
     * @brief 当前加载进度
     * 范围 0.0f - 100.0f。
     * 在 update 函数中随时间增加。
     */
    float current_percent_ = 0.0f;
};

#endif // __LOADING_SCENE_H__
