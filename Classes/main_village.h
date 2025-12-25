#ifndef __MAIN_VILLAGE_H__
#define __MAIN_VILLAGE_H__

#include "cocos2d.h"
#include "base_building.h"
#include "resource_producer.h"
#include "resource_storage.h"
#include "player_data.h"
#include "Wall.h"
#include "ArcherTower.h"
#include "TownHall.h"
#include "Cannon.h"
#include "Archer.h"
#include "Barbarian.h"
#include "WallBreaker.h"
#include "Giant.h"
#include "ui/CocosGUI.h"

//主村庄场景
class MainVillage :public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void MenuCloseCallback(cocos2d::Ref* p_sender);
    /**
	 * 检查指定瓦片坐标是否为障碍物
     * 
	 * @param tile_coord 瓦片坐标
	 * @return 如果是障碍物，返回true，否则返回false
	 */
    bool IsTileBlock(cocos2d::Vec2 tile_coord);
    // implement the "static create()" method manually
    CREATE_FUNC(MainVillage);

    /**
    * @brief 刷新资源上限
    * 遍历地图上所有的存储类建筑（金库 / 圣水瓶），累加它们的容量
    * 并更新全局的 maxGold 和 maxElixir 变量。
    * 在建筑建造完成或升级完成时调用。
    */
    void RefreshTotalCapacity();

    /**
     * @brief 初始化资源显示界面 (HUD)
     * 创建屏幕右上角的金币、圣水图标以及对应的数字标签。
     * 仅在场景初始化 (init) 时调用一次。
     */
    void CreateResourceUi();

    /**
     * @brief 刷新资源UI数值
     * 将界面上的数字标签更新为当前的资源值。
     * 在资源建筑收集资源时调用。
     */
    void UpdateResourceUi();

    /**
     * @brief 显示指定建筑的操作菜单
     * 当玩家点击地图上的某个建筑时调用。
     * 功能：
     * 1. 将菜单容器移动到该建筑头顶。
     * 2. 创建菜单按钮(升级、拆除、信息)。
     * @param building 被选中的目标建筑指针
     */
    void ShowBuildingMenu(BaseBuilding* building);

    /**
     * @brief 关闭当前建筑操作菜单
     * 隐藏操作菜单，并清理 _currentSelectedBuilding 指针。
     * 在点击空地或点击菜单按钮后调用。
     */
    void CloseBuildingMenu();

    /**
     * @brief 初始化建造选择菜单 (商店界面)
     * 创建底部的建造面板容器、背景图以及顶部的分类标签页(资源/防御)。
     * 默认隐藏，点击“建造”按钮时显示。
     */
    void CreateBuildUi();

    /**
     * @brief 切换建造面板显示的类别内容
     * 清空当前面板内的图标，根据传入的分类索引重新生成建筑图标。
     * @param category 分类索引:
     *                 0 = 资源类 (金矿、圣水收集器等)
     *                 1 = 防御类 (加农炮、箭塔、城墙等)
     */
    void SwitchBuildCategory(int category);

    /**
    * @brief 左下角Attack战斗按钮
    */
    void CreateAttackUi();

    /**
    * @brief 显示关卡选择弹窗
    */
    void ShowLevelSelection();

    /**
    * @brief 进攻之前存档 防止数据丢失
    */
    void SaveVillageData();

    /**
    * @brief 返回大本营读档 获取建筑信息
    */
    void RestoreVillageData();

    /**
    * @brief 绘制非法放置位置
    * 红色绘制非法区域
    */
    void UpdateOccupiedGridVisual();

    /**
    * @brief 显示设置菜单
    * 大本营的设置菜单
    * 包括：BGM和音效音量 返回按钮
    */
    void ShowSettingsLayer();

private:
    std::map<std::string, bool> occupied_tiles_;
    std::string selected_sprite_path_;
    bool is_dragging_;       // 是否正在按住鼠标
    bool is_click_valid_;     // 标记这次点击是否有效（用于生成精灵）
    cocos2d::Vec2 last_mouse_pos_; // 上一帧鼠标位置（用于移动地图）
    cocos2d::Vec2 start_click_pos_;// 按下鼠标时的初始位置（用于判断距离）
    cocos2d::TMXTiledMap* main_village_map_; 
    BuildingType selected_building_type_;

    cocos2d::Label* gold_label_;   // 金币UI
    cocos2d::Label* elixir_label_; // 圣水UI
    cocos2d::Label* people_label_; // 人口UI

   
    std::vector<ResourceStorage*> storage_list_; //用来储存所有建造的存储类建筑(用于计算当前资源量)

    cocos2d::Node* active_menu_node_ = nullptr;   // 建筑右键的菜单指针

    cocos2d::Node* build_menu_node_ = nullptr;    // 建筑选择菜单指针
    cocos2d::Node* icon_container_ = nullptr;    // 用于存放建筑(资源/防御)图标的容器

    cocos2d::DrawNode* grid_draw_node_ = nullptr;    // 用于绘制占用格子的画笔节点
    BaseBuilding* current_preview_building_ = nullptr;// 当前正在预览/拖拽的建筑指针

    cocos2d::LayerColor* settings_layer_ = nullptr;  // 设置覆盖层的指针

public:
    // 简单定义士兵结构体，后面有了士兵类可以删掉
    struct TroopInfo {
        std::string name;
        std::string img;
        int weight;
        int cost;
    };
    static std::vector<TroopInfo> troops_; 


};
#endif // __MAIN_VILLAGE_H__