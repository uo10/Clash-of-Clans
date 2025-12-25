#include "FindPath.h"
#include <algorithm>

USING_NS_CC;

//寻找可行路径
std::vector<Vec2> FindPath::ComputePath(Vec2 start_grid, Vec2 end_grid, Size map_size, Size tile_size, std::function<bool(Vec2)> is_walkable)
{
    std::vector<Vec2> path;//最终可行路径点集合
    std::vector<PathNode*> open_list;//待检查的点集合
    std::vector<PathNode*> closed_list;//已经检查过的点集合

    PathNode* start_node = new PathNode((int)start_grid.x, (int)start_grid.y);
    PathNode* end_node = new PathNode((int)end_grid.x, (int)end_grid.y);
    open_list.push_back(start_node);

    PathNode* current_node = nullptr;

    while (!open_list.empty()) {
        // 找最小总路程的节点
        auto it = std::min_element(open_list.begin(), open_list.end(), [](PathNode* a, PathNode* b) { return a->GetTotalDis() < b->GetTotalDis(); });
        current_node = *it;

        // 到达终点
        if (current_node->x == end_node->x && current_node->y == end_node->y) {
            PathNode* temp = current_node;
            while (temp != nullptr) {
                // 转为像素中心坐标
                float pixel_x = temp->x * tile_size.width + tile_size.width / 2.0f;
                float pixel_y = temp->y * tile_size.height + tile_size.height / 2.0f;
                path.push_back(Vec2(pixel_x, pixel_y));
                temp = temp->parent;
            }
            //从终点入库，反转顺序
            std::reverse(path.begin(), path.end());
            break;
        }
        //将当前节点从待检查的点集合中消去，添加到已检查的点集合中
        open_list.erase(it);
        closed_list.push_back(current_node);

        // 检查当前节点上下左右节点是否可走
        int dirs[4][2] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
        for (int i = 0; i < 4; i++) {
            int nx = current_node->x + dirs[i][0];
            int ny = current_node->y + dirs[i][1];

            // 越界检查，放置走出地图
            if (nx < 0 || nx >= map_size.width || ny < 0 || ny >= map_size.height) continue;
            // 阻挡检查，防止走到block区域
            Vec2 tile;
            tile.x = nx;
            tile.y = ny;
            if (!is_walkable(tile)) {
                CCLOG("该点不可走！");
                continue;
            }
            // 检查 ClosedList，防止重复检查
            bool in_closed = false;
            for (auto n : closed_list) if (n->x == nx && n->y == ny) { in_closed = true; break; }
            if (in_closed) continue;

            // 检查 OpenList，更新节点值并纳入新节点
            PathNode* neighbor = nullptr;
            for (auto n : open_list) if (n->x == nx && n->y == ny) { neighbor = n; break; }

            int new_begin_dis = current_node->dis_to_begin + 1;
            //若neighbor不在待检查的集合中，则创建一个新neighbor节点。并纳入待检查点集合中
            if (!neighbor) {
                neighbor = new PathNode(nx, ny);
                neighbor->dis_to_begin = new_begin_dis;
                neighbor->dis_to_end = abs(nx - end_node->x) + abs(ny - end_node->y);
                neighbor->parent = current_node;
                open_list.push_back(neighbor);
            }
            //若在，则更新neighbor节点到起点的值和父节点
            else if (new_begin_dis < neighbor->dis_to_begin) {
                neighbor->dis_to_begin = new_begin_dis;
                neighbor->parent = current_node;
            }
        }
    }
    for (auto n : open_list) delete n;
    for (auto n : closed_list) delete n;
    return path;
}