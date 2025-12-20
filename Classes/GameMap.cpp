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
    
    // 5、设置临时士兵数据
    _battleTroops = PlayerData::getInstance()->getTroopsCopy();

    // 7、设置放置士兵菜单
    createTroopMenu();

    // 8、================== 右上角设置按钮 ======================

   // 1. 创建容器 (Wrapper)
    auto settingWrapper = Node::create();
    settingWrapper->setContentSize(Size(100, 100));
    settingWrapper->setAnchorPoint(Vec2(0.5, 0.5));

    // 2. 创建图标
    auto settingSprite = Sprite::create("Settings_Icon.png"); 

    // --- 自动缩放逻辑 ---
    float targetSetSize = 100.0f;
    float setContentMax = std::max(settingSprite->getContentSize().width, settingSprite->getContentSize().height);

    // 防止除以0
    if (setContentMax <= 0) setContentMax = 80.0f;

    float setScale = targetSetSize / setContentMax;
    settingSprite->setScale(setScale);

    // --- 定位 ---
    // 放在容器正中心
    settingSprite->setPosition(settingWrapper->getContentSize().width / 2, settingWrapper->getContentSize().height / 2);
    settingWrapper->addChild(settingSprite);

    // 3. 创建按钮
    auto btnSettings = MenuItemSprite::create(settingWrapper, nullptr, [=](Ref* sender) {

        // --- 点击反馈动画 ---
        // 让内部的图标缩放一下
        settingSprite->stopAllActions();
        settingSprite->runAction(Sequence::create(
            ScaleTo::create(0.1f, setScale * 1.2f), // 变大一点
            ScaleTo::create(0.1f, setScale),        // 恢复
            nullptr
        ));

        // --- 打开设置弹窗 ---
        CCLOG("Clicked Settings");
        this->showSettingsLayer();
        });

    // 4. 设置位置 (右上角)
    btnSettings->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));

    // 5. 添加到菜单
    auto settingsMenu = Menu::create(btnSettings, nullptr);
    settingsMenu->setPosition(Vec2::ZERO);
    this->addChild(settingsMenu, 2000); // UI层级，保证在最上层

    // 9、================== 播放音乐 ======================
        PlayerData::getInstance()->playBGM("bgm_battle_planning.mp3"); // 未放士兵 准备阶段

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

    // ------判断鼠标是否点到了右键菜单------
    auto isMouseOnMenu = [=](Vec2 mousePos) -> bool {
        if (_troopMenuNode) {
            // --- 情况 A: 士兵选择菜单 ---
            auto infoBg = _troopMenuNode->getChildByName("TroopsInfo");
            if (infoBg) {
                Vec2 localPos = infoBg->convertToNodeSpace(mousePos);

                Size s = infoBg->getContentSize();
                Rect bgRect = Rect(0, 0, s.width, s.height);

                // 判断鼠标是否在背景板范围内
                if (bgRect.containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
                else {
                  // 士兵菜单要求一直打开，不用关闭
                }
            }
        }
        // --- 情况 B: 设置界面 ---
        if (_settingsLayer) {
            // 只要设置界面开着（指针不为空）
            _isDragging = false;    // 禁止拖拽地图
            _isClickValid = false;  // 禁止放置操作

            return true; 
        }
        return false;
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
        // 如果点在右键菜单上，直接结束，_isClickValid = false
        if (isMouseOnMenu(mousePos)) {
            _isDragging = false;
            _isClickValid = false;
            return;
        }
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
                        // 判断是否为有该种士兵
                        if (_battleTroops.find(_currentSelectedTroop) != _battleTroops.end() &&
                            _battleTroops[_currentSelectedTroop] > 0) {
                            // 调整大小，适应屏幕
                            spawnSoldier(_currentSelectedTroop, Vec2(finalX, finalY));
                            // -1
                            _battleTroops[_currentSelectedTroop]--;
                            // 刷新 UI
                            updateTroopCountUI(_currentSelectedTroop);
                            // 第一次放兵 切换到激昂的战斗音乐
                            if (!_hasBattleStarted) {
                                _hasBattleStarted = true; // 标记已开战

                                // 切换到激昂的战斗音乐
                                PlayerData::getInstance()->playBGM("bgm_battle.mp3");

                                CCLOG("战斗打响！切换 BGM");
                            }
                        }

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
void GameMap::spawnSoldier(std::string troopName, Vec2 pos) {

    /* 根据名字判断创建具体的兵种对象 
    * 利用士兵类的初始化 这里先统一调用妙蛙种子
    if (troopName == "Barbarian") {
        soldier = Barbarian::create(); 
    }
    else if (troopName == "Archer") {
        soldier = Archer::create();
    }
    else if (troopName == "Giant") {
        soldier = Giant::create();
    }
    else if (troopName == "WallBreaker") {
        soldier = WallBreaker::create();
    }*/
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
    // 先判断：如果暂停了，直接跳过所有逻辑
    if (_isGamePaused || _isGameOver) return;

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

    // --- 胜负判断检测 ---
    if (_isGameOver) return;    // 如果游戏已经结束，就不再检测

    checkGameState();    // 执行检测
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

// 绘制士兵选择菜单
void GameMap::createTroopMenu() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建底部面板容器
    _troopMenuNode = Node::create();
    _troopMenuNode->setPosition(Vec2(visibleSize.width / 2, 80)); // 屏幕底部居中
    this->addChild(_troopMenuNode, 1000); // UI层级高

    // 背景板 (半透明黑条)
    auto bg = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width, 140);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5, 0.5));
    bg->setPosition(Vec2::ZERO);
    bg->setName("TroopsInfo");
    _troopMenuNode->addChild(bg);

    // 2. 创建选中高亮框 (一开始隐藏)
    _selectionHighlight = Sprite::create("selected_frame.png"); // 找一个发光的框图
    if (!_selectionHighlight) {
        // 这里先用用黄色框代替
        _selectionHighlight = Sprite::create();
        _selectionHighlight->setTextureRect(Rect(0, 0, 90, 110));
        _selectionHighlight->setColor(Color3B::YELLOW);
        _selectionHighlight->setOpacity(100);
    }
    _selectionHighlight->setVisible(false);
    _troopMenuNode->addChild(_selectionHighlight, 0); // 层级在按钮下面，背景上面

    // 3. 动态创建按钮的辅助函数 
    auto createBtn = [&](MainVillage::TroopInfo info, int index)->Node* {

        // 获取玩家拥有的该兵种数量
        int count = 0;
        if (_battleTroops.find(info.name) != _battleTroops.end()) {
            count = _battleTroops[info.name];
        }

        // 如果数量为0，选择变灰显示。
        bool hasTroops = (count > 0);

        // --- 容器 ---
        auto container = Node::create();
        container->setContentSize(Size(80, 100)); // 稍微大一点

        // --- Wrapper 和 Sprite ---
        auto iconWrapper = Node::create();
        iconWrapper->setContentSize(Size(70, 70));
        iconWrapper->setAnchorPoint(Vec2(0.5, 0.5));

        auto sprite = Sprite::create(info.img);

        float s = 70.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
        sprite->setScale(s);
        sprite->setPosition(35, 35);
        if (!hasTroops) sprite->setColor(Color3B::GRAY); // 没兵变灰
        iconWrapper->addChild(sprite);

        // --- 按钮逻辑 ---
        auto btn = MenuItemSprite::create(iconWrapper, nullptr, [=](Ref* sender) {
            // 点击回调
            int currentCount = _battleTroops[info.name];
            if (currentCount > 0) { // 只有有兵才能选中
                _currentSelectedTroop = info.name; // 设置选兵

                // 移动高亮框到当前按钮位置
                _selectionHighlight->setVisible(true);
                float btnX = (index - (MainVillage::troops.size() - 1) / 2.0f) * 100; 
                _selectionHighlight->setPosition(btnX + 40, 10);

                // 缩放动画反馈
                sprite->stopAllActions();
                sprite->runAction(Sequence::create(ScaleTo::create(0.1f, s * 1.2f), ScaleTo::create(0.1f, s), nullptr));

                CCLOG("选中出战兵种: %s", info.name.c_str());
            }
            else {
                CCLOG("该兵种数量为0，无法出战");
            }
            });
        btn->setPosition(40, 20);
        btn->setEnabled(hasTroops); // 没兵禁用点击

        // --- 数量标签 ---
        auto countLbl = Label::createWithSystemFont(StringUtils::format("x%d", count), "Arial", 20);
        countLbl->setColor(hasTroops ? Color3B::GREEN : Color3B::RED); // 有兵为绿 没兵为红
        countLbl->setPosition(40, -30); // 图标下方
        container->addChild(countLbl);

        // 存起来 用来更新UI
        _troopCountLabels[info.name] = countLbl;

        // --- 组装菜单 ---
        auto menu = Menu::create(btn, nullptr);
        menu->setPosition(Vec2::ZERO);
        container->addChild(menu);

        return container;
        };

    // 4. 排列按钮
    float startX = -((MainVillage::troops.size() - 1) * 100.0f) / 2.0f; // 居中排列
    for (int i = 0; i < MainVillage::troops.size(); ++i) {
        auto itemNode = createBtn(MainVillage::troops[i], i);
        itemNode->setPosition(startX + i * 100, 10);
        _troopMenuNode->addChild(itemNode);
    }
}

// 更新士兵数量label
void GameMap::updateTroopCountUI(std::string name) {
    // 1. 获取最新数量
    int currentCount = 0;
    // 这里用的是临时复制的数据
    if (_battleTroops.find(name) != _battleTroops.end()) {
        currentCount = _battleTroops[name];
    }

    // 2. 更新 Label
    if (_troopCountLabels.count(name)) {
        auto lbl = _troopCountLabels[name];
        lbl->setString(StringUtils::format("x%d", currentCount));

        if (currentCount <= 0) {
            lbl->setColor(Color3B::RED);
            // 如果当前选中的兵种用光了，取消选中状态
            if (_currentSelectedTroop == name) {
                _currentSelectedTroop = "";
                _selectionHighlight->setVisible(false);
            }
        }
    }
}

// 判断游戏结束 
void GameMap::checkGameState() {

    // -------------- 1. 检测胜利 (除了墙以外的建筑全毁) --------------
    bool hasLivingBuilding = false;

    for (auto building : _buildings) {
        // 排除空指针
        if (!building) continue;

        // 1. 如果是围墙，跳过
        if (building->getUnitName() == "Fence") continue;

        // 2. 检查是否存活 
        if (building->getCurrentHP() > 0) {
            hasLivingBuilding = true;
            break; // 只要还有一个活着的，就不算赢，直接退出循环
        }
    }

    if (!hasLivingBuilding) {
        // 场上没有活着的关键建筑 说明胜利
        _isGameOver = true;
        CCLOG("============= VICTORY! =============");

        // 停止所有战斗逻辑 
        // this->unscheduleUpdate();

         // 3. 呼叫弹窗 (参数 true 代表胜利)
        this->showGameOverLayer(true);
        return; // 赢了就不用检查是否失败
    }

    // ---------------- 2. 检测失败 (没库存兵 且 场上兵全死) -----------------

    // Step A: 检查库存 (是否还有没放出来的兵)
    bool hasReserves = false;
    // 遍历 map
    for (auto const& pair : _battleTroops) {
        // pair.first  是 name
        // pair.second 是 count

        int count = pair.second; // 获取数量

        if (count > 0) {
            hasReserves = true;
            break;
        }
    }

    // 如果还有库存，说明还没输，直接返回
    if (hasReserves) return;

    // Step B: 检查场上 (是否还有活着的兵)
    bool hasActiveSoldiers = false;
    for (auto soldier : _soldiers) {
        if (soldier && soldier->getCurrentHP() > 0) {
            hasActiveSoldiers = true;
            break;
        }
    }

    // 如果没库存 且 场上没兵 说明失败
    if (!hasActiveSoldiers) {
        _isGameOver = true;
        CCLOG("============= DEFEAT... =============");

        // 呼叫弹窗 (参数 false 代表失败)
        this->showGameOverLayer(false);
    }
}

// 游戏结束UI弹窗
void GameMap::showGameOverLayer(bool isWin) {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // --------------- 1. 创建半透明遮罩 ---------------
    // 初始透明度设为 0，为了做渐变动画
    auto layer = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width, visibleSize.height);

    // 保证盖住所有兵、建筑、UI菜单
    this->addChild(layer, 20000);

    // --- 添加触摸拦截 ---
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // 吞噬事件
    listener->onTouchBegan = [](Touch*, Event*) { return true; }; // 拦截所有点击
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, layer);

    // --------------- 2. 创建内容容器 -----------------
    auto container = Node::create();
    container->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    container->setScale(0.0f); // 初始缩放到0，准备弹出来
    layer->addChild(container);

    // -------------- 3. 根据胜负设置标题 --------------
    std::string titleStr = isWin ? "VICTORY!" : "DEFEAT";
    Color3B titleColor = isWin ? Color3B::YELLOW : Color3B::RED;

    auto lblTitle = Label::createWithSystemFont(titleStr, "Arial", 120);
    lblTitle->setColor(titleColor);
    lblTitle->enableOutline(Color4B::BLACK, 4); 
    lblTitle->setPosition(0, 50); // 居中偏上
    container->addChild(lblTitle);

    // ----------- 4. 创建 "返回大本营" 按钮 ------------
    auto btnLabel = Label::createWithSystemFont("Return Home", "Arial", 30);
    auto btnItem = MenuItemLabel::create(btnLabel, [=](Ref*) {
        // 返回大本营逻辑
        auto homeScene = MainVillage::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, homeScene));
        });

    // 呼吸动画吸引点击
    btnItem->runAction(RepeatForever::create(Sequence::create(
        ScaleTo::create(0.5f, 1.1f),
        ScaleTo::create(0.5f, 1.0f),
        nullptr
    )));

    auto menu = Menu::create(btnItem, nullptr);
    menu->setPosition(0, -60); 
    container->addChild(menu);

    // 5. ------------- 执行入场动画 -------------

    // 动画 A: 出现背景
    layer->runAction(FadeTo::create(0.5f, 200)); // 设置透明度200

    // 动画 B: 文字面板
    auto popUpSeq = Sequence::create(
        DelayTime::create(0.3f), // 延迟0.3s
        EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)), // 弹性放大
        nullptr
    );
    container->runAction(popUpSeq);
}

// 设置界面显示
void GameMap::showSettingsLayer() {
    // 防止重复打开
    if (_settingsLayer) return;

    // 1. 暂停游戏逻辑
    this->_isGamePaused = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 2. 遮罩层 
    _settingsLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    _settingsLayer->setName("SettingsLayer"); // 命名

    // 触摸拦截
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _settingsLayer);
    this->addChild(_settingsLayer, 20000); // 最顶层

    // =============================================================
       // 3. 背景板 (修改为图片)
       // =============================================================
       // 请将 "Settings_Panel.png" 替换为你实际的背景图片文件名
    std::string bgPath = "setting_panel.png";

    // 使用九宫格 Scale9Sprite，这样可以随意拉伸大小而不变形
    // 如果没有九宫格需求，直接用 Sprite::create 也可以，但 Scale9Sprite 更适合 UI 面板
    // Rect(边距) 需要根据你的图片实际边框厚度调整，这里假设是 20px
    auto bg = ui::Scale9Sprite::create(bgPath);

    if (!bg) {
        // 防崩处理：没图用色块
        auto sprite = Sprite::create();
        sprite->setTextureRect(Rect(0, 0, 100, 100));
        sprite->setColor(Color3B(50, 50, 50));
        bg = ui::Scale9Sprite::createWithSpriteFrame(sprite->getSpriteFrame());
    }

    // 设置面板大小 (宽450, 高350，根据内容调整)
    bg->setContentSize(Size(450, 350));
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    bg->setName("SettingsBackground"); // 用于点击检测

    _settingsLayer->addChild(bg);

    // 4. 标题
    auto lblTitle = Label::createWithSystemFont("SETTINGS", "Arial", 30);
    lblTitle->enableOutline(Color4B::BLACK, 2); // 加个描边更清晰
    lblTitle->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 40);
    bg->addChild(lblTitle);

  // ====================== 音量调节 (加减按钮模式) ======================

    // =============================================================
    // 定义一个通用的“创建音量控制条”函数
    // 参数1: 标题 (Music/Effect)
    // 参数2: Y坐标
    // 参数3: 获取当前值的函数
    // 参数4: 设置值的回调函数
    // =============================================================
    auto createVolumeControl = [&](std::string title, std::string iconPath, float posY, std::function<float()> getVal, std::function<void(float)> setVal) {

        // 0. 创建小喇叭图标 

        auto icon = Sprite::create(iconPath);

        if (!icon) {
            // 防崩：没图画个黄圈
            auto draw = DrawNode::create();
            draw->drawSolidCircle(Vec2::ZERO, 10, 0, 10, Color4F::YELLOW);
            icon = Sprite::create();
            icon->addChild(draw);
        }
        else {
            // 缩放图标到合适大小 (比如 30x30)
            float targetSize = 30.0f;
            float maxSide = std::max(icon->getContentSize().width, icon->getContentSize().height);
            if (maxSide > 0) icon->setScale(targetSize / maxSide);
        }

        // 位置：放在最左边 (比如 X=30)
        icon->setPosition(50, posY);
        bg->addChild(icon);
        // 1. 标题
        auto lbl = Label::createWithSystemFont(title, "Kenney Future Narrow", 24);
        lbl->setAnchorPoint(Vec2(0, 0.5));
        lbl->setPosition(80, posY);
        bg->addChild(lbl);

        // 2. 格子容器
        auto barNode = Node::create();
        barNode->setPosition(200, posY);
        bg->addChild(barNode);

        // 刷新格子的辅助函数
        auto refreshBar = [=](float percent) {
            barNode->removeAllChildren();
            int level = (int)(percent * 10 + 0.5f); // 0.5 -> 5

            for (int i = 0; i < 10; i++) {
                auto block = Sprite::create();
                block->setTextureRect(Rect(0, 0, 15, 20));
                // 亮色用绿色，暗色用深灰
                block->setColor(i < level ? Color3B(0, 255, 0) : Color3B(50, 50, 50));
                block->setPosition(i * 18, 0);
                barNode->addChild(block);
            }
            };

        // 初始刷新
        refreshBar(getVal());

        // 3. 减号按钮 [-]
        auto lblMinus = Label::createWithSystemFont("-", "Kenney Future Narrow", 40);
        lblMinus->enableOutline(Color4B::BLACK, 2);
        auto btnMinus = MenuItemLabel::create(lblMinus, [=](Ref*) {
            float v = getVal();
            int level = (int)(v * 10 + 0.5f);
            if (level > 0) {
                level--;
                setVal(level / 10.0f); // 调用外部传入的设置函数
                refreshBar(level / 10.0f); // 刷新显示

                // 如果是调节音效，最好播放一下声音给玩家听听大小
                if (title == "Effect") PlayerData::getInstance()->playEffect("click.mp3");
            }
            });
        btnMinus->setPosition(170, posY);

        // 4. 加号按钮 [+]
        auto lblPlus = Label::createWithSystemFont("+", "Kenney Future Narrow", 40);
        lblPlus->enableOutline(Color4B::BLACK, 2);
        auto btnPlus = MenuItemLabel::create(lblPlus, [=](Ref*) {
            float v = getVal();
            int level = (int)(v * 10 + 0.5f);
            if (level < 10) {
                level++;
                setVal(level / 10.0f);
                refreshBar(level / 10.0f);
                if (title == "Effect") PlayerData::getInstance()->playEffect("click.mp3");
            }
            });
        btnPlus->setPosition(190 + 180 + 20, posY);

        // 返回菜单项，以便添加到主菜单
        return Vector<MenuItem*>{btnMinus, btnPlus};
        };

    // =============================================================
    // 使用上面的通用函数，创建两排控制器
    // =============================================================

    // 1. 音乐控制 (Music) - 放在 Y=220
    auto musicItems = createVolumeControl("Music", "icon_music.png", 220,
        []() { return PlayerData::getInstance()->musicVolume; }, // 获取
        [](float v) { PlayerData::getInstance()->setMusicVol(v); } // 设置
    );

    // 2. 音效控制 (Effect) - 放在 Y=160
    auto effectItems = createVolumeControl("Effect", "icon_effect.png", 160,
        []() { return PlayerData::getInstance()->effectVolume; }, // 获取
        [](float v) { PlayerData::getInstance()->setEffectVol(v); } // 设置
    );

    // =============================================================
    // 把所有按钮加到一个 Menu 里
    // =============================================================
    auto volMenu = Menu::create();
    for (auto item : musicItems) volMenu->addChild(item);
    for (auto item : effectItems) volMenu->addChild(item);

    volMenu->setPosition(Vec2::ZERO);
    bg->addChild(volMenu);

    // =============================================================
     // 5. 继续游戏 (Resume) - 保持文字按钮，调整位置
     // =============================================================
    auto btnResumeLabel = Label::createWithSystemFont("Resume", "Kenney Future Narrow", 28);
    // 加个描边让它好看点
    btnResumeLabel->enableOutline(Color4B::BLACK, 2);

    auto btnResume = MenuItemLabel::create(btnResumeLabel, [=](Ref*) {
        this->_isGamePaused = false;
        if (_settingsLayer) {
            _settingsLayer->removeFromParent();
            _settingsLayer = nullptr;
        }
        });
    // 放在底部偏上
    btnResume->setPosition(Vec2(bg->getContentSize().width / 2, 110));

    // =============================================================
    // 6. 退出战斗 (End Battle) - 修改为图片按钮
    // =============================================================

    // --- 容器 ---
    auto quitWrapper = Node::create();
    quitWrapper->setContentSize(Size(160, 60)); // 设定点击区域大小
    quitWrapper->setAnchorPoint(Vec2(0.5, 0.5));

    // --- 图片 ---
    auto quitSprite = Sprite::create("End_Battle.png");

    // 防崩
    if (!quitSprite) {
        quitSprite = Sprite::create();
        quitSprite->setTextureRect(Rect(0, 0, 140, 50));
        quitSprite->setColor(Color3B::RED);
        auto l = Label::createWithSystemFont("END BATTLE", "Kenney Future Narrow", 20);
        l->setPosition(70, 25);
        quitSprite->addChild(l);
    }

    // 缩放适应容器 (宽140, 高50)
    float qScaleX = 140.0f / std::max(1.0f, quitSprite->getContentSize().width);
    float qScaleY = 50.0f / std::max(1.0f, quitSprite->getContentSize().height);
    // 保持等比缩放，取较小值，或者非等比拉伸
    quitSprite->setScale(qScaleX, qScaleY);

    quitSprite->setPosition(80, 30); // 居中
    quitWrapper->addChild(quitSprite);

    // --- 按钮 ---
    auto btnQuit = MenuItemSprite::create(quitWrapper, nullptr, [=](Ref*) {
        // 直接回大本营
        auto homeScene = MainVillage::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, homeScene));
        });

    // 放在最下面
    btnQuit->setPosition(Vec2(bg->getContentSize().width / 2, 50));

    // 组装菜单
    auto bottomMenu = Menu::create(btnResume, btnQuit, nullptr);
    bottomMenu->setPosition(Vec2::ZERO);
    bg->addChild(bottomMenu);

    // 入场动画 (Q弹效果)
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}