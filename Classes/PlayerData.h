#ifndef __PLAYER_DATA_H__
#define __PLAYER_DATA_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * @brief 玩家全局数据类 (单例模式)
 * 负责管理玩家当前的资源数值（金币、圣水）以及资源容量上限。
 * 所有涉及到资源增加、消耗、显示的地方都应该调用此类。
 */
class PlayerData {
private:
    /**
     * @brief 私有构造函数
     * 确保外部无法通过 new PlayerData() 创建实例，维持单例唯一性。
     */
    PlayerData();

    static PlayerData* _instance; // 静态单例指针

    int _totalGold;   // 当前持有的金币数量
    int _totalElixir; // 当前持有的圣水数量
    int _totalPeople; // 当前拥有的人口数量

    int _maxGold;     // 金币存储上限 (由金库等级决定)
    int _maxElixir;   // 圣水存储上限 (由圣水瓶等级决定)
    int _maxPeople;   // 人口存储上限 (由兵营等级决定) 

public:
    /**
     * @brief 获取单例实例
     * @return 返回 PlayerData 的全局唯一指针。如果尚未创建，则会自动创建一个。
     */
    static PlayerData* getInstance();

    /**
     * @brief 更新资源上限
     * 当存储类建筑（金库/圣水瓶）建造完成或升级后调用此函数。
     * @param maxGold 新的金币总容量
     * @param maxElixir 新的圣水总容量
     * @param maxPeople 新的人口总容量
     */
    void updateMaxLimits(int maxGold, int maxElixir, int maxPeople);

    /**
     * @brief 获取金币剩余存储空间
     * 用于判断产出设施是否已满，或者是否还能收集资源。
     * @return (上限 - 当前值)
     */
    int getGoldSpace() { return _maxGold - _totalGold; }

    /**
     * @brief 获取圣水剩余存储空间
     * @return (上限 - 当前值)
     */
    int getElixirSpace() { return _maxElixir - _totalElixir; }

    // ================== 金币相关操作 ==================

    /**
     * @brief 获取当前金币数量
     * @return 当前金币数量
     */
    int getGold();

    /**
     * @brief 增加金币 (例如：收集金矿产出)
     * 会自动处理上限溢出，如果超过上限，则只取出最多数量
     * @param amount 尝试增加的数量
     * @return 实际增加的数量 
     */
    int addGold(int amount);

    /**
     * @brief 消耗金币 (例如：建造建筑、升级)
     * @param amount 需要消耗的数量
     * @return true: 余额充足且扣除成功; false: 余额不足，扣除失败。
     */
    bool consumeGold(int amount);

    // ================== 圣水相关操作 ==================

    /**
     * @brief 获取当前圣水数量
     * @return 当前圣水数量
     */
    int getElixir();

    /**
     * @brief 增加圣水 (例如：收集圣水收集器产出)
     * 会自动处理上限溢出。
     * @param amount 尝试增加的数量
     * @return 实际增加的数量
     */
    int addElixir(int amount);

    /**
     * @brief 消耗圣水 (例如：训练士兵、建造建筑)
     * @param amount 需要消耗的数量
     * @return true: 余额充足且扣除成功; false: 余额不足，扣除失败。
     */
    bool consumeElixir(int amount);

    // ================== 人口相关操作 ==================

    /**
    * @brief 获取当前圣水数量
    * @return 当前人口数量
    */
    int getPeople();

    /**
     * @brief 增加人口 (例如：生产士兵增加人口)
     * 会自动处理上限溢出。
     * @param amount 尝试增加的数量
     * @param amount 这次造兵需要的圣水
     * @return true: 容量充足且增加成功; false: 容量不足，增加失败。
     */
    bool addPeople(int amount,int cost);

    /**
     * @brief 返还人口 (点击减号 减少人口)
     * @param amount 返还的数量
     */
    void removePeople(int amount);

};

#endif
