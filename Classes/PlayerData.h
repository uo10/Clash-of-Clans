#ifndef __PLAYER_DATA_H__
#define __PLAYER_DATA_H__

#include "cocos2d.h"
#include "BaseBuilding.h"
#include "AudioEngine.h" // 引入音频引擎，用于控制声音播放

USING_NS_CC;

/**
 * @brief 建筑存档数据结构体
 * 用于序列化（Save）和反序列化（Load）地图上的建筑。
 * 只存储恢复建筑所需的最核心数据，不存储临时状态（如当前血量）。
 */
struct BuildingData {
    BuildingType type; // 建筑类型 (枚举)
    int level;         // 建筑等级
    int tileX;         // 在瓦片地图上的 X 网格坐标
    int tileY;         // 在瓦片地图上的 Y 网格坐标
};

/**
 * @brief 玩家全局数据管理类 (单例模式)
 *
 * 核心职责：
 * 1. 数值管理：金币、圣水、人口的当前值与上限值。
 * 2. 兵力管理：存储训练好的士兵数据，跨场景（大本营<->战斗地图）共享。
 * 3. 存档管理：暂存建筑布局数据，用于场景切换时的销毁与重建。
 * 4. 音频管理：全局背景音乐(BGM)和音效(SFX)的播放与音量控制。
 */
class PlayerData {
private:
    /**
     * @brief 私有构造函数
     * 遵循单例模式原则，禁止外部直接使用 new PlayerData() 创建实例。
     */
    PlayerData();

    /** 静态单例指针，全局唯一 */
    static PlayerData* _instance;

    // ================= 资源核心数据 =================

    int _totalGold;   // 当前持有的金币数量 (资源)
    int _totalElixir; // 当前持有的圣水数量 (资源)
    int _totalPeople; // 当前已占用的人口数量 (兵力占用)

    int _maxGold;     // 金币存储上限 (由所有金库等级累加决定)
    int _maxElixir;   // 圣水存储上限 (由所有圣水瓶等级累加决定)
    int _maxPeople;   // 人口总上限 (由所有兵营等级累加决定) 

    // ================= 兵力核心数据 =================

    /**
     * @brief 兵力库存容器
     * Key: 士兵名称 (如 "Barbarian")
     * Value: 数量
     * 数据持久化存储在这里，不会因为场景切换而丢失。
     */
    std::map<std::string, int> _ownedTroops;

    // ================= 音频核心数据 =================

    /** 记录当前正在播放的背景音乐 ID，用于切换歌曲或调整音量 */
    int _currentBgmID = -1;

    // ================= 关卡信息 =================
    
    // 关卡是否通关状态
    std::map<int, int> _levelStars;

public:
    // ================= 公共成员变量 =================

    /**
     * @brief 建筑布局存档列表
     * 当玩家离开大本营进入战斗时，将所有建筑信息保存至此。
     * 当玩家回到大本营时，读取此列表重建地图。
     */
    std::vector<BuildingData> _villageLayout;

    float musicVolume = 1.0f;  // 全局背景音乐音量 (0.0 - 1.0)
    float effectVolume = 1.0f; // 全局音效音量 (0.0 - 1.0)

public:
    /**
     * @brief 获取单例实例 (工厂方法)
     * @return PlayerData* 全局唯一的指针。若未创建则自动创建。
     */
    static PlayerData* getInstance();

    /**
     * @brief 更新资源存储上限
     * 通常在以下情况调用：
     * 1. 读档恢复建筑后 (refreshTotalCapacity)。
     * 2. 存储类建筑建造/升级完成时。
     * @param maxGold 新的金币总容量
     * @param maxElixir 新的圣水总容量
     * @param maxPeople 新的人口总容量
     */
    void updateMaxLimits(int maxGold, int maxElixir, int maxPeople);

    // ================== 资源辅助查询 ==================

    /**
     * @brief 获取金币剩余存储空间
     * @return (最大容量 - 当前持有量)
     */
    int getGoldSpace() { return _maxGold - _totalGold; }

    /**
     * @brief 获取圣水剩余存储空间
     * @return (最大容量 - 当前持有量)
     */
    int getElixirSpace() { return _maxElixir - _totalElixir; }

    // ================== 金币操作 ==================

    /** @return 当前拥有的金币数 */
    int getGold();

    /**
     * @brief 增加金币
     * 逻辑：currentGold = min(currentGold + amount, maxGold)
     * @param amount 尝试增加的数量
     * @return 实际增加的数量 (受上限限制)
     */
    int addGold(int amount);

    /**
     * @brief 消耗金币
     * 用于建造、升级等。
     * @param amount 需要消耗的数量
     * @return true: 余额充足，扣除成功; false: 余额不足，扣除失败。
     */
    bool consumeGold(int amount);

    // ================== 圣水操作 ==================

    /** @return 当前拥有的圣水数 */
    int getElixir();

    /**
     * @brief 增加圣水
     * 逻辑同 addGold，会自动处理溢出。
     * @param amount 尝试增加的数量
     * @return 实际增加的数量
     */
    int addElixir(int amount);

    /**
     * @brief 消耗圣水
     * 用于造兵、科研等。
     * @param amount 需要消耗的数量
     * @return true: 余额充足，扣除成功; false: 余额不足。
     */
    bool consumeElixir(int amount);

    // ================== 人口操作 ==================

    /** @return 当前已占用的人口数量 */
    int getPeople();

    /**
     * @brief 尝试占用人口并扣除造兵资源
     * 这是一个组合操作，通常在点击训练士兵按钮时调用。
     *
     * 逻辑检查顺序：
     * 1. 检查人口空间是否足够 (_totalPeople + amount <= _maxPeople)。
     * 2. 检查圣水是否足够 (consumeElixir)。
     *
     * @param amount 该士兵占用的人口数
     * @param cost 该士兵消耗的圣水数
     * @return true: 资源和人口都充足，操作成功; false: 任一条件不满足。
     */
    bool addPeople(int amount, int cost);

    /**
     * @brief 释放人口
     * 用于取消训练或士兵死亡时，归还占用的人口空间。
     * @param amount 释放的数量
     */
    void removePeople(int amount);

    // ================= 士兵管理操作 ==================

    /**
     * @brief 增加士兵库存
     * 训练完成后调用，将士兵存入全局仓库。
     * @param name 士兵名称
     * @param count 增加数量
     */
    void addTroop(std::string name, int count);

    /**
     * @brief 消耗士兵库存
     * 用于战斗投放或取消训练。
     * @param name 士兵名称
     * @param count 消耗数量
     * @return true: 库存充足，扣除成功; false: 库存不足。
     */
    bool consumeTroop(std::string name, int count);

    /**
     * @brief 查询士兵库存
     * @param name 士兵名称
     * @return 当前拥有的数量
     */
    int getTroopCount(std::string name);

    /**
     * @brief 获取士兵数据的副本
     * 用于进入战斗场景时，创建一个临时的兵力表。
     * 这样在战斗中消耗兵力不会直接影响全局数据 (除非战斗结算时确认消耗)。
     * @return 全局兵力 Map 的拷贝
     */
    std::map<std::string, int> getTroopsCopy() {
        return _ownedTroops;
    }

    // ================= 音频管理操作 ==================

    /**
     * @brief 设置背景音乐音量
     * 同时更新变量并实时调整正在播放的 BGM 音量。
     * @param vol 音量值 (0.0 ~ 1.0)
     */
    void setMusicVol(float vol);

    /**
     * @brief 设置音效音量
     * 仅更新变量，下次播放音效时生效。
     * @param vol 音量值 (0.0 ~ 1.0)
     */
    void setEffectVol(float vol);

    /**
     * @brief 播放背景音乐 (BGM)
     * 会自动停止当前正在播放的 BGM (如果有)。
     * @param filename 音乐文件路径 (如 "bgm_village.mp3")
     */
    void playBGM(std::string filename,bool opt);

    /**
     * @brief 播放一次性音效 (SFX)
     * @param filename 音效文件路径 (如 "click.mp3")
     */
    void playEffect(std::string filename);

    // ================= 关卡管理操作 ==================

    /**
     * @brief 设置关卡通过状态 (战斗结算时调用)
     * 根据战斗结果更新存档中的星星数据。
     * 逻辑：
     * - 如果 isWin 为 true：直接将该关卡设为 3 星 (代表通关)。
     * - 如果 isWin 为 false：不修改现有数据 (保留原成绩)。
     *
     * @param levelID 关卡ID (1, 2, 3...)
     * @param isWin 是否取得胜利
     */
    void setLevelStatus(int levelID, bool isWin);

    /**
     * @brief 获取指定关卡的星星数量
     * 用于在“选关界面”显示该关卡的完成度。
     *
     * @param levelID 关卡ID
     * @return int 星星数量 (0 = 未打过或未通过, 3 = 已完美通关)
     */
    int getLevelStar(int levelID);

    /**
     * @brief 判断某关卡是否处于锁定状态
     * 用于决定选关界面中，该关卡显示为“锁”还是“可点击按钮”。
     *
     * 解锁规则：
     * 1. 第 1 关：永远解锁。
     * 2. 第 N 关：只有当第 N-1 关获得 3 星后，第 N 关才解锁。
     *
     * @param levelID 关卡ID
     * @return true: 锁定 (不可进入); false: 解锁 (可进入)
     */
    bool isLevelLocked(int levelID);
};

#endif // __PLAYER_DATA_H__