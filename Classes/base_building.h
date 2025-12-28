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
    kGoldMine = 1,      // 金矿 (生产金币)
    kElixirPump,    // 圣水收集器 (生产圣水)
    kGoldStorage,   // 金库 (存储金币)
    kElixirStorage, // 圣水瓶 (存储圣水)
    kBarracks,        // 兵营 (训练士兵)
    kCannon,         // 加农炮
    kArcherTower,   // 箭塔
    kWall,            // 围墙
	kTownHall,     // 大本营
    kNone = 0 // empty states
};

/**
 * @brief 建筑状态枚举
 * 这是一个简易的状态机定义，决定了建筑当前的表现和行为。
 */
enum class BuildingState {
    kPreview,    // 预览状态：跟随鼠标移动，半透明，绿色/红色指示能否建造
    kBuilding,   // 建造/升级中：显示倒计时和进度条，功能暂停
    kIdle,       // 正常待机：正在工作 (生产/存储/防御)
    kAttacking,  // 攻击状态：防御塔发现敌人并开火
    kDestroyed   // 摧毁状态：HP归零，显示废墟图，功能失效
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
    int cost_gold;       // 建造/升级所需金币
    int cost_elixir;     // 建造/升级所需圣水
    int build_time;      // 建造/升级所需时间(秒)

    // --- 资源类特有属性 ---
    int capacity;       // 资源存储容量
    int production_rate; // 资源生产效率 (每小时产量)

    // --- 防御类特有属性 ---
    double damage;       // 单次伤害

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
    BuildingStats stats_;

    // --- 倒计时 UI 组件 ---
    cocos2d::Node* progress_node_;       // 进度条的父节点(容器)
    cocos2d::Sprite* prog_bar_;          // 进度条
    cocos2d::Sprite* hammer_icon_ = nullptr; // 锤子图标
    // 标记当前倒计时是为了升级还是为了建造
    // true = 正在升级 (结束后 level++)
    // false = 正在建造 (结束后 level 不变，只是进入 IDLE)
    bool is_upgrading_target_ = false;
    cocos2d::Label* speed_up_cost_label_ = nullptr; // 显示加速金币数的文字


    // --- 时间变量 ---
    float build_total_time_ = 0.0f;       // 总时间
    float build_left_time_ = 0.0f;        // 剩余时间

    // 初始化进度条 UI 的辅助函数
    void InitBuildUI();


public:

    // ------------------------------------------------
    // 基础属性 (从配置中读取)
    // ------------------------------------------------
    int build_cost_gold_;   // 升级下一级所需的金币
    int build_cost_elixir_; // 升级下一级所需的圣水
    int build_time_seconds_;// 建造/升级所需总时间

    // ------------------------------------------------
    // 身份标识
    // ------------------------------------------------
    BuildingType type_;    // 建筑类型
    int level_;            // 当前等级
    int instance_id_;       // 实例ID (存档用，区分地图上多个同类建筑)
    std::string name_;     // 建筑名称

    // ------------------------------------------------
    // 战斗属性
    // ------------------------------------------------
    float max_hp_;          // 血量上限
    float current_hp_;      // 当前血量

    // ------------------------------------------------
    // 状态管理
    // ------------------------------------------------
    BuildingState state_;          // 当前状态机状态
    float build_time_total_;         // 建造总时长 (用于计算进度百分比)
    float build_time_remaining_;     // 剩余建造时间 (倒计时用)

    // ------------------------------------------------
    // UI 组件 (需在 updateUI 中刷新)
    // ------------------------------------------------
    cocos2d::ui::LoadingBar* build_bar_;    // 建造/升级时的黄色进度条
    cocos2d::Label* time_label_;            // 显示剩余时间的文字 (如 "10s")
    cocos2d::Sprite* main_sprite_;          // 建筑的主体图片精灵

    // Key: 兵种名字 (如 "Barbarian"), Value: 士兵节点列表
    std::map<std::string, std::vector<cocos2d::Node*>> visual_troops_; //用来存储可视化军营士兵容器
    // 同时存在BaseBuilding里，和MainVillage里面的list区分开

    // ------------------------------------------------
    // 核心函数
    // ------------------------------------------------

    /**
     * @brief 静态创建函数 
     * @param type 建筑类型
     * @param level 初始等级
     * @return 创建成功的建筑指针，失败返回 nullptr
     */
    static BaseBuilding* Create(BuildingType type, int level);

    /**
     * @brief 初始化函数
     * 设置默认属性，加载配置，但不创建具体图片(由 updateView 处理)
     */
    virtual bool Init(BuildingType type, int level);

    /**
     * @brief 获取图片路径辅助函数
     * 根据类型和等级拼接文件名，例如: GOLD_MINE, 1 -> "gold_mine_1.png"
     */
    std::string GetTextureName(BuildingType type, int level);

    /**
     * @brief 
     * 根据当前的 type, level, state 统一刷新外观。
     * 处理：更换图片、显示/隐藏血条和进度条。
     */
    void UpdateView();

    /**
     * @brief 切换状态
     * 修改 state 变量，并自动调用 updateView() 刷新外观。
     * @param newState 目标状态
     */
    void ChangeState(BuildingState new_state);

    /**
     * @brief 升级逻辑
     * 1. 等级+1
     * 2. 读取新等级配置(_stats)
     * 3. 刷新属性(MaxHP, 产量等)
     * 4. 调用 updateView() 换新图
     */
    void UpgradeLevel();

    /**
     * @brief 虚函数：通知子类更新特有属性
     * 当 BaseBuilding 完成通用属性升级后调用。
     * 子类(如 ResourceProducer)需重写此函数来更新 productionRate 等。
     */
    virtual void UpdateSpecialProperties() {};

    /**
     * @brief 静态配置读取器
     * 模拟从数据库/配置表中查询指定等级的数值。
     */
    static BuildingStats GetStatsConfig(BuildingType type, int level);

    /**
       * @brief 开始升级过程
       * 当玩家点击“升级”按钮，并且资源和条件满足时调用。
       * 1. 检查是否已达最高等级。
       * 2. 检查是否满足大本营等级要求。
       * 3. 获取下一级配置（所需时间、资源）。
       * 4. 扣除资源。
       * 5. 设置建造时间 (`_buildTotalTime`, `_buildLeftTime`)。
       * 6. 切换建筑状态到 `BUILDING`。
       * 7. 显示进度条和倒计时文字。
       * 8. 触发 UI 刷新（如扣费后更新资源显示）。
       */
    void StartUpgradeProcess();

    /**
     * @brief 游戏帧更新函数 (每帧自动调用)
     * @param dt 距离上一帧的时间间隔（秒）。
     *
     * 核心逻辑：
     * 1. 只在 `BUILDING` 状态下执行倒计时。
     * 2. 减少剩余时间 (`_buildLeftTime`)。
     * 3. 更新进度条 (`_progBar`) 和计时文字 (`_timeLabel`)。
     * 4. 当时间耗尽 (`_buildLeftTime <= 0`) 时：
     *    - 如果是升级 (`_isUpgradingTarget == true`)，调用 `upgradeLevel()` 完成升级。
     *    - 如果是初次建造 (`_isUpgradingTarget == false`)，调用 `constructionFinished()` 完成建造。
     */
    virtual void update(float dt) override;

    /**
     * @brief 开始初次建造过程
     * 当玩家在地图上放置一个新建筑（非升级）时调用。
     * 1. 获取当前等级（通常是 Lv.1）的配置。
     * 2. 设置建造时间 (`_buildTotalTime`, `_buildLeftTime`)。
     * 3. 标记 `_isUpgradingTarget = false`，表明这是初次建造。
     * 4. 切换建筑状态到 `BUILDING`。
     * 5. 显示进度条和锤子图标。
     */
    void StartConstruction();

    void TakeDamage(int damage);

    /**
    * @brief 开始初次建造结束
    * 当一个建筑非升级初次建造完成后调用
    */
    void ConstructionFinished();
};

#endif