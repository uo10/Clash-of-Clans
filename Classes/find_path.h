#ifndef __FINDPATH_H__
#define __FINDPATH_H__

#include "cocos2d.h"
#include <vector>
#include <functional>

struct PathNode {
    int x, y;//x，y为在瓦块地图中的瓦块网格坐标
    int dis_to_begin, dis_to_end;//dis_to_begin:从起点到当前位置已经走过的距离，简单的递增表示；disToEnd:从当前位置到终点的距离，用曼哈顿距离表示
    PathNode* parent;//前一步走过的节点
    PathNode(int _x, int _y) : x(_x), y(_y), dis_to_begin(0), dis_to_end(0), parent(nullptr) {}
    int GetTotalDis() const { return dis_to_begin + dis_to_end; }//获取当前当前节点从起点到终点所需总路程
};

class FindPath {
public:
    /**
    * 寻找从起点到终点可行的路径
    *
    * @param start_grid 起点坐标
    * @param end_grid 终点坐标
    * @param map_size 地图尺寸（网格规格表示）
    * @param tile_size 瓦块尺寸
    * @param iswalkable 当前瓦块是否可走
    *
    * @return 返回可走路径数组
    */
    static std::vector<cocos2d::Vec2> ComputePath(
        cocos2d::Vec2 start_grid,
        cocos2d::Vec2 end_grid,
        cocos2d::Size map_size,
        cocos2d::Size tile_size,
        std::function<bool(cocos2d::Vec2)> is_walkable
    );
};
#endif//__FINDPATH_H__
