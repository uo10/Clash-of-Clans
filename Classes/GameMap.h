#ifndef __GAMEMAP_H__
#define __GAMEMAP_H__

#include "cocos2d.h"
#include"GameUnit.h"
#include"FindPath.h"
#include "MainVillage.h"
#include "Trap.h"
#include "ui/CocosGUI.h"

class GameMap : public cocos2d::Scene
{
public:
	//构造函数
	GameMap(std::string str) :_MapName(str), _isDragging(false), _isClickValid(false), _debugDrawNode(nullptr) {}
	// a selector callback
	void menuCloseCallback(cocos2d::Ref* pSender);
	/**
	* 创建一个GameMap对象
	* 
	* @param str 传入的地图名字
	*/
	static GameMap* create(const std::string& str);
	/**
	* 调用create函数
	* 
	* @param mapName 传入的地图名字
	*/
	static cocos2d::Scene* createScene(const std::string& mapName);
	/**
	* 初始化GameMap对象，调整地图大小，添加对象
	* 
	* @param MapName 传入的地图名字
	* 
	* @return 如果地图创建成功，返回true，否则返回false
	*/
	bool init(const std::string& MapName);
	/**
	* 获取瓦块地图上的建筑对象层，将实体对象添加到地图上
	* 
	*/
	void getBuildings();
	/**
	* 获取瓦块地图上的不可放置对象层，设置不可放置区
	* 
	*/
	void getForbiddenArea();
	/**
	* 显示不可放置区
	* 
	* @param visible 设置是否显示不可放置区 true显示，false不显示
	*/
	void showForbiddenAreas(bool visible);
	/**
	* 在指定地点放置士兵
	* 
	* @param pos 放置点的像素坐标
	*/
	void spawnSoldier(std::string troopName, cocos2d::Vec2 pos);
	/**
	* 检查该点是否可以放置士兵
	* 
	* @param mapPos 所选中的放置点
	* 
	* @return 如果该点在不可放置区内，返回false，否则返回true
	*/
	bool canPlaceSoldierAt(cocos2d::Vec2 mapPos);

	/**
	 * 检查指定瓦片坐标是否为地图预设障碍物
	 *
	 * @param tileCoord 瓦片坐标
	 * 
	 * @return 如果是障碍物，返回true，否则返回false
	 */
	bool isTileBlock(cocos2d::Vec2 tileCoord);
	/**
	* 检查指定瓦片坐标是否有添加的存活状态的对象
	* 
	* @param tileCoord 瓦片坐标
	* 
	* @return 如果有存活的对象，返回true，否则返回false
	*/
	bool isBuildingBlock(cocos2d::Vec2 tile);
	/**
	* 重写update函数，按帧更新士兵状态，实时选择路径
	* 
	* @param dt 每一帧的时间 
	*/
	virtual void update(float dt) override;
	/**
	* 按帧更新防御塔（大炮）状态，实时选择攻击对象
	* 
	* @param dt 每一帧的时间
	*/
	void updateTowers(float dt);
	/**
	* 从防御塔（大炮）向目标（士兵）发射炮弹
	* 
	* @param tower 发射炮弹的防御塔
	* @param target 被防御塔锁定的、待炮弹攻击的对象
	* 
	*/
	void shootCannonBall(GameUnit* tower, GameUnit* target);
	/**
	* 传入当前士兵位置，按优先级寻找攻击目标
	* 
	* @param pos 当前士兵的像素位置
	* 
	* @return 先找防御塔（大炮），若有防御塔则返回防御塔节点，否则返回其他最近的建筑，若建筑都已被消灭，返回空指针
	*/
	GameUnit* findBestTarget(cocos2d::Vec2 pos);
	/**
	* 为当前士兵计算到目标的最佳路径
	* 
	* @param soldier 需要计算路径的士兵
	*/
	void calculatePath(GameUnit* soldier);
	/**
	* 获取当前网格坐标下的的对象
	* 
	* @param x 网格横坐标
	* @param y 网格纵坐标
	* 
	* @return 当前网格坐标上的建筑对象，若对象都已不存在，返回空指针
	*/
	GameUnit* getUnitAtGrid(int x, int y);

	/**
    * @brief 初始化士兵选择菜单
    */
	void createTroopMenu();

	/**
    * @brief 更新士兵数量label（调用士兵之后）
	* @param name 更改label的士兵的名字
    */
	void updateTroopCountUI(std::string name);

	/**
	* @brief 判断游戏结束函数
	* 胜利：所有建筑HP为0
	* 失败：士兵容器为空 创建的士兵HP均为0
	*/
	void checkGameState();

	/**
	* @brief 游戏结束画面处理
	* @param isWin 是否胜利
	* true 为胜利 false 为失败
	*/
	void showGameOverLayer(bool isWin);

	/**
	* @brief 显示设置菜单
	* 战斗地图的设置菜单 
	* 包括：BGM和音效音量 结束战斗 返回按钮
	* 同时包括暂停战斗
	*/
	void showSettingsLayer();

private:
	std::string _MapName;//所要创建的地图名（tmx文件名）
	cocos2d::TMXTiledMap* _Map;//创建的瓦块地图
	cocos2d::Vector<GameUnit*>_buildings;//建筑对象数组，储存建筑对象层上的对象
	cocos2d::Vector<GameUnit*>_soldiers;//士兵数组，储存创建的士兵
	cocos2d::Vector<GameTrap*> _traps;//陷阱数组，储存陷阱
	std::map<std::string, GameUnit*> _unitGridLookup; // 哈希表，将网格坐标设置为Key,快速查找网格上的对象
	std::vector<cocos2d::Rect> _forbiddenAreas;//禁止区对象数组，储存禁止放置层上的对象
	cocos2d::DrawNode* _debugDrawNode; // 用于显示区域
	bool _isDragging;       // 是否正在按住鼠标
	bool _isClickValid;     // 标记这次点击是否有效（用于生成精灵）
	cocos2d::Vec2 _lastMousePos; // 上一帧鼠标位置（用于移动地图）
	cocos2d::Vec2 _startClickPos;// 按下鼠标时的初始位置（用于判断距离）

	cocos2d::Node* _troopMenuNode = nullptr;    // 士兵选择菜单容器
	std::string _currentSelectedTroop = "";    	// 当前选中的兵种名字 (例如 "Barbarian")，为空代表没选
	std::map<std::string, cocos2d::Label*> _troopCountLabels; 	// 记录 UI 上的数字标签，方便投放兵力后实时更新数量
	cocos2d::Sprite* _selectionHighlight = nullptr;  	// 选中框精灵 (用于显示哪个被选中了)
	std::map<std::string, int> _battleTroops;  // 战斗临时兵力表 进入战斗时copy PlayerData的数据
	cocos2d::LayerColor* _settingsLayer = nullptr;  // 存储设置覆盖层的指针
	bool _isGameOver = false; // 判断胜负逻辑
	bool _isGamePaused = false; // 暂停标志位
	bool _hasBattleStarted = false; // 标记是否开始战斗

};

#endif // __GAMEMAP_H__
