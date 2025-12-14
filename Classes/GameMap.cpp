#include "GameMap.h"

USING_NS_CC;

//创建GameMap对象
 GameMap* GameMap::create(const std::string& str) {
     GameMap* pRet = new(std::nothrow) GameMap(str);
     //创建成功返回对象指针
    if (pRet && pRet->init(str)) {
        pRet->autorelease();
        return pRet;
    }
    //否则返回空指针
    else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

//调用create函数
Scene* GameMap::createScene(const std::string& str)
{
	return GameMap::create(str);
}

//检查当前瓦块是否Block
bool GameMap::isTileBlock(Vec2 tileCoord)
{
    // 1、获取地图图层 
    //检查背景层
    auto layer = _Map->getLayer("BackGround");
    if (!layer) return false;  
    // 检查前景层
    auto layer1 = _Map->getLayer("ForeGround");
    if (!layer1) return false;
    // 检查建筑层
    auto layer2 = _Map->getLayer("BuildingGround");
    if (!layer2)return false;

    Size mapSize = _Map->getMapSize();
    //转换Y坐标，计算出的Y坐标原点在左下角，瓦块地图中原点在左上角
    int tiledY = mapSize.height - 1 - tileCoord.y;
    // 2、获取该网格坐标下的 GID (全局图块ID)
    //背景层
    int gid = layer->getTileGIDAt(Vec2(tileCoord.x, tiledY));
    //前景层
    int gid1 = layer1->getTileGIDAt(Vec2(tileCoord.x, tiledY));
    //建筑层
    int gid2 = layer2->getTileGIDAt(Vec2(tileCoord.x, tiledY));
    // 如果 GID 为 0，说明这地方是空的（没有图块），可通行
    if (gid == 0 && gid1 == 0 && gid2 == 0) return false;

    // 3、查询该 GID 的属性
    //背景层
    Value properties = _Map->getPropertiesForGID(gid);
    //前景层
    Value properties1 = _Map->getPropertiesForGID(gid1);
    //建筑层
    Value properties2 = _Map->getPropertiesForGID(gid2);
    // 4、检查Block属性
    //背景层
    if (!properties.isNull()) {
        ValueMap map = properties.asValueMap();
        if (map.find("Block") != map.end()) {
            // 如果属性存在，且为 true，则返回 true (表示是障碍物)
            drawGrid(tileCoord.x, tileCoord.y, Color4F::RED);
            return map.at("Block").asBool();
        }
    }
    //前景层
    if (!properties1.isNull()) {
        ValueMap map = properties1.asValueMap();
        if (map.find("Block") != map.end()) {
            drawGrid(tileCoord.x, tileCoord.y, Color4F::RED);
            return map.at("Block").asBool();
        }
    }
    //建筑层
    if (!properties2.isNull()) {
        ValueMap map = properties2.asValueMap();
        if (map.find("Block") != map.end()) {
            drawGrid(tileCoord.x, tileCoord.y, Color4F::RED);
            return map.at("Block").asBool();
        }
    }

    return false; // 默认没有障碍
}

//初始化地图
bool GameMap::init(const std::string& MapName)
{
    if (!Scene::init())
    {
        return false;
    }

    //获取初始窗口大小
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Director::getInstance()->getTextureCache()->removeAllTextures();

    // 1、加载 TMX 地图
    _Map = TMXTiledMap::create(MapName);

    Size mapSize = _Map->getMapSize(); // 图块数量
    Size tileSize = _Map->getTileSize(); // 单个图块像素 

    // 2、计算地图实际像素宽度和高度
    float mapPixelWidth = mapSize.width * tileSize.width;
    float mapPixelHeight = mapSize.height * tileSize.height;

    // 3、计算缩放因子
    float scaleX = visibleSize.width / mapPixelWidth;
    float scaleY = visibleSize.height / mapPixelHeight;

    // 4、设置缩放
    _Map->setScaleX(scaleX);
    _Map->setScaleY(scaleY);
    if (!_Map) return false;

    // ==================== 鼠标操作  ==================
    auto mouseListener = EventListenerMouse::create();

    // =================  一、滚轮缩放  ==================
    mouseListener->onMouseScroll = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        auto map = _Map;
        if (!map) return;//没有成功创建地图就返回

        Size mapContentSize = map->getContentSize(); // 获取地图原始大小

        float scrollY = e->getScrollY();// 获取滚轮滚动值
        float currentScaleX = map->getScaleX();// 获取当前X缩放值
        float currentScaleY = map->getScaleY();//   获取当前Y缩放值
        //获取缩放前鼠标坐标
        Vec2 mouseLocation = Vec2(e->getCursorX(), e->getCursorY());
        Vec2 nodePosBeforeScale = map->convertToNodeSpace(mouseLocation);
        // 1、计算新的缩放值
        float factor = (scrollY > 0) ? 1.1f : 0.9f;
        float newScaleX = currentScaleX * factor;
        float newScaleY = currentScaleY * factor;

        // ---------------------------------------------------------
        //限制最小缩放值 (防止地图缩得比屏幕还小,导致出现黑边)
        // ---------------------------------------------------------
        // 最小缩放比例 = 屏幕尺寸 / 地图原始尺寸
        float minScaleX = visibleSize.width / mapContentSize.width;
        float minScaleY = visibleSize.height / mapContentSize.height;

        // 保证不能小于最小缩放值
        if (newScaleX < minScaleX) newScaleX = minScaleX;
        if (newScaleY < minScaleY) newScaleY = minScaleY;

        // 限制最大缩放值
        if (newScaleX > 5.0f) newScaleX = 5.0f;
        if (newScaleY > 5.0f) newScaleY = 5.0f;

        // 应用缩放
        map->setScaleX(newScaleX);
        map->setScaleY(newScaleY);

        //调整地图位置，使鼠标位置不变
        Vec2 nodePosAfterScale = map->convertToNodeSpace(mouseLocation);
        Vec2 diff = nodePosAfterScale - nodePosBeforeScale;
        Vec2 currentPos = map->getPosition();
        map->setPosition(currentPos + Vec2(diff.x * newScaleX, diff.y * newScaleY));

        // ---------------------------------------------------------
        // 位置修正 (防止缩放后边缘露出黑底)
        // ---------------------------------------------------------

        // 计算当前地图缩放后的实际宽高
        float currentMapWidth = mapContentSize.width * newScaleX;
        float currentMapHeight = mapContentSize.height * newScaleY;

        // --- X轴修正 ---
        // 1、地图左边界不能往右跑 (x不能大于 0，否则左边出现黑边)
        if (currentPos.x > 0) {
            currentPos.x = 0;
        }
        // 2、地图右边界不能往左跑 (x不能小于屏幕宽 - 地图宽，否则右边出现黑边)
        float minX = visibleSize.width - currentMapWidth;
        if (currentPos.x < minX) {
            currentPos.x = minX;
        }

        // --- Y轴修正 ---
        // 1、地图下边界不能往上跑 (y不能大于0，否则下边出现黑边)
        if (currentPos.y > 0) {
            currentPos.y = 0;
        }
        // 2、地图上边界不能往下跑 (y不能小于屏幕高-地图高，否则上边出现黑边)
        float minY = visibleSize.height - currentMapHeight;
        if (currentPos.y < minY) {
            currentPos.y = minY;
        }
        map->setPosition(currentPos);

        };

    // ==================  二、鼠标按下  ==================
    mouseListener->onMouseDown = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        //1、判断是否点在菜单上
        Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());// 获取鼠标点击位置
        //Vec2 localPos = menu->getParent()->convertToNodeSpace(mousePos);// 转换到菜单节点空间
        /*if (menu->getBoundingBox().containsPoint(localPos)) {// 判断是否点在菜单区域内
            _isDragging = false;
            return; // 点在菜单上了，忽略拖拽
        }*/
        //2、如果是左键按下
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _isDragging = true; // 只有没点在菜单上，才允许拖拽
            _isClickValid = true;
            _lastMousePos = mousePos;
            _startClickPos = mousePos;
        }
        };

    // ==================  三、鼠标松开  ==================
    mouseListener->onMouseUp = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        auto map = _Map;
        if (!map) return;
        //1、判断是否点在菜单上
        Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());
        //Vec2 localPos = menu->getParent()->convertToNodeSpace(mousePos);
        /*if (menu->getBoundingBox().containsPoint(localPos)) {
            return; // 点在菜单上了，忽略拖拽
        }*/
        // 2、如果是左键松开
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {

            // 只有当没有发生大幅度拖拽时，才执行放置逻辑
            if (_isClickValid) {

                if (map) {
                    // 1、获取点击在地图内部的坐标
                    Vec2 nodePos = map->convertToNodeSpace(mousePos);

                    // 2、计算对应的瓦片坐标                
                    int tileX = (int)(nodePos.x / tileSize.width);
                    int tileY = (int)(mapSize.height - (nodePos.y / tileSize.height));

                    // 3、边界检查
                    if (tileX >= 0 && tileX < mapSize.width && tileY >= 0 && tileY < mapSize.height) {
                        Vec2 targetCoord = Vec2(tileX, tileY);// 目标瓦片坐标                     
                        // 检查该瓦片是否为障碍物
                        if (isTileBlock(targetCoord)) {
                            _isDragging = false;
                            CCLOG("Blocked! Cannot place item on water or mountain.");
                            return;
                        }
                        // 4、创建精灵                                                
                        float finalX = tileX * tileSize.width + tileSize.width / 2;
                        float finalY = (mapSize.height - 1 - tileY) * tileSize.height + tileSize.height / 2;
                        //调整大小，适应屏幕
                        spawnSoldier(Vec2(finalX, finalY));
                    }
                }
            }

            // 结束拖拽状态
            _isDragging = false;
        }
        };

    // ==================  四、鼠标移动  ==================
    mouseListener->onMouseMove = [=](Event* event) {
        // 只有当“正在拖拽”状态为 true 时才移动地图
        if (_isDragging) {
            EventMouse* e = (EventMouse*)event;
            auto map = _Map;
            if (!map) return;

            // 1、获取当前鼠标位置
            Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());

            // 2、计算移动距离 (Delta)
            Vec2 delta = currentMousePos - _lastMousePos;

            // 3、计算新位置
            Vec2 newPos = map->getPosition() + delta;

            // 限制边界，防止出现黑边
            Size mapContentSize = map->getContentSize();
            float currentMapWidth = mapContentSize.width * map->getScaleX();
            float currentMapHeight = mapContentSize.height * map->getScaleY();

            // X 轴限制
            float minX = visibleSize.width - currentMapWidth;
            float maxX = 0;
            if (newPos.x < minX) newPos.x = minX; // 右边黑边限制
            if (newPos.x > maxX) newPos.x = maxX; // 左边黑边限制

            // Y 轴限制
            float minY = visibleSize.height - currentMapHeight;
            float maxY = 0;
            if (newPos.y < minY) newPos.y = minY; // 上边黑边限制
            if (newPos.y > maxY) newPos.y = maxY; // 下边黑边限制

            map->setPosition(newPos);
            _lastMousePos = currentMousePos;
            // 如果鼠标移动距离超过阈值，取消点击有效性
            if (_isClickValid && currentMousePos.distance(_startClickPos) > 10.0f) {
                _isClickValid = false;
                CCLOG("Mode switched to Dragging. Click invalidated.");
            }
        }
        };
    // ================== 五、添加监听器捕获鼠标操作 ==================
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    //添加建筑对象层
    getBuildings();
    getForbiddenArea();
    this->addChild(_Map);
    //逐帧更新
    this->scheduleUpdate();
    debugDrawLogicGrid();
    return true;
}

//获取建筑对象层，并在指定位置添加对象
void GameMap::getBuildings()
{
    // 1. 获取对象层
    auto objectGroup = _Map->getObjectGroup("PlaceGround");
    if (!objectGroup) return;

    // 2. 遍历层中所有对象
    auto& objects = objectGroup->getObjects();
    Size tileSize = _Map->getTileSize();
    Size mapSize = _Map->getMapSize();
    float mapHeight = _Map->getMapSize().height * tileSize.height;

    for (auto& obj : objects) {
        cocos2d::ValueMap& dict = obj.asValueMap();

        // 读取坐标
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        std::string name = dict["name"].asString();
        CCLOG("Found Object: %s at (%f, %f)", name.c_str(), x, y);
        int row = int((x+1.0f ) / tileSize.width);
        int col = int((y+1.0f ) / tileSize.height);
        float finalX = row * tileSize.width + tileSize.width / 2;
        float finalY = (col) * tileSize.height + tileSize.height / 2;
        GameUnit* building;
        bool isset = false;
        // 3. 根据名字或类型生成不同的建筑
        //大炮（Cannon）
        if (name == "Cannon") {
           building = GameUnit::create("map/cannon.png", 400,0,10,200, UnitType::BUILDING_DEFENSE); // 800血
           building->setUnitName("Cannon");
           isset = true;
        }
        //大本营（TownHall）
        else if (name == "TownHall") {
            building = GameUnit::create("map/townhall.png", 1000,0,0,0,UnitType::BUILDING_RESOURCE); // 大本营血厚
            building->setUnitName("TownHall");
            isset = true;
        }
        //小屋（Hut）
        else if (name == "Hut") {
            building = GameUnit::create("map/Hut.png", 600, 0, 0, 0, UnitType::BUILDING_RESOURCE); // 大本营血厚
            building->setUnitName("Hut");
            isset = true;
        }
        //围栏（Fence）
        else if (name == "Fence") {
            building = GameUnit::create("map/Fence.png", 300, 0, 0, 0, UnitType::BUILDING_RESOURCE); // 大本营血厚
            isset = true;
            building->setUnitName("Fence");
        }
        if (isset) {
            Size spriteSize = building->getContentSize();
            building->setScaleX(tileSize.width / spriteSize.width);
            building->setScaleY(tileSize.height / spriteSize.height);
            //调整大小
            building->setPosition(Vec2(finalX, finalY));
            std::string key = StringUtils::format("%d_%d", row,col);  //读入哈希表中
            CCLOG("%s", building->getUnitName().c_str());
            _unitGridLookup[key] = building;
            _Map->addChild(building,10); // 加到地图上，随地图移动
            building->setLocalZOrder((int)(mapHeight - building->getPositionY()));
            _buildings.pushBack(building); // 存入列表管理
        }
    }
}

//放置士兵
void GameMap::spawnSoldier(Vec2 pos) {
    // 创建一个士兵(图像、血量、速度、伤害、攻击范围、阵营)
    auto soldier = GameUnit::create("R-C.jpg", 100, 100.0f, 20.0f, 50.0f, UnitType::SOLDIER);
    if (soldier) {
        soldier->setPosition(pos);
        Size tileSize = _Map->getTileSize();
        Size spriteSize = soldier->getContentSize();
        soldier->setScaleX(tileSize.width / spriteSize.width);
        soldier->setScaleY(tileSize.height / spriteSize.height);
        //调整大小
        _Map->addChild(soldier, 20); // Z轴比建筑高一点
        _soldiers.pushBack(soldier);
    }
}

//获取不可放置对象层
void GameMap::getForbiddenArea() {
    auto objectGroup = _Map->getObjectGroup("ForbiddenGround");
    if (!objectGroup) return;

    auto& objects = objectGroup->getObjects();
    for (auto& obj : objects) {
        ValueMap& dict = obj.asValueMap();

        // 获取矩形属性
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        float width = dict["width"].asFloat();
        float height = dict["height"].asFloat();

        // 存入列表（这些坐标是相对于地图左下角的坐标）
        _forbiddenAreas.push_back(Rect(x, y, width, height));
    }
}

//显示不可放置区
void GameMap::showForbiddenAreas(bool visible) {
    if (!_debugDrawNode) {
        _debugDrawNode = DrawNode::create();
        _Map->addChild(_debugDrawNode, 99); // 确保在最上层
    }

    _debugDrawNode->clear(); // 清除之前的绘制

    if (visible) {
        for (const auto& rect : _forbiddenAreas) {
            // 绘制半透明红色矩形
            _debugDrawNode->drawSolidRect(
                rect.origin,
                Vec2(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height),
                Color4F(1.0f, 0.0f, 0.0f, 0.4f) // 半透明红
            );
        }
    }
}

//检查当前位置是否在禁区内
bool GameMap::canPlaceSoldierAt(Vec2 mapPos) {
    // 遍历所有禁止区域
    for (const auto& rect : _forbiddenAreas) {
        // 使用 Rect 的 containsPoint 方法判断点是否在矩形内
        if (rect.containsPoint(mapPos)) {
            return false; // 在禁止区内
        }
    }
    return true; // 可以放置
}

//寻找最佳目标
GameUnit* GameMap::findBestTarget(Vec2 pos) {
    GameUnit* best = nullptr;
    float minDst = FLT_MAX;

    // 优先找防御塔
    for (auto b : _buildings) {
        if (b->isAlive() && b->getType() == UnitType::BUILDING_DEFENSE) {
            float dst = pos.distanceSquared(b->getPosition());
            if (dst < minDst) { minDst = dst; best = b; }//检查攻击范围
        }
    }
    if (best) return best;

    // 否则找最近的任意建筑
    minDst = FLT_MAX;
    for (auto b : _buildings) {
        if (b->isAlive()&&b->getUnitName()!="Fence") {
            float dst = pos.distanceSquared(b->getPosition());
            if (dst < minDst) { minDst = dst; best = b; }
        }
    }
    if (best) return best;
    for (auto b : _buildings) {
        if (b->isAlive()) {
            float dst = pos.distanceSquared(b->getPosition());
            if (dst < minDst) { minDst = dst; best = b; }
        }
    }
    return best;
}

//逐帧更新士兵状态
void GameMap::update(float dt)
{
    Size tileSize = _Map->getTileSize();
    float mapHeight = _Map->getMapSize().height * tileSize.height;

    for (auto soldier : _soldiers) {
        if (!soldier->isAlive()) continue;

        // 1. Z-Order 动态遮挡处理
        soldier->setLocalZOrder((int)(mapHeight - soldier->getPositionY()));

        // 2. 目标检测
        GameUnit* target = soldier->getTarget();
        if (!target || !target->isAlive()) {
            target = findBestTarget(soldier->getPosition()); // 找最近的大炮
            soldier->setTarget(target);
            if (target) calculatePath(soldier); // 找到新目标，计算路径
        }

        if (!target) continue; // 没目标停留原地（结束获胜）

        // 3. 距离检测
        float dist = soldier->getPosition().distance(target->getPosition());
        if (dist <= soldier->getRange()) {
            soldier->updateUnit(dt); // 在射程内，攻击
        }
        else {
            if (soldier->hasPath() && !soldier->isPathFinished()) {

                // 1. 获取目标点
                Vec2 nextPathNode = soldier->getNextStep();

                // 2. 转换为网格坐标
                int targetGridX = (int)(nextPathNode.x / tileSize.width);
                int targetGridY = (int)(nextPathNode.y / tileSize.height);               

                // 3. 查表检测
                GameUnit* obstacle = getUnitAtGrid(targetGridX, targetGridY);

                // 4. 判定逻辑
                bool isBlocked = false;

                if (obstacle && obstacle->isAlive()) {
                    // 只要这个格子里有东西，且不是当前目标，且是敌对的围栏就进攻
                    if (obstacle->getTeam() != soldier->getTeam()) {
                        if (obstacle->getUnitName() == "Fence") {

                            // 只要还没把围栏设为目标，就拦截
                            if (soldier->getTarget() != obstacle) {
                                CCLOG("!!! BLOCKED BY FENCE at (%d, %d) !!!", targetGridX, targetGridY);
                                soldier->setTarget(obstacle);
                                soldier->clearPath();
                                isBlocked = true;
                            }
                            // 防止士兵已经锁定了围栏，但还在往前走，走进围栏体内
                            else {
                                isBlocked = true;
                            }
                        }
                    }
                }

                // 5. 只有未受阻挡才移动
                if (!isBlocked) {
                    Vec2 direction = (nextPathNode - soldier->getPosition()).getNormalized();
                    Vec2 nextFramePos = soldier->getPosition() + direction * soldier->getSpeed() * dt;
                    soldier->setPosition(nextFramePos);

                    // 检查是否到达路点
                    if (soldier->getPosition().distance(nextPathNode) < 5.0f) {
                        soldier->advancePath();
                    }
                }
            }
        }
    }
    updateTowers(dt);
    // 清理死亡单位的 GridLookup （可改为智能指针）
    for (auto it = _unitGridLookup.begin(); it != _unitGridLookup.end(); ) {
        if (!it->second->isAlive()) it = _unitGridLookup.erase(it);
        else ++it;
    }
}

//更新防御塔状态
void GameMap::updateTowers(float dt) {
    for (auto building : _buildings) {
        // 1. 只有活着的防御塔才工作
        if (!building->isAlive() || building->getType() != UnitType::BUILDING_DEFENSE) continue;

        // 2. 寻找目标 (如果没有目标，或者目标死了/跑出了射程)
        GameUnit* target = building->getTarget();
        if (!target || !target->isAlive() ||
            building->getPosition().distance(target->getPosition()) > building->getRange()) {

            // 重新寻找最近的士兵
            float minDst = building->getRange(); // 初始值为射程，超过射程忽视
            GameUnit* newTarget = nullptr;

            for (auto soldier : _soldiers) {
                if (!soldier->isAlive()) continue;
                float dst = building->getPosition().distance(soldier->getPosition());
                if (dst < minDst) {
                    minDst = dst;
                    newTarget = soldier;
                }
            }
            building->setTarget(newTarget);
            target = newTarget;
        }

        // 3. 如果有目标，执行攻击逻辑
        if (target) {
            static std::map<GameUnit*, float> towerCooldowns; // 简单静态表记录冷却
            towerCooldowns[building] += dt;

            if (towerCooldowns[building] >= 1.0f) { // 设置攻击间隔为1s
                towerCooldowns[building] = 0;
                shootCannonBall(building, target); // 发射
            }
        }
    }
}

//发出炮弹
void GameMap::shootCannonBall(GameUnit* tower, GameUnit* target) {
    if (!tower || !target) return;

    // 1. 创建炮弹精灵
    auto ball = Sprite::create("map/cannonball.png"); 
    if (!ball) return;

    // 初始位置设为塔的位置
    ball->setPosition(tower->getPosition());
    Size towersize = tower->getContentSize();
    ball->setScale(0.5f);
    _Map->addChild(ball, 100);

    // 2. 计算飞行参数
    Vec2 startPos = tower->getPosition();
    Vec2 endPos = target->getPosition();
    float distance = startPos.distance(endPos);
    float speed = 300.0f; // 炮弹速度
    float duration = distance / speed;

    // 3. 创建动作序列
    // 动作A: 移动到目标当前位置
    auto moveTo = MoveTo::create(duration, endPos);
    // 动作B: 击中回调
    auto hitCallback = CallFunc::create([=]() {
        // a. 移除炮弹
        ball->removeFromParent();
        // b. 再次检查目标是否还活着 (防止子弹飞的时候人死了)
        if (target && target->isAlive()) {
            // c. 造成伤害 (假设塔的攻击力是 10)
            target->getDamage(10);
        }
        });

    // 执行动作
    ball->runAction(Sequence::create(moveTo, hitCallback, nullptr));
}

//检查当前坐标是否有建筑阻挡
bool GameMap::isBuildingBlock(Vec2 tile)
{
    int x = tile.x;
    int y = tile.y;
    GameUnit* building = getUnitAtGrid(x,y);
    if (building==nullptr) {
        return false;
    }
    std::string name = building->getUnitName();
    //Fence可被检测，并打破
    if (name != "Fence") {
        return true;
    }
    return false;
}

//计算攻击路径
void GameMap::calculatePath(GameUnit* soldier) {
    if (!soldier->getTarget()) return;
    Size tileSize = _Map->getTileSize();
    Size mapSize = _Map->getMapSize();

    // 像素转网格
    auto toGrid = [&](Vec2 p) { return  Vec2((int)((p.x) / tileSize.width),
        (int)((p.y) / tileSize.height)); };
    Vec2 start = toGrid(soldier->getPosition());
    Vec2 end = toGrid(soldier->getTarget()->getPosition());

    // 定义可走性
    auto isWalkable = [&](Vec2 tile) {
        //终点可走
        if (tile.x == (int)end.x && tile.y == (int)end.y) {
            return true;
        }
        // Tileblock 不可走
        if (isTileBlock(tile)) return false;
        //Buildingblock 不可走
        if (isBuildingBlock(tile))return false;
        //其余均可走
        return true;
        };

    //创建路径
    std::vector<Vec2> path = FindPath::findPath(start, end, mapSize, tileSize, isWalkable);
    soldier->setPath(path);
}

//获取当前坐标下的建筑
GameUnit* GameMap::getUnitAtGrid(int x, int y) {
    std::string key = StringUtils::format("%d_%d", x, y);
    if (_unitGridLookup.count(key)) return _unitGridLookup[key];
    return nullptr;
}

//对tileblock区域画出区分
void GameMap::drawGrid(int x, int y, Color4F color) {
    if (!_debugDrawNode) {
        _debugDrawNode = DrawNode::create();
        _Map->addChild(_debugDrawNode, 999);
    }

    Size tileSize = _Map->getTileSize();
    Vec2 origin(x * tileSize.width, y * tileSize.height);
    Vec2 dest(origin.x + tileSize.width, origin.y + tileSize.height);

    _debugDrawNode->drawRect(origin, dest, color);
}

//对building层画出区分
void GameMap::debugDrawLogicGrid() {
    auto drawNode = DrawNode::create();
    _Map->addChild(drawNode, 9999);
    Size tileSize = _Map->getTileSize();

    // 遍历查找表，把所有注册了单位的格子画个 绿框
    for (auto const&pair : _unitGridLookup) {
        if (!pair.second->isAlive()) continue;

        // 解析 key "x_y"
        int underscorePos = pair.first.find('_');
        int gx = std::stoi(pair.first.substr(0, underscorePos));
        int gy = std::stoi(pair.first.substr(underscorePos + 1));

        Vec2 origin(gx * tileSize.width, gy * tileSize.height);
        Vec2 dest = origin + Vec2(tileSize.width, tileSize.height);

        // 绿色空心框
        drawNode->drawRect(origin, dest, Color4F::GREEN);
    }
}