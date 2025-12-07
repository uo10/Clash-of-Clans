#ifndef __MAIN_VILLAGE_H__
#define __MAIN_VILLAGE_H__

#include "cocos2d.h"

//主村庄场景
class MainVillage :public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    /**
	 * 检查指定瓦片坐标是否为障碍物
     * 
	 * @param tileCoord 瓦片坐标
	 * @return 如果是障碍物，返回true，否则返回false
	 */
    bool isTileBlock(cocos2d::Vec2 tileCoord);
    // implement the "static create()" method manually
    CREATE_FUNC(MainVillage);
private:
    std::map<std::string, bool> _occupiedTiles;
    std::string _selectedSpritePath;
    bool _isDragging;       // 是否正在按住鼠标
    bool _isClickValid;     // 标记这次点击是否有效（用于生成精灵）
    cocos2d::Vec2 _lastMousePos; // 上一帧鼠标位置（用于移动地图）
    cocos2d::Vec2 _startClickPos;// 按下鼠标时的初始位置（用于判断距离）
    cocos2d::TMXTiledMap* _MainVillageMap; 
};
#endif // __MAIN_VILLAGE_H__