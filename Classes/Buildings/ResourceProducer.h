#ifndef __RESOURCE_PRODUCER_H__
#define __RESOURCE_PRODUCER_H__

#include "BaseBuilding.h"

class ResourceProducer : public BaseBuilding {
public:
    float productionRate; // 每秒生产多少 
    float maxCapacity;    // 最大暂存容量
    float currentRes;     // 当前暂存资源

    cocos2d::Sprite* bubbleIcon; // 资源气泡图标
    bool isBubbleShowing;

    static ResourceProducer* create(BuildingType type, int level);
    virtual bool init(BuildingType type, int level) override;
    
    // 每一帧自动调用
    virtual void update(float dt) override;
    
    // 触摸事件回调
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    
    // 收集资源逻辑
    void collectResource();
    
    // 显示飘字动画
    void showFloatText(int amount);
};

#endif