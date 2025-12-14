#ifndef __TOWNHALL_H__
#define __TOWNHALL_H__

#include "BaseBuilding.h"

class TownHall : public BaseBuilding
{
public:
    // 标准创建函数
    static TownHall* create(int level);
    virtual bool init(int level);

    /**
     * @brief 获取某等级大本营下，某种建筑的最大建造数量
     * @param buildingType 要建造的建筑类型
     * @param townHallLevel 当前大本营等级
     */
    static int getMaxBuildingCount(BuildingType buildingType, int townHallLevel);

    /**
     * @brief 检查是否满足升级条件
     * @param buildingType 建筑类型
     * @param targetLevel 想要升到的等级
     * @param townHallLevel 当前大本营等级
     * @return true=允许升级, false=大本营等级不够
     */
    static bool isUpgradeAllowed(BuildingType buildingType, int targetLevel, int townHallLevel);
};

#endif