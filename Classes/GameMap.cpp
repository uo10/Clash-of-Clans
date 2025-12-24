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
            return map.at("Block").asBool();
        }
    }
    //前景层
    if (!properties1.isNull()) {
        ValueMap map = properties1.asValueMap();
        if (map.find("Block") != map.end()) {           
            return map.at("Block").asBool();
        }
    }
    //建筑层
    if (!properties2.isNull()) {
        ValueMap map = properties2.asValueMap();
        if (map.find("Block") != map.end()) {          
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
    this->current_map_file_name_ = MapName;
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
    battle_troops_ = PlayerData::GetInstance()->GetTroopsCopy();

    // 7、设置放置士兵菜单
    CreateTroopMenu();

    // 8、================== 右上角设置按钮 ======================

   // 1. 创建容器 (Wrapper)
    auto settingWrapper = Node::create();
    settingWrapper->setContentSize(Size(100, 100));
    settingWrapper->setAnchorPoint(Vec2(0.5, 0.5));

    // 2. 创建图标
    auto settingSprite = Sprite::create("Icon/Settings_Icon.png"); 

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

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

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
        this->ShowSettingsLayer();
        });

    // 4. 设置位置 (右上角)
    btnSettings->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));

    // 5. 添加到菜单
    auto settingsMenu = Menu::create(btnSettings, nullptr);
    settingsMenu->setPosition(Vec2::ZERO);
    this->addChild(settingsMenu, 2000); // UI层级，保证在最上层

    // 9、================== 播放音乐 ======================
        PlayerData::GetInstance()->PlayBgm("bgm_battle_planning.mp3",true); // 未放士兵 准备阶段

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
        if (troop_menu_node_) {
            // --- 情况 A: 士兵选择菜单 ---
            auto infoBg = troop_menu_node_->getChildByName("TroopsInfo");
            if (infoBg) {
                Vec2 localPos = infoBg->convertToNodeSpace(mousePos);

                Size s = infoBg->getContentSize();
                Rect bgRect = Rect(0, 0, s.width, s.height);

                // 判断鼠标是否在背景板范围内
                if (bgRect.containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
                else {
                  // 士兵菜单要求一直打开，不用关闭
                }
            }
        }
        // --- 情况 B: 设置界面 ---
        if (settings_layer_) {
            // 只要设置界面开着（指针不为空）
            is_dragging_ = false;    // 禁止拖拽地图
            is_click_valid_ = false;  // 禁止放置操作

            return true; 
        }
        return false;
        };

    // ==================  二、鼠标按下  ==================
    mouseListener->onMouseDown = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        //1、判断是否点在菜单上
        Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());// 获取鼠标点击位置        
        // 如果点在右键菜单上，直接结束，_isClickValid = false
        if (isMouseOnMenu(mousePos)) {
            int count = battle_troops_[current_selected_troop_];

            if (count > 0) {
                // 只有还有兵的时候，才显示放兵区域
                showForbiddenAreas(true);             
            }           

            // 无论有没有兵，只要点了菜单，就不进行地图拖拽
            is_dragging_ = false;
            is_click_valid_ = false;
            return;
        }
        //2、如果是左键按下
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            is_dragging_ = true; // 只有没点在菜单上，才允许拖拽
            is_click_valid_ = true;
            last_mouse_pos_ = mousePos;
            start_click_pos_ = mousePos;
        }
        };

    // ==================  三、鼠标松开  ==================
    mouseListener->onMouseUp = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        auto map = _Map;
        if (!map) return;
        //1、判断是否点在菜单上
        Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());      
        // 2、如果是左键松开
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {

            // 只有当没有发生大幅度拖拽时，才执行放置逻辑
            if (is_click_valid_) {
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
                            is_dragging_ = false;
                            CCLOG("Blocked! Cannot place item on water or mountain.");
                            return;
                        }
                        if (!canPlaceSoldierAt(nodePos)) {
                            is_dragging_ = false;
                            CCLOG("Blocked! Cannot place item on Forbidden.");
                            return;
                        }
                        // 4、创建精灵                                                
                        float finalX = tileX * tileSize.width + tileSize.width / 2;
                        float finalY = (mapSize.height - 1 - tileY) * tileSize.height + tileSize.height / 2;
                        // 判断是否为有该种士兵
                        if (battle_troops_.find(current_selected_troop_) != battle_troops_.end() &&
                            battle_troops_[current_selected_troop_] > 0) {
                            // 调整大小，适应屏幕
                            spawnSoldier(current_selected_troop_, Vec2(finalX, finalY));
                            showForbiddenAreas(false);
                            // -1
                            battle_troops_[current_selected_troop_]--;
                            // 刷新 UI
                            UpdateTroopCountUi(current_selected_troop_);
                            // 第一次放兵 切换到激昂的战斗音乐
                            if (!has_battle_started_) {
                                has_battle_started_ = true; // 标记已开战                            
                                // 切换到激昂的战斗音乐
                                PlayerData::GetInstance()->PlayBgm("bgm_battle.mp3",true);

                                CCLOG("战斗打响！切换 BGM");
                            }
                        }

                    }
                }
            }

            // 结束拖拽状态
            is_dragging_ = false;
        }
        };

    // ==================  四、鼠标移动  ==================
    mouseListener->onMouseMove = [=](Event* event) {
        // 只有当“正在拖拽”状态为 true 时才移动地图
        if (is_dragging_) {
            EventMouse* e = (EventMouse*)event;
            auto map = _Map;
            if (!map) return;

            // 1、获取当前鼠标位置
            Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());

            // 2、计算移动距离 (Delta)
            Vec2 delta = currentMousePos - last_mouse_pos_;

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
            last_mouse_pos_ = currentMousePos;
            // 如果鼠标移动距离超过阈值，取消点击有效性
            if (is_click_valid_ && currentMousePos.distance(start_click_pos_) > 10.0f) {
                is_click_valid_ = false;
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
           building = GameUnit::create("map/cannon.png", 420,0,7.2,100, UnitType::BUILDING_DEFENSE); // 420血
           building->setAttackTime(0.8f);
           building->setUnitName("Cannon");
           isset = true;
        }
        //大本营（TownHall）
        else if (name == "TownHall") {
            building = GameUnit::create("map/townhall.png", 1850,0,0,0,UnitType::BUILDING_RESOURCE); // 大本营血厚
            building->setUnitName("TownHall");
            isset = true;
        }
        //小屋（Hut）
        else if (name == "Hut") {
            building = GameUnit::create("map/Hut.png", 250, 0, 0, 0, UnitType::BUILDING_RESOURCE); // 大本营血厚
            building->setUnitName("Hut");
            isset = true;
        }
        //围栏（Fence）
        else if (name == "Fence") {
            building = GameUnit::create("map/Fence.png", 300, 0, 0, 0, UnitType::BUILDING_RESOURCE); // 大本营血厚
            isset = true;
            building->setUnitName("Fence");
        }
        else if (name == "Trapbomb") {
            auto trap = GameTrap::createTrap(Vec2(finalX, finalY),tileSize);         
            _traps.pushBack(trap);
            _Map->addChild(trap, 5);
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

    GameUnit* soldier = nullptr;

    // 创建士兵对象
    if (troopName == "Barbarian") {
        soldier = GameUnit::create("Barbarian.png", 45, 70.0f, 8.0f, 40.0f, UnitType::SOLDIER);
    }
    else if (troopName == "Archer") {
        soldier = GameUnit::create("Archer.png", 20, 100.0f, 7.0f, 80.0f, UnitType::SOLDIER);
    }
    else if (troopName == "Giant") {
        soldier = GameUnit::create("Giant.png", 300, 50.0f, 11.0f, 40.0f, UnitType::SOLDIER);
    }
    else if (troopName == "WallBreaker") {
        soldier = GameUnit::create("Wall_Breaker.png", 20, 100.0f, 12.0f, 40.0f, UnitType::SOLDIER);
    }

    if (soldier) {
        // 设置名字
        soldier->setUnitName(troopName);

        soldier->setPosition(pos);
        Size tileSize = _Map->getTileSize();
        Size spriteSize = soldier->getContentSize();

        soldier->setScaleX(tileSize.width / spriteSize.width);
        soldier->setScaleY(tileSize.height / spriteSize.height);

        _Map->addChild(soldier, 20);
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
    if (is_game_paused_ || is_game_over_) return;

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
    //检测陷阱激活情况
    for (auto trap : _traps) {
        trap->updateTrap(dt, _soldiers);
    }
    updateTowers(dt);
    // 清理死亡单位的 GridLookup
    for (auto it = _unitGridLookup.begin(); it != _unitGridLookup.end(); ) {
        if (!it->second->isAlive()) it = _unitGridLookup.erase(it);
        else ++it;
    }

    // --- 胜负判断检测 ---
    if (is_game_over_) return;    // 如果游戏已经结束，就不再检测

    CheckGameState();    // 执行检测
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

// 绘制士兵选择菜单
void GameMap::CreateTroopMenu() {
    auto visible_size = Director::getInstance()->getVisibleSize();

    // 1. 创建底部面板容器
    troop_menu_node_ = Node::create();
    troop_menu_node_->setPosition(Vec2(visible_size.width / 2, 80)); // 屏幕中心位置偏下
    this->addChild(troop_menu_node_, 1000); 

    // 背景板 (半透明黑条)
    auto bg = LayerColor::create(Color4B(0, 0, 0, 150), visible_size.width, 180);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5, 0.5));
    bg->setPosition(Vec2::ZERO);
    bg->setName("TroopsInfo");  // 设置tag，用于点击防触
    troop_menu_node_->addChild(bg);

    // 2. 创建选中时的高亮框 （初始为隐藏状态）
    selection_highlight_ = Sprite::create(); // 创建高亮框精灵
    selection_highlight_->setTextureRect(Rect(0, 0, 90, 110));
    selection_highlight_->setColor(Color3B::YELLOW);  // 黄色表示高亮
    selection_highlight_->setOpacity(100);

    selection_highlight_->setVisible(false);
    troop_menu_node_->addChild(selection_highlight_, 0); // 层级在按钮下面，背景上面

    // 3. 动态创建按钮的辅助函数 
    auto create_btn = [&](MainVillage::TroopInfo info, int index)->Node* {

        // 获取玩家拥有的该兵种数量
        int count = 0;
        if (battle_troops_.find(info.name) != battle_troops_.end()) {
            count = battle_troops_[info.name];
        }

        // 如果数量为0，选择变灰显示
        bool has_troops = (count > 0);

        // a. 设置按钮容器大小
        auto container = Node::create();
        container->setContentSize(Size(80, 100)); // 80*100的大小

        // b. 创建按钮可点区域大小
        auto icon_wrapper = Node::create();        // 创建Wrapper
        icon_wrapper->setContentSize(Size(70, 70)); // 大小略小于容器，大于按钮图片
        icon_wrapper->setAnchorPoint(Vec2(0.5, 0.5));

        // c. 创建图片并放缩
        auto sprite = Sprite::create(info.img);  // 根据士兵img表示照片来创建精灵

        float s = 70.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
        sprite->setScale(s);
        sprite->setPosition(35, 35);
        if (!has_troops) sprite->setColor(Color3B::GRAY); // 士兵数量为0 则变灰
        icon_wrapper->addChild(sprite);


        // d. 设置按钮逻辑 
        auto btn = MenuItemSprite::create(icon_wrapper, nullptr, [=](Ref* sender) {
            // 点击回调
            int current_count = battle_troops_[info.name]; // 获取当前最新士兵数量
            if (current_count > 0) { // 只有有兵才能选中

                PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

                current_selected_troop_ = info.name; // 设置鼠标选兵

                // 移动高亮框到当前按钮位置
                selection_highlight_->setVisible(true);
                float btn_x = (index - (MainVillage::troops_.size() - 1) / 2.0f) * 100; 
                selection_highlight_->setPosition(btn_x + 40, 10);

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
        btn->setEnabled(has_troops); // 没兵的时候 点击无效

        // e. 数量标签 
        auto count_lbl = Label::createWithSystemFont(StringUtils::format("x%d", count), "Arial", 20);
        count_lbl->setColor(has_troops ? Color3B::GREEN : Color3B::RED); // 有兵为绿 没兵为红
        count_lbl->setPosition(40, -30); // 图标下方
        container->addChild(count_lbl);

        // 将士兵名称存起来 用来更新对应的数量UI
        troop_count_labels_[info.name] = count_lbl;

        // f. 组装菜单 
        auto menu = Menu::create(btn, nullptr);
        menu->setPosition(Vec2::ZERO);
        container->addChild(menu);

        return container;
        };

    // 4. 排列按钮
    float start_x = -((MainVillage::troops_.size() - 1) * 100.0f) / 2.0f; // 居中排列
    for (int i = 0; i < MainVillage::troops_.size(); ++i) {  // 用troop的size将所有士兵创建出来
        auto item_node = create_btn(MainVillage::troops_[i], i);
        item_node->setPosition(start_x + i * 100, 10);
        troop_menu_node_->addChild(item_node);
    }
}

// 更新士兵数量label
void GameMap::UpdateTroopCountUi(std::string name) {
    // 1. 获取最新数量
    int current_count = 0;
    // 这里用的是临时复制的数据
    if (battle_troops_.find(name) != battle_troops_.end()) {
        current_count = battle_troops_[name];
    }

    // 2. 更新 Label
    if (troop_count_labels_.count(name)) {
        auto lbl = troop_count_labels_[name];
        lbl->setString(StringUtils::format("x%d", current_count));

        if (current_count <= 0) {
            lbl->setColor(Color3B::RED); // 图片变为红色
            // 如果当前选中的兵种用光了，取消选中状态
            if (current_selected_troop_ == name) {
                current_selected_troop_ = "";  // 名字设为空
                selection_highlight_->setVisible(false); // 退出鼠标选兵
            }
        }
    }
}

// 判断游戏结束 
void GameMap::CheckGameState() {

    // ====================== 1. 检测胜利 (除了墙以外的建筑全被摧毁) =======================
    bool has_living_building = false; // 表示是否还有存活建筑

    for (auto building : _buildings) {
        // 排除空指针
        if (!building) continue;

        // 如果是围墙，不检测 跳过
        if (building->getUnitName() == "Fence") continue;

        // 如果不是围墙 检查是否存活 
        if (building->getCurrentHP() > 0) {
            has_living_building = true; // 存活置为true
            break; // 只要还有一个活着的，就不算赢，直接退出循环
        }
    }

    if (!has_living_building) {
        // 场上没有活着的关键建筑 说明胜利
        is_game_over_ = true;
        CCLOG("============= VICTORY! =============");

        // 更新对应的关卡状态
        int level_id = 0;
        std::string number_str = "";
        // 获取对应的关卡ID
        for (char c : current_map_file_name_) {
            if (isdigit(c)) {
                number_str += c;
            }
        }
        if (!number_str.empty()) {
            level_id = std::stoi(number_str);
        }
        else {
            // 解析失败，默认设为 1
            level_id = 1;
        }
        PlayerData::GetInstance()->SetLevelStatus(level_id, true);// 更新状态为win

        CCLOG("关卡 %d 胜利！获得 3 星！",level_id);

         // 展示胜利弹窗 
        this->ShowGameOverLayer(true); // true表示胜利
        return; // 赢了就不用检查是否失败 直接返回
    }

    // ========================= 2. 检测失败 (没库存兵 且 场上兵全死) ========================

    // Step A: 检查库存 (是否还有没放出来的兵)
    bool has_reserves = false;
    // 遍历 map
    for (auto const& pair : battle_troops_) {
        // pair.first  是 name
        // pair.second 是 count
        int count = pair.second; // 获取数量
        if (count > 0) {
            has_reserves = true; // 还有库存
            break;
        }
    }
    // 如果还有库存，说明还没输，直接返回
    if (has_reserves) return;

    // Step B: 检查场上 (是否还有活着的兵)
    bool has_active_soldiers = false;
    for (auto soldier : _soldiers) {
        if (soldier && soldier->getCurrentHP() > 0) {
            has_active_soldiers = true;   // 还有存活的兵
            break;
        }
    }

    // 如果没库存 且 场上没兵 说明失败
    if (!has_active_soldiers) {
        is_game_over_ = true;
        CCLOG("============= DEFEAT... =============");

        // 展示失败弹窗
        this->ShowGameOverLayer(false); // false表示失败
    }
}

// 游戏结束UI弹窗
void GameMap::ShowGameOverLayer(bool is_win) {
    auto visible_size = Director::getInstance()->getVisibleSize();

    // =================== 1. 播放结算bgm ====================
    if (is_win) {
        PlayerData::GetInstance()->PlayBgm("Audio/Battle Win.mp3", false);
    }
    else {
        PlayerData::GetInstance()->PlayBgm("Audio/Battle Lost.mp3", false);  // false 不循环
    }

    // ===================== 2. 半透明遮罩 ================== 
    auto layer = LayerColor::create(Color4B(0, 0, 0, 0), visible_size.width, visible_size.height);
    this->addChild(layer, 20000);

    // 触摸拦截 
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, layer);

    // ==================== 3. 创建内容容器 =================
    auto container = Node::create();
    container->setPosition(visible_size.width / 2, visible_size.height / 2);
    container->setScale(0.0f);
    layer->addChild(container);

    // 添加弹窗底板 
    std::string bg_path = is_win ? "Icon/panel_win.png" : "Icon/panel_lose.png"; // 胜利和失败 分别用 金色和灰色
    auto panel_bg = cocos2d::ui::Scale9Sprite::create(bg_path); // 根据路径创建

    // 设置大小 (宽500, 高400)
    panel_bg->setContentSize(Size(500, 400));
    panel_bg->setPosition(0, 0); // 居中
    container->addChild(panel_bg); // 加到 container

    std::string title_str = is_win ? "VICTORY!" : "DEFEAT";  // 设立标题文字
    Color3B title_color = is_win ? Color3B(255, 220, 0) : Color3B(200, 50, 50);

    // 创建标题label
    auto lbl_title = Label::createWithTTF(title_str, "fonts/GROBOLD.ttf", 80);
    lbl_title->setColor(title_color);
    lbl_title->enableOutline(Color4B::BLACK, 6);
    lbl_title->enableShadow(Color4B::BLACK, Size(2, -2), 0);
    lbl_title->setPosition(0, 120); // 放在面板上方
    container->addChild(lbl_title);

    // =================== 4. 星星评级 ==================
    if (is_win) { // 胜利用金星
        int stars = 3;  // 默认为三颗星
        for (int i = 0; i < 3; i++) {
            // 背景灰星
            auto star_bg = Sprite::create("Icon/star_gray.png");
            if (star_bg) {
                // 排列: -100, 0, 100 保持间距
                star_bg->setPosition((i - 1) * 100, 10);
                container->addChild(star_bg);
            }

            if (i < stars) {
                auto star_gold = Sprite::create("Icon/star_gold.png");   // 金星
                if (star_gold) {
                    star_gold->setPosition((i - 1) * 100, 10);
                    star_gold->setScale(0);
                    container->addChild(star_gold);

                    // 弹射出现
                    auto seq = Sequence::create(
                        DelayTime::create(0.5f + i * 0.3f),
                        EaseBackOut::create(ScaleTo::create(0.3f, 1.3f)), //放大到1.3
                        ScaleTo::create(0.1f, 1.0f),
                        nullptr
                    );
                    star_gold->runAction(seq);
                }
            }
        }
    }
    else { // 失败场景用灰星
        int stars = 3;
        for (int i = 0; i < 3; i++) {
            auto star_bg = Sprite::create("Icon/star_gray.png"); // 背景灰星
            if (star_bg) {
                // 排列: -100, 0, 100 
                star_bg->setPosition((i - 1) * 100, 10);
                container->addChild(star_bg);
            }
            if (i < stars) {

                auto star_gold = Sprite::create("Icon/star_gray.png"); // 灰星
                if (star_gold) {
                    star_gold->setPosition((i - 1) * 100, 10);
                    star_gold->setScale(0);
                    container->addChild(star_gold);
                    // 弹出动画
                    auto seq = Sequence::create(
                        DelayTime::create(0.5f + i * 0.3f),
                        EaseBackOut::create(ScaleTo::create(0.3f, 1.3f)), 
                        ScaleTo::create(0.1f, 1.0f),
                        nullptr
                    );
                    star_gold->runAction(seq);
                }
            }
        }
    }
    // ====================== 5. 返回按钮 ======================
    // a. 设置按钮容器大小
    auto home_wrapper = Node::create();
    home_wrapper->setContentSize(Size(200, 80)); // 长方形按钮区域
    home_wrapper->setAnchorPoint(Vec2(0.5, 0.5));

    // b. 设置按钮图片
    auto btn_sprite = Sprite::create("Icon/Return_btn.png");    
    btn_sprite->setPosition(100, 40); // 居中
    home_wrapper->addChild(btn_sprite);

    // c. 按钮文字label
    auto lbl_btn = Label::createWithTTF("RETURN HOME", "fonts/GROBOLD.ttf", 22);
    lbl_btn->enableOutline(Color4B::BLACK, 2);
    lbl_btn->enableShadow(Color4B::BLACK, Size(2, -2), 0);
    lbl_btn->setPosition(100, 40);
    home_wrapper->addChild(lbl_btn);

    // d. 设置按钮逻辑
    auto btn_item = MenuItemSprite::create(home_wrapper, nullptr, [=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3"); // 播放选择音效

        auto homeScene = MainVillage::createScene(); // 创建大本营
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, homeScene)); // 1s延迟，返回大本营
        });

    // e. 按钮动画
    btn_item->runAction(RepeatForever::create(Sequence::create(
        ScaleTo::create(0.8f, 1.05f),
        ScaleTo::create(0.8f, 1.0f),
        nullptr
    )));

    auto menu = Menu::create(btn_item, nullptr);
    menu->setPosition(0, -120);    // 位置改为负数，放在面板底部
    container->addChild(menu);

    // ====================== 6. 入场动画 ======================
    layer->runAction(FadeTo::create(0.5f, 200));

    auto pop_up_seq = Sequence::create(
        DelayTime::create(0.3f),
        EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)),
        nullptr
    );
    container->runAction(pop_up_seq);
}

// 设置界面显示
void GameMap::ShowSettingsLayer() {
    if (settings_layer_) return;    // 防止重复打开

    // ===================== 1. 暂停游戏逻辑 =====================
    this->is_game_paused_ = true;

    auto visible_size = Director::getInstance()->getVisibleSize();

    // ===================== 2. 创建遮罩层 ===================== 
    settings_layer_ = LayerColor::create(Color4B(0, 0, 0, 180), visible_size.width, visible_size.height);
    settings_layer_->setName("SettingsLayer"); // Tag命名 用于点击检测

    // 触摸拦截
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, settings_layer_);
    this->addChild(settings_layer_, 20000); // 最顶层

    // ==================== 3. 设置背景板 =======================
    std::string bg_path = "Icon/setting_panel.png";
    auto bg = ui::Scale9Sprite::create(bg_path);

    // 设置面板大小 
    bg->setContentSize(Size(450, 350)); // (宽450, 高350)
    bg->setPosition(visible_size.width / 2, visible_size.height / 2); // 位于中心位置
    bg->setName("SettingsBackground"); // 用于点击检测

    settings_layer_->addChild(bg);

    // ==================== 4. 标题 ======================
    auto lbl_title = Label::createWithTTF("SETTINGS", "fonts/GROBOLD.ttf", 36);
    lbl_title->enableOutline(Color4B::BLACK, 3); // 描边加粗
    lbl_title->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 40);
    bg->addChild(lbl_title);

    // ================== 5. 音量（Music和Effect）按钮 ===============
    /**
     * @brief 动态创建一行音量控制组件
     * @param title    标题文字 (如 "Music", "Effect")
     * @param iconPath 左侧图标路径 (如 "icon_music.png")
     * @param posY     在背景板上的 Y 轴坐标
     * @param get_val   获取当前音量的回调 (返回 0.0 ~ 1.0)
     * @param set_val   设置音量的回调 (传入 0.0 ~ 1.0)
     * @return Vector<MenuItem*> 返回生成的加减按钮，以便添加到 Menu 中
     */
    auto create_volume_control = [&](std::string title, std::string iconPath, float posY, std::function<float()> get_val, std::function<void(float)> set_val) {
        // 1. 根据iconPath来创建小喇叭图标 
        auto icon = Sprite::create(iconPath);

        if (!icon) {
            // 防止找不到图
            auto draw = DrawNode::create();
            draw->drawSolidCircle(Vec2::ZERO, 10, 0, 10, Color4F::YELLOW);
            icon = Sprite::create();
            icon->addChild(draw);
        }
        else {
            // 缩放图标到合适大小 
            float target_size = 30.0f;
            float max_side = std::max(icon->getContentSize().width, icon->getContentSize().height);
            if (max_side > 0) icon->setScale(target_size / max_side);
        }
        // 位置：放在最左边位置
        icon->setPosition(50, posY);
        bg->addChild(icon);

        // 2. 标题
        auto lbl = Label::createWithTTF(title, "fonts/GROBOLD.ttf", 26);
        lbl->setAnchorPoint(Vec2(0, 0.5));
        lbl->enableOutline(Color4B::BLACK, 2); 
        lbl->setPosition(80, posY); // 偏左位置
        bg->addChild(lbl);

        // 3. 音量格子容器
        auto bar_node = Node::create();
        bar_node->setPosition(200, posY);
        bg->addChild(bar_node);

        // 刷新格子的辅助函数
        auto refresh_bar = [=](float percent) {
            bar_node->removeAllChildren();
            int level = static_cast<int>(percent * 10 + 0.5f); // 确定目前的音量格子数 +0.5是为了保证int强制转型避免截断

            for (int i = 0; i < 10; i++) { // 创建十个音量块
                auto block = Sprite::create();
                block->setTextureRect(Rect(0, 0, 15, 20));
                block->setColor(i < level ? Color3B(0, 255, 0) : Color3B(50, 50, 50)); // 小于level的部分亮色用绿色，大于的部分暗色用深灰
                block->setPosition(i * 18, 0);
                bar_node->addChild(block);
            }
            };

        // 初始刷新
        refresh_bar(get_val());

        // 4. 减号按钮 [-]
        auto lbl_minus = Label::createWithTTF("-", "fonts/GROBOLD.ttf", 45); 
        lbl_minus->enableOutline(Color4B::BLACK, 3);
        auto btn_minus = MenuItemLabel::create(lbl_minus, [=](Ref*) {
            float v = get_val();
            int level = static_cast<int>(v * 10 + 0.5f); // 同理现在音量格数
            if (level > 0) {
                level--;  // 音量-1
                set_val(level / 10.0f); // 调用设置音量函数
                refresh_bar(level / 10.0f); // 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
            }
            });
        btn_minus->setPosition(170, posY);

        // 5. 加号按钮 [+]
        auto lbl_plus = Label::createWithTTF("+", "fonts/GROBOLD.ttf", 45);
        lbl_plus->enableOutline(Color4B::BLACK, 3);
        auto btn_plus = MenuItemLabel::create(lbl_plus, [=](Ref*) {
            float v = get_val();
            int level = static_cast<int>(v * 10 + 0.5f);// 同理现在音量格数
            if (level < 10) {
                level++;// 音量+1
                set_val(level / 10.0f);// 调用设置音量函数
                refresh_bar(level / 10.0f);// 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
            }
            });
        btn_plus->setPosition(190 + 180 + 20, posY);

        // 返回菜单项，以便添加到主菜单
        return Vector<MenuItem*>{btn_minus, btn_plus};
        };

    // 使用上面的通用函数，创建两排控制器
    // 1. 音乐控制 (Music) - 放在 Y=220
    auto music_items = create_volume_control("Music", "Icon/icon_music.png", 220,
        []() { return PlayerData::GetInstance()->music_volume_; }, // 获取
        [](float v) { PlayerData::GetInstance()->SetMusicVol(v); } // 设置
    );

    // 2. 音效控制 (Effect) - 放在 Y=160
    auto effect_items = create_volume_control("Effect", "Icon/icon_effect.png", 160,
        []() { return PlayerData::GetInstance()->effect_volume_; }, // 获取
        [](float v) { PlayerData::GetInstance()->SetEffectVol(v); } // 设置
    );

    // 把所有按钮加到一个 Menu 里
    auto vol_menu = Menu::create();
    for (auto item : music_items) vol_menu->addChild(item); // 音乐按钮
    for (auto item : effect_items) vol_menu->addChild(item);// 音效按钮

    vol_menu->setPosition(Vec2::ZERO);
    bg->addChild(vol_menu);

    // ===================== 6. 继续游戏 (Resume) ======================
    auto btn_resume_label = Label::createWithTTF("Resume", "fonts/GROBOLD.ttf", 30);
    btn_resume_label->enableOutline(Color4B::BLACK, 2);
    // 按钮回调处理
    auto btn_resume = MenuItemLabel::create(btn_resume_label, [=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        this->is_game_paused_ = false; // 取消暂停 回复逻辑
        if (settings_layer_) {  // 如果打开设置菜单
            settings_layer_->removeFromParent(); // 关闭设置菜单
            settings_layer_ = nullptr; // 指针置空
        }
        });

    // 放在底部偏上
    btn_resume->setPosition(Vec2(bg->getContentSize().width / 2, 110));

    // =================== 7. 退出战斗 (End Battle) ==================== 
    // a. 创建退出战斗按钮容器
    auto quit_wrapper = Node::create();
    quit_wrapper->setContentSize(Size(160, 60)); // 设定点击区域大小
    quit_wrapper->setAnchorPoint(Vec2(0.5, 0.5));

    // b. 创建按钮图片并放缩大小
    auto quit_sprite = Sprite::create("Icon/End_Battle.png");

    // 缩放适应容器 (宽140, 高50)
    float q_scale_x = 140.0f / std::max(1.0f, quit_sprite->getContentSize().width);
    float q_scale_y = 50.0f / std::max(1.0f, quit_sprite->getContentSize().height);

    quit_sprite->setScale(q_scale_x, q_scale_y);

    quit_sprite->setPosition(80, 30); // 居中偏下位置
    quit_wrapper->addChild(quit_sprite);

    // c. 按钮回调逻辑
    auto btn_quit = MenuItemSprite::create(quit_wrapper, nullptr, [=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        // 直接回大本营
        auto home_scene = MainVillage::createScene();  // 创建mainvillage的scene
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, home_scene)); // 切换场景
        });

    // d. 位置放置最下处
    btn_quit->setPosition(Vec2(bg->getContentSize().width / 2, 50));

    // ================ 8. 组装btnResume和btnQuit菜单 =================
    auto bottom_menu = Menu::create(btn_resume, btn_quit, nullptr);
    bottom_menu->setPosition(Vec2::ZERO);
    bg->addChild(bottom_menu);

    // ==================== 9. 弹出入场动画 ====================
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}