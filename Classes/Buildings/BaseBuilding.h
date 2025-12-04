#ifndef __BASE_BUILDING_H__
#define __BASE_BUILDING_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

// 建筑类型枚举
enum class BuildingType {
    GOLD_MINE,      // 金矿
    ELIXIR_PUMP,    // 圣水收集器
    GOLD_STORAGE,   // 金库
    ELIXIR_STORAGE  // 圣水瓶
};

// 建筑状态枚举
enum class BuildingState {
    PREVIEW,    // 建造预览中 (半透明，跟随着鼠标，还没真正造下来)
    BUILDING,   // 建造/升级倒计时中 (显示进度条，不能工作)
    IDLE,       // 正常待机中 (可以生产资源，可以被攻击)
    ATTACKING,  // 攻击中 (仅防御塔有效)
    DESTROYED   // 被摧毁 (显示废墟图片)
};

// 这是一个初始化数据包
struct BuildingStats {
    std::string name;   // 名字
    int hp;             // 血量
    int costGold;       // 造价(金币)
    int costElixir;     // 造价(圣水)
    int buildTime;      // 建造时间(秒)

    // 资源相关 (如果是仓库或矿)
    int capacity;       // 容量
    int productionRate; // 生产效率(每小时)
};

class BaseBuilding : public cocos2d::Node {
protected:
    //保存配置数据，供子类访问
    BuildingStats _stats;
public:

    // 1.建造/升级所需的资源
    int buildCostGold;   // 需要消耗的金币
    int buildCostElixir; // 需要消耗的圣水

    // 建造所需时间 (秒) 
    int buildTimeSeconds;

    //网格坐标
    int gridX;
    int gridY;
    // 建筑占地大小 (例如 3x3)
    int width;
    int height;

    // 2. 身份标识
    BuildingType type;    //建筑的类型
    int level;            //建筑的等级
    int instanceID;       // 唯一ID (例如：1001)，用于保存数据时区分是哪一个金矿
    int configID;         // 配置ID (例如：1)，对应 JSON 表里的 "gold_mine_lv1"
    std::string name;     // 建筑名称 (例如："Gold Mine")

    // 3. 战斗数值 (配合被别人攻击)
    float maxHP;          // 总血量
    float currentHP;      // 当前血量
    bool isTargetable;    // 是否可被攻击 (装饰品不可被攻击)

    // 4. 状态管理
    BuildingState state;          // 当前状态
    float buildTimeTotal;         // 建造/升级需要总时间 (秒)
    float buildTimeRemaining;     // 剩余建造时间

    // 5. UI 组件 (血条和进度条)
    cocos2d::ui::LoadingBar* hpBar;       // 位于头顶的血条
    cocos2d::ui::LoadingBar* buildBar;    // 建造进度条
    cocos2d::Label* timeLabel;            // 显示剩余时间文字 (如 "10s")

    cocos2d::Sprite* mainSprite; // 建筑主图

    virtual bool init(BuildingType type, int level);
    static BaseBuilding* create(BuildingType type, int level);

    // 设置在地图网格上的位置
    void setGridPosition(int x, int y);

    // 获取建筑名称（用于加载图片）
    std::string getTextureName(BuildingType type, int level);

    // 返回值 true 代表建筑被摧毁了
    bool takeDamage(float damage);
    
    // 刷新画面函数(在changeState和upgradeLevel函数中运用)
    void updateView();

    // 切换状态 (比如从 建造 -> 正常)
    void changeState(BuildingState newState);

    // 升级函数
    void upgradeLevel();

    // 通知子类更新自己特有属性
    virtual void updateSpecialProperties() {}; 

    // 更新血条显示
    void updateUI();

};

#endif