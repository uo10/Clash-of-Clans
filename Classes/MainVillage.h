#ifndef __MAIN_VILLAGE_H__
#define __MAIN_VILLAGE_H__

#include "cocos2d.h"

//Ö÷´å×¯³¡¾°
class MainVillage :public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(MainVillage);
};
#endif // __MAIN_VILLAGE_H__