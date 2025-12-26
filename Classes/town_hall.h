#ifndef __TOWNHALL_H__
#define __TOWNHALL_H__

#include "base_building.h"

class TownHall : public BaseBuilding
{
public:
    static TownHall* Create(int level);
    virtual bool Init(int level);

    /**
     * @brief 获取某等级大本营下，某种建筑的最大建造数量
     * @param building_type 要建造的建筑类型
     * @param town_hall_level 当前大本营等级
     */
    static int GetMaxBuildingCount(BuildingType building_type, int town_hall_level);

    /**
     * @brief 检查是否满足升级条件
     * @param building_type 建筑类型
     * @param target_level 想要升到的等级
     * @param town_hall_level 当前大本营等级
     * @return true=允许升级, false=大本营等级不够
     */
    static bool IsUpgradeAllowed(BuildingType building_type, int target_level, int town_hall_level);
};

#endif