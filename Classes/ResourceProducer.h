#ifndef __RESOURCE_PRODUCER_H__
#define __RESOURCE_PRODUCER_H__

#include "BaseBuilding.h"

/**
 * @brief 资源生产类建筑 (ResourceProducer)
 * 继承自 BaseBuilding。
 * 专门用于处理具有“自动生产”功能的建筑（如：金矿、圣水收集器）。
 * 核心逻辑：在 update 中随时间累加资源，点击时收集资源。
 */
class ResourceProducer : public BaseBuilding {
public:
    // ------------------------------------------------
    // 特有属性
    // ------------------------------------------------

    /**
     * @brief 生产速率
     * 单位通常为：资源/秒。
     * 计算公式为：配置每小时产量 / 3600。
     */
    float productionRate;

    /**
     * @brief 内部暂存容量上限
     * @return 矿机内部能存放的最大资源量。达到此值后停止生产，直到玩家收集。
     * 注意：区别于仓库(capacity)的容量。
     */
    float maxCapacity;

    /**
     * @brief 当前暂存资源量
     * @return 还没被玩家收集走的资源
     */
    float currentRes;

    // ------------------------------------------------
    // UI 组件
    // ------------------------------------------------

    /**
     * @brief 资源气泡图标
     * 当暂存资源达到一定比例（如20%）时，在建筑头顶显示的图标（金币/圣水滴）。
     * 提示玩家可以收集了。
     */
    cocos2d::Sprite* bubbleIcon;

    bool isBubbleShowing; // 标记气泡当前是否可见，避免重复设置显隐

    // ------------------------------------------------
    // 核心函数
    // ------------------------------------------------

    static ResourceProducer* create(BuildingType type, int level);

    /**
     * @brief 初始化函数
     * 1. 调用父类 init 初始化通用属性。
     * 2. 初始化生产数值 (productionRate, maxCapacity)。
     * 3. 创建气泡图标并默认隐藏。
     * 4. 开启 scheduleUpdate。
     * 5. 创建触摸监听器。
     */
    virtual bool init(BuildingType type, int level) override;

    /**
     * @brief 帧循环更新 (核心生产逻辑)
     * 1. 只有在 IDLE 状态下才生产。
     * 2. currentRes += productionRate * dt。
     * 3. 检查是否达到上限，达到则停止增加。
     * 4. 根据 currentRes 的量决定是否显示 bubbleIcon。
     */
    virtual void update(float dt) override;

    /**
     * @brief 触摸回调
     * 检测玩家是否点击了该建筑。
     * @return
     * 如果点击了且有资源可收 -> 调用 collectResource()。
     * 如果点击了但没资源 -> 仅选中建筑（显示菜单）。
     */
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * @brief 执行收集动作
     * 1. 将 currentRes 加到玩家全局资源中。
     * 2. 清空 currentRes。
     * 3. 播放收钱音效。
     * 4. 隐藏气泡。
     * 5. 播放 "+100" 飘字动画。
     */
    void collectResource();

    /**
     * @brief 显示飘字特效
     * @param amount 收集到的数量
     */
    void showFloatText(int amount);

    /**
     * @brief 更新特有属性 (重写父类虚函数)
     * 当建筑升级(upgradeLevel)完成后，父类会调用此函数。
     * 这里的任务是：从新的 _stats 配置中读取 productionRate 和 capacity，
     * 并应用到当前的逻辑变量中，让升级立即生效。
     */
    virtual void updateSpecialProperties() override;
};
#endif