#ifndef __FINDPATH_H__
#define __FINDPATH_H__

#include "cocos2d.h"
#include <vector>
#include <functional>

struct PathNode {
    int x, y;//x，y为在瓦块地图中的瓦块网格坐标
    int disToBegin, disToEnd;//disToBegin:从起点到当前位置已经走过的距离，简单的递增表示；disToEnd:从当前位置到终点的距离，用曼哈顿距离表示
    PathNode* parent;//前一步走过的节点
    PathNode(int _x, int _y) : x(_x), y(_y), disToBegin(0), disToEnd(0), parent(nullptr) {}
    int getTotalDis() const { return disToBegin + disToEnd; }//获取当前当前节点从起点到终点所需总路程
};

class FindPath {
public:
    /**
    * 寻找从起点到终点可行的路径
    * 
    * @param startGrid 起点坐标
    * @param endGrid 终点坐标
    * @param mapSize 地图尺寸（网格规格表示）
    * @param tileSize 瓦块尺寸
    * @param iswalkable 当前瓦块是否可走
    * 
    * @return 返回可走路径数组
    */
    static std::vector<cocos2d::Vec2> findPath(
        cocos2d::Vec2 startGrid,
        cocos2d::Vec2 endGrid,
        cocos2d::Size mapSize,
        cocos2d::Size tileSize,
        std::function<bool(cocos2d::Vec2)> isWalkable
    );
};
#endif//__FINDPATH_H__
