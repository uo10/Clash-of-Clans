#ifndef __TOWNHALL_H__
#define __TOWNHALL_H__

#include "base_building.h"

class TownHall : public BaseBuilding
{
public:
    /*
    * 创建TownHall对象
    * @param level 等级
    * @return 创建成功，返回对象，失败则返回空指针
    */
    static TownHall* Create(int level);

    /*
    * 初始化对象
    * @param level 等级
    * @return 初始化成功返回true 否则返回false
    */
    virtual bool Init(int level);

    /**
     * @brief 获取某等级大本营下，某种建筑的最大建造数量
     * @param building_type 要建造的建筑类型
     * @param town_hall_level 当前大本营等级
     * @return 建筑的最大建造数量
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