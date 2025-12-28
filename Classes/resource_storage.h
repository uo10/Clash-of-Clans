#ifndef __RESOURCE_STORAGE_H__
#define __RESOURCE_STORAGE_H__

#include "base_building.h"

/**
 * @brief 资源存储类建筑 (ResourceStorage)
 * 继承自 BaseBuilding。
 * 专门用于处理“仓库”类建筑（如：金库、圣水瓶）。
 * 核心逻辑：本身不产出资源，而是提供资源上限（Capacity）。
 * 当此类建筑建造或升级时，需要通知游戏全局增加资源储存上限。
 */
class ResourceStorage : public BaseBuilding {
public:
    // ------------------------------------------------
    // 特有属性
    // ------------------------------------------------

    /**
     * @brief 当前建筑提供的储存量上限
     * 例如：一个 1 级金库可能提供 1500 容量。
     * 此数值来源于 _stats.capacity。
     * 游戏全局总容量 = 所有 ResourceStorage 的 maxLimit 之和 + 大本营容量(设定的初始容量)。
     */
    int max_limit_;

    // ------------------------------------------------
    // 核心函数
    // ------------------------------------------------

    /**
     * @brief 静态创建函数
     * 工厂方法，创建一个新的仓库实例。
     */
    static ResourceStorage* Create(BuildingType type, int level);

    /**
     * @brief 初始化函数
     * 1. 调用父类 init 初始化通用属性(HP, 图片等)。
     * 2. 从配置中读取 capacity 赋值给 this->maxLimit。
     * 3. 初始化完成后，通常需要通知游戏场景刷新一次全局资源上限。
     */
    virtual bool Init(BuildingType type, int level) override;

    /**
     * @brief 更新特有属性 (重写父类虚函数)
     * 场景：当仓库升级完成 (upgradeLevel) 后被调用。
     * 逻辑：
     * 1. 从新的 _stats 中获取更大的 capacity 赋值给 maxLimit。
     * 2. 触发全局事件 (如 "REFRESH_MAX_CAPACITY")，让 UI 上的资源上限数字(如 1000/5000)及时更新。
     */
    virtual void UpdateSpecialProperties() override;
};

#endif