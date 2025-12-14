#include "FindPath.h"
#include <algorithm>

USING_NS_CC;

//寻找可行路径
std::vector<Vec2> FindPath::findPath(Vec2 startGrid, Vec2 endGrid, Size mapSize, Size tileSize, std::function<bool(Vec2)> isWalkable)
{
    std::vector<Vec2> path;//最终可行路径点集合
    std::vector<PathNode*> openList;//待检查的点集合
    std::vector<PathNode*> closedList;//已经检查过的点集合

    PathNode* startNode = new PathNode((int)startGrid.x, (int)startGrid.y);
    PathNode* endNode = new PathNode((int)endGrid.x, (int)endGrid.y);
    openList.push_back(startNode);

    PathNode* currentNode = nullptr;

    while (!openList.empty()) {
        // 找最小总路程的节点
        auto it = std::min_element(openList.begin(), openList.end(), [](PathNode* a, PathNode* b) { return a->getTotalDis() < b->getTotalDis(); });
        currentNode = *it;

        // 到达终点
        if (currentNode->x == endNode->x && currentNode->y == endNode->y) {
            PathNode* temp = currentNode;
            while (temp != nullptr) {
                // 转为像素中心坐标
                float pixelX = temp->x * tileSize.width + tileSize.width / 2.0f;
                float pixelY = temp->y * tileSize.height + tileSize.height / 2.0f;
                path.push_back(Vec2(pixelX, pixelY));
                temp = temp->parent;
            }
            //从终点入库，反转顺序
            std::reverse(path.begin(), path.end());
            break;
        }
        //将当前节点从待检查的点集合中消去，添加到已检查的点集合中
        openList.erase(it);
        closedList.push_back(currentNode);

        // 检查当前节点上下左右节点是否可走
        int dirs[4][2] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
        for (int i = 0; i < 4; i++) {
            int nx = currentNode->x + dirs[i][0];
            int ny = currentNode->y + dirs[i][1];

            // 越界检查，放置走出地图
            if (nx < 0 || nx >= mapSize.width || ny < 0 || ny >= mapSize.height) continue;
            // 阻挡检查，防止走到block区域
            Vec2 tile;
            tile.x = nx;
            tile.y = ny;
            if (!isWalkable(tile)) {
                CCLOG("Blocked! isnt walkable");
                continue;
            }
            // 检查 ClosedList，防止重复检查
            bool inClosed = false;
            for (auto n : closedList) if (n->x == nx && n->y == ny) { inClosed = true; break; }
            if (inClosed) continue;

            // 检查 OpenList，更新节点值并纳入新节点
            PathNode* neighbor = nullptr;
            for (auto n : openList) if (n->x == nx && n->y == ny) { neighbor = n; break; }

            int newBeginDis = currentNode->disToBegin + 1;
            //若neighbor不在待检查的集合中，则创建一个新neighbor节点。并纳入待检查点集合中
            if (!neighbor) {
                neighbor = new PathNode(nx, ny);
                neighbor->disToBegin = newBeginDis;
                neighbor->disToEnd = abs(nx - endNode->x) + abs(ny - endNode->y);
                neighbor->parent = currentNode;
                openList.push_back(neighbor);
            }
            //若在，则更新neighbor节点到起点的值和父节点
            else if (newBeginDis < neighbor->disToBegin) {
                neighbor->disToBegin = newBeginDis;
                neighbor->parent = currentNode;
            }
        }
    }
    for (auto n : openList) delete n;
    for (auto n : closedList) delete n;
    return path;
}