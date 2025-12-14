#ifndef __BASE_BUILDING_H__
#define __BASE_BUILDING_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

// ==========================================
// 1. 枚举定义
// ==========================================

/**
 * @brief 建筑类型枚举
 * 用于区分不同的建筑逻辑（如：只有 GOLD_MINE 会产金币，只有 BARRACKS 能造兵）
 */
enum class BuildingType {
    GOLD_MINE = 1,      // 金矿 (生产金币)
    ELIXIR_PUMP,    // 圣水收集器 (生产圣水)
    GOLD_STORAGE,   // 金库 (存储金币)
    ELIXIR_STORAGE, // 圣水瓶 (存储圣水)
    BARRACKS,        // 兵营 (训练士兵)
    CANNON,         // 加农炮
    ARCHER_TOWER,   // 箭塔
    WALL,            // 围墙
	TOWN_HALL,     // 大本营
    NONE = 0 // empty states
};

/**
 * @brief 建筑状态枚举
 * 这是一个简易的状态机定义，决定了建筑当前的表现和行为。
 */
enum class BuildingState {
    PREVIEW,    // 预览状态：跟随鼠标移动，半透明，绿色/红色指示能否建造
    BUILDING,   // 建造/升级中：显示倒计时和进度条，功能暂停
    IDLE,       // 正常待机：正在工作 (生产/存储/防御)
    ATTACKING,  // 攻击状态：防御塔发现敌人并开火
    DESTROYED   // 摧毁状态：HP归零，显示废墟图，功能失效
};

// ==========================================
// 2. 数据结构定义
// ==========================================

/**
 * @brief 建筑数值配置结构体
 * 用于从硬编码数据中读取“这一级”建筑的固定属性。
 * 不包含动态数据(如当前位置、当前血量)。
 */
struct BuildingStats {
    std::string name;   // 显示名称
    int hp;             // 血量上限
    int costGold;       // 建造/升级所需金币
    int costElixir;     // 建造/升级所需圣水
    int buildTime;      // 建造/升级所需时间(秒)

    // --- 资源类特有属性 ---
    int capacity;       // 资源存储容量
    int productionRate; // 资源生产效率 (每小时产量)
};

// ==========================================
// 3. 核心类定义
// ==========================================

/**
 * @brief 建筑基类 (BaseBuilding)
 * 所有具体建筑（金矿、炮塔、围墙）的父类。
 * 负责处理通用的逻辑：渲染图片、显示血条、升级、状态切换、承受伤害。
 * 继承自 Node，方便管理子节点 (Sprite, UI, Label等)。
 */
class BaseBuilding : public cocos2d::Node {
protected:
    // 保存当前等级的配置数据，供子类逻辑使用
    BuildingStats _stats;

public:
    // ------------------------------------------------
    // 基础属性 (从配置中读取)
    // ------------------------------------------------
    int buildCostGold;   // 升级下一级所需的金币
    int buildCostElixir; // 升级下一级所需的圣水
    int buildTimeSeconds;// 建造/升级所需总时间

    // ------------------------------------------------
    // 身份标识
    // ------------------------------------------------
    BuildingType type;    // 建筑类型
    int level;            // 当前等级
    int instanceID;       // 实例ID (存档用，区分地图上多个同类建筑)
    std::string name;     // 建筑名称

    // ------------------------------------------------
    // 战斗属性
    // ------------------------------------------------
    float maxHP;          // 血量上限
    float currentHP;      // 当前血量

    // ------------------------------------------------
    // 状态管理
    // ------------------------------------------------
    BuildingState state;          // 当前状态机状态
    float buildTimeTotal;         // 建造总时长 (用于计算进度百分比)
    float buildTimeRemaining;     // 剩余建造时间 (倒计时用)

    // ------------------------------------------------
    // UI 组件 (需在 updateUI 中刷新)
    // ------------------------------------------------
    cocos2d::ui::LoadingBar* hpBar;       // 位于头顶的绿色血条
    cocos2d::ui::LoadingBar* buildBar;    // 建造/升级时的黄色进度条
    cocos2d::Label* timeLabel;            // 显示剩余时间的文字 (如 "10s")
    cocos2d::Sprite* mainSprite;          // 建筑的主体图片精灵

    // ------------------------------------------------
    // 核心函数
    // ------------------------------------------------

    /**
     * @brief 静态创建函数 
     * @param type 建筑类型
     * @param level 初始等级
     * @return 创建成功的建筑指针，失败返回 nullptr
     */
    static BaseBuilding* create(BuildingType type, int level);

    /**
     * @brief 初始化函数
     * 设置默认属性，加载配置，但不创建具体图片(由 updateView 处理)
     */
    virtual bool init(BuildingType type, int level);

    /**
     * @brief 获取图片路径辅助函数
     * 根据类型和等级拼接文件名，例如: GOLD_MINE, 1 -> "gold_mine_1.png"
     */
    std::string getTextureName(BuildingType type, int level);

    /**
     * @brief 承受伤害逻辑
     * @param damage 伤害数值
     * @return bool 如果建筑因此次伤害被摧毁(HP<=0)返回 true，否则 false
     */
    bool takeDamage(float damage);

    /**
     * @brief 【视图刷新核心】
     * 根据当前的 type, level, state 统一刷新外观。
     * 处理：更换图片、显示/隐藏血条和进度条。
     */
    void updateView();

    /**
     * @brief 切换状态
     * 修改 state 变量，并自动调用 updateView() 刷新外观。
     * @param newState 目标状态
     */
    void changeState(BuildingState newState);

    /**
     * @brief 升级逻辑
     * 1. 等级+1
     * 2. 读取新等级配置(_stats)
     * 3. 刷新属性(MaxHP, 产量等)
     * 4. 调用 updateView() 换新图
     */
    void upgradeLevel();

    /**
     * @brief 虚函数：通知子类更新特有属性
     * 当 BaseBuilding 完成通用属性升级后调用。
     * 子类(如 ResourceProducer)需重写此函数来更新 productionRate 等。
     */
    virtual void updateSpecialProperties() {};

    /**
     * @brief 帧刷新 UI
     * 通常在 scheduleUpdate 中调用，用于平滑更新血条和进度条的百分比。
     */
    void updateUI();

    /**
     * @brief 静态配置读取器
     * 模拟从数据库/配置表中查询指定等级的数值。
     */
    static BuildingStats getStatsConfig(BuildingType type, int level);
};

#endif