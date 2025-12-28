#ifndef COC_SPLASH_SCENE_H_
#define COC_SPLASH_SCENE_H_

#include "cocos2d.h"

/**
 * @brief 游戏启动温馨提示界面
 * 极其轻量级，用于掩盖后续资源的加载延迟。
 */
class SplashScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* CreateScene();
	virtual bool init();

	// 回调函数：跳转到加载条界面
	void GoToLoadingScene(float dt);

	CREATE_FUNC(SplashScene);
};

#endif // COC_SPLASH_SCENE_H_
