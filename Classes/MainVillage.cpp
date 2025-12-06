#include "MainVillage.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
Scene* MainVillage::createScene()
{
	return MainVillage::create();
}
bool MainVillage::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto MainVillageMap = TMXTiledMap::create("MainVillage.tmx");
	this->addChild(MainVillageMap, -1);
    return true;
}
void MainVillage::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}