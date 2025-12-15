#include "MainVillage.h"
#include "SimpleAudioEngine.h"
#include "Barbarian.h"   
#include "Archer.h"
#include "Giant.h"
#include "WallBreaker.h" 
#include "GameMap.h"


USING_NS_CC;
Scene* MainVillage::createScene()
{
	return MainVillage::create();
}
bool MainVillage::isTileBlock(Vec2 tileCoord)
{
    // 1、获取地图图层 
	//检查背景层
    auto layer = _MainVillageMap->getLayer("MainBackGround"); 
    if (!layer) return false;
	// 检查前景层
    auto layer1 = _MainVillageMap->getLayer("MainForeGround");
    if (!layer1) return false;

    // 2、获取该网格坐标下的 GID (全局图块ID)
    int gid = layer->getTileGIDAt(tileCoord);
	//前景层
    int gid1 = layer1->getTileGIDAt(tileCoord);
    // 如果 GID 为 0，说明这地方是空的（没有图块），可通行
    if (gid == 0 && gid1 == 0) return false;

    // 3、查询该 GID 的属性
    Value properties = _MainVillageMap->getPropertiesForGID(gid);
    //前景层
    Value properties1 = _MainVillageMap->getPropertiesForGID(gid1);

    // 4、检查Block属性
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

    return false; // 默认没有障碍
}
bool MainVillage::init()
{
    if (!Scene::init())
    {
        return false;
    }
	_isDragging = false;// 初始化未在拖拽状态
	_isClickValid = false;// 初始化点击无效
    _selectedSpritePath = "R-C.jpg";//默认选择图片
    //_selectedBuildingType = BuildingType::GOLD_MINE;
    _selectedBuildingType = BuildingType::NONE;

	// ================  设置初始化地图  ======================
    // 
	//获取屏幕可见大小和原点位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    Director::getInstance()->getTextureCache()->removeAllTextures();

	// 1、加载 TMX 地图
     _MainVillageMap = TMXTiledMap::create("MainVillage1.tmx");

    Size mapSize = _MainVillageMap->getMapSize(); // 图块数量
    Size tileSize = _MainVillageMap->getTileSize(); // 单个图块像素 

    // 2、计算地图实际像素宽度和高度
    float mapPixelWidth = mapSize.width * tileSize.width;
    float mapPixelHeight = mapSize.height * tileSize.height;

    // 3、计算缩放因子
    float scaleX = visibleSize.width / mapPixelWidth;
    float scaleY = visibleSize.height / mapPixelHeight;

    // 4、设置缩放
    _MainVillageMap->setScaleX(scaleX);
    _MainVillageMap->setScaleY(scaleY);

    createBuildUI();

    // ====================最左上角建筑主按钮========================
    auto itemBuild = MenuItemImage::create(
        "itemBuild.png", 
        "itemBuild.png", 
        [=](Ref* sender) {
            // 点击逻辑保持不变
            if (_buildMenuNode) {
                bool isVisible = _buildMenuNode->isVisible();
                _buildMenuNode->setVisible(!isVisible);

                if (!_buildMenuNode->isVisible()) {
                    this->closeBuildingMenu();
                }
            }
        }
    );
    itemBuild->setScale(0.12f);
    auto mainMenu = Menu::create(itemBuild, nullptr);
    mainMenu->setPosition(Vec2(75, visibleSize.height - 70)); // 左上角
    this->addChild(mainMenu, 1000); // 加到 this，层级高

    // ====================最左下角战斗主按钮========================
    this->createAttackUI();

    // ==================== 恢复之前的存档 ========================
    this->restoreVillageData();

	//=============================================================
    auto mouseListener = EventListenerMouse::create();

    // 监听器用来监听刷新最大资源量信号
    auto refreshListener = EventListenerCustom::create("REFRESH_MAX_CAPACITY", [=](EventCustom* event) {
        CCLOG("收到刷新指令，开始统计全局容量...");
        this->refreshTotalCapacity();
        });

    _eventDispatcher->addEventListenerWithSceneGraphPriority(refreshListener, this);

	// ==================== 鼠标操作  ==================
   
    // =================  一、滚轮缩放  ==================
    mouseListener->onMouseScroll = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        auto map = _MainVillageMap;
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
        if (_activeMenuNode ) {
            // --- 情况 A: 建筑操作菜单 ---
            auto activemenu = _activeMenuNode->getChildByName("BuildingMenu");
            if (activemenu) {
                bool isHitAnyButton = false; // 标记：是否点到了任意一个按钮

                for (const auto& child : activemenu->getChildren()) {
                    Vec2 localPos = activemenu->convertToNodeSpace(mousePos);

                    // 只要点中了其中一个
                    if (child->getBoundingBox().containsPoint(localPos)) {
                        isHitAnyButton = true;
                        break; // 找到，停止
                    }
                }
                if (isHitAnyButton) {
                    // 点在按钮上了 -> 拦截事件，但不关闭菜单
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
                else {
                    // 循环结束了，说明所有按钮都没点中 -> 关闭菜单，并拦截事件
                    this->closeBuildingMenu();
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
            }
            // --- 情况 B: 建筑信息弹窗 ---
            auto infoBg = _activeMenuNode->getChildByName("InfoBackground");
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
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    this->closeBuildingMenu();
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
            }
            // --- 情况 C: 兵营士兵窗口 ---
            auto info = _activeMenuNode->getChildByName("TrainPanel");
            if (info) {
                Vec2 localPos = info->convertToNodeSpace(mousePos);

                Size s = info->getContentSize();
                Rect bgRect = Rect(0, 0, s.width, s.height);

                // 判断鼠标是否在背景板范围内
                if (bgRect.containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
                else {
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    this->closeBuildingMenu();
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
            }
        }
        // --- 情况D：建筑选择菜单 ---
        if (_buildMenuNode && _buildMenuNode->isVisible()) {
            auto bg = _buildMenuNode->getChildByName("BuildPanelBG");
            if (bg) {
                Vec2 localPos = bg->convertToNodeSpace(mousePos);
                // 判断鼠标是否在背景板范围内
                if (bg->getBoundingBox().containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
                else {
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    _buildMenuNode->setVisible(false);
                    _isDragging = false;
                    _isClickValid = false;
                    return true;
                }
            }
        }
        return false;
        };

    // ==================  二、鼠标按下  ==================
    mouseListener->onMouseDown = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
		//1、判断是否点在菜单上
		Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());// 获取鼠标点击位置
		Vec2 localPos = mainMenu->getParent()->convertToNodeSpace(mousePos);// 转换到菜单节点空间
		if (mainMenu->getBoundingBox().containsPoint(localPos)) {// 判断是否点在菜单区域内
            _isDragging = false;
            return; // 点在菜单上了，忽略拖拽
        }
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
        auto map = _MainVillageMap;
		if (!map) return;
		//1、判断是否点在菜单上
        Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());
        Vec2 localPos = mainMenu->getParent()->convertToNodeSpace(mousePos);
        if (mainMenu->getBoundingBox().containsPoint(localPos)) {
            return; // 点在菜单上了，忽略拖拽
        }
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

                    // 如果当前没有选中要建造的类型，直接返回，不执行建造逻辑
                    if (_selectedBuildingType == BuildingType::NONE) {
                        _isDragging = false;
                        return;
                    }

                    // 3、边界检查
                    if (tileX >= 0 && tileX < mapSize.width && tileY >= 0 && tileY < mapSize.height) {
						Vec2 targetCoord = Vec2(tileX, tileY);// 目标瓦片坐标
						// 检查该瓦片是否已被占用
                        std::string key = StringUtils::format("%d_%d", tileX, tileY);
                        if (_occupiedTiles.find(key) != _occupiedTiles.end() && _occupiedTiles[key] == true) {
                            CCLOG("Tile is already occupied by another sprite!");
                            _isDragging = false;
                            // 直接返回，不执行后面的放置代码
                            return;
                        }
						// 检查该瓦片是否为障碍物
                        if (isTileBlock(targetCoord)) {
                            _isDragging = false;
                            CCLOG("Blocked! Cannot place item on water or mountain.");                        
                            return;
                        }
                        // 4、创建精灵
                        BaseBuilding* newBuilding = nullptr; 

                        // 判断建筑类型，如果是矿或采集器，创建 ResourceProducer
                        if (_selectedBuildingType == BuildingType::GOLD_MINE ||
                            _selectedBuildingType == BuildingType::ELIXIR_PUMP) {
                            newBuilding = ResourceProducer::create(_selectedBuildingType, 1);
                        }
                        //如果是存储类
                        else if (_selectedBuildingType == BuildingType::GOLD_STORAGE ||
                            _selectedBuildingType == BuildingType::ELIXIR_STORAGE ||
                            _selectedBuildingType == BuildingType::BARRACKS) {
                            // 1. 使用具体的子类指针来创建
                            // auto 会自动推导为 ResourceStorage*
                            ResourceStorage* storageBuilding = ResourceStorage::create(_selectedBuildingType, 1);

                            _storageList.push_back(storageBuilding);
                            newBuilding = storageBuilding;

                            this->refreshTotalCapacity();
                        }
                        //其他建筑类型
                        else {
                                if (_selectedBuildingType == BuildingType::WALL) {
                                    newBuilding = Wall::create(1);
                                }
                                if (_selectedBuildingType == BuildingType::CANNON) {
                                    newBuilding = Cannon::create(1);
                                }
                                if (_selectedBuildingType == BuildingType::ARCHER_TOWER) {
                                    newBuilding = ArcherTower::create(1);
                                }
                                if (_selectedBuildingType == BuildingType::TOWN_HALL) {
                                    newBuilding = TownHall::create(1); 
                                }
                        }
                        if (newBuilding) {

                            // --- 获取当前大本营等级 ---
                            int currentTHLevel = 0;
                            for (auto child : map->getChildren()) {
                                // 因为所有建筑 Tag 都是 999，我们要区分出谁是 TownHall
                                if (child->getTag() == 999) {
                                    // 使用 dynamic_cast 尝试转换
                                    TownHall* th = dynamic_cast<TownHall*>(child);
                                    if (th) {
                                        currentTHLevel = th->level;
                                        break; // 找到了，退出循环
                                    }
                                }
                            }

                            int maxLimit = TownHall::getMaxBuildingCount(_selectedBuildingType, currentTHLevel);

                            // --- 统计当前地图上该类建筑的数量 ---
                            int currentCount = 0;
                            for (auto child : map->getChildren()) {
                                if (child->getTag() == 999) {
                                    BaseBuilding* b = dynamic_cast<BaseBuilding*>(child);
                                    // 统计同类型且未被摧毁的建筑
                                    if (b && b->type == _selectedBuildingType && b->state != BuildingState::DESTROYED) {
                                        currentCount++;
                                    }
                                }
                            }

                            // --- 判断是否超限 ---
                            if (currentCount >= maxLimit) {
                                CCLOG("建造失败：数量已达上限！当前: %d / %d (大本营 Lv.%d)", currentCount, maxLimit, currentTHLevel);
                                _isDragging = false;
                                return; // 拦截，不准建造
                            }

                            // 1. 获取该建筑的造价
                            int costGold = newBuilding->buildCostGold;
                            int costElixir = newBuilding->buildCostElixir;

                            // 2. 尝试扣费
                            if (PlayerData::getInstance()->consumeGold(costGold)&& PlayerData::getInstance()->consumeElixir(costElixir) ){
                                // === 扣费成功，执行建造逻辑 ===

                                // 刷新 UI 显示扣钱后的结果
                                this->updateResourceUI();
                                // 适应瓦片大小
                                Size spriteSize = newBuilding->getContentSize();
                                newBuilding->setScaleX(tileSize.width / spriteSize.width);
                                newBuilding->setScaleY(tileSize.height / spriteSize.height);

                                // 居中位置
                                float finalX = tileX * tileSize.width + tileSize.width / 2;
                                float finalY = (mapSize.height - 1 - tileY) * tileSize.height + tileSize.height / 2;
                                newBuilding->setPosition(Vec2(finalX, finalY));
                                newBuilding->setTag(999);// 设置Tag以便后续识别
                                map->addChild(newBuilding, 10);
                                // 标记该瓦片为已占用
                                std::string key = StringUtils::format("%d_%d", tileX, tileY);
                                _occupiedTiles[key] = true;
                                CCLOG("Sprite placed at tile (%d, %d)", tileX, tileY);
                            }
                            else {
                                // === 钱不够，取消建造 ===

                                CCLOG("资源不足！需要: 金币%d, 圣水%d，但你只有: 金币%d, 圣水%d",
                                    costGold, costElixir,
                                    PlayerData::getInstance()->getGold(),
                                    PlayerData::getInstance()->getElixir());
                                // 如果刚才 push_back 进了 _storageList，现在要把它弹出来
                                if (_selectedBuildingType == BuildingType::GOLD_STORAGE ||
                                    _selectedBuildingType == BuildingType::ELIXIR_STORAGE) {
                                    if (!_storageList.empty() && _storageList.back() == newBuilding) {
                                        _storageList.pop_back(); // 还没建成就不算进容量
                                        this->refreshTotalCapacity(); // 刷新回滚容量
                                    }
                                }
                            }
                        }
                        // 造完一个就退出建造模式
                        if(_selectedBuildingType != BuildingType::WALL)
                            _selectedBuildingType = BuildingType::NONE;
                    }
                }
            }

            // 结束拖拽状态
            _isDragging = false;
        }
		// 3、如果是右键松开
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {

            // 1. 获取点击坐标
            Vec2 nodePos = map->convertToNodeSpace(mousePos);

            // 2. 遍历查找点中了谁
            auto& children = map->getChildren();

            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                Node* child = *it;

                // 3. 筛选 Tag (只看建筑)
                if (child->getTag() == 999) {

                    // 4. 碰撞检测
                    if (child->getBoundingBox().containsPoint(nodePos)) {

                        // 如果是Building类型的话 尝试转为 BaseBuilding
                        BaseBuilding* building = dynamic_cast<BaseBuilding*>(child);

                        if (building) {
                            CCLOG("右键选中了建筑，弹出操作菜单");

                            // 调用菜单显示函数，把 building 传过去
                            this->showBuildingMenu(building);

                            break;
                        }
                    }
                }
            }
        }
        };

    // ==================  四、鼠标移动  ==================
    mouseListener->onMouseMove = [=](Event* event) {
        // 只有当“正在拖拽”状态为 true 时才移动地图
        if (_isDragging) {
            EventMouse* e = (EventMouse*)event;
            auto map = _MainVillageMap;
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

	this->addChild(_MainVillageMap, 0);

    // 创立Icon右上角 UI
    createResourceUI();



    // ==========================================================
    // 键盘调试生成士兵，方便测试攻击功能
    // 按 1: 野蛮人, 2: 弓箭手, 3: 巨人, 4: 炸弹人
    // ==========================================================
    auto debugListener = EventListenerKeyboard::create();
    debugListener->onKeyPressed = [=](EventKeyboard::KeyCode code, Event* event) {

        // 1. 获取生成位置 (鼠标当前在地图内的位置)
        // 注意：这里依赖 _lastMousePos，请确保你在 onMouseMove 里更新了这个变量
        Vec2 spawnPos = _MainVillageMap->convertToNodeSpace(_lastMousePos);

        Soldier* s = nullptr;
        std::string name = "";

        switch (code) {
        case EventKeyboard::KeyCode::KEY_1:
            s = Barbarian::create();
            name = "野蛮人";
            break;
        case EventKeyboard::KeyCode::KEY_2:
            s = Archer::create();
            name = "弓箭手";
            break;
        case EventKeyboard::KeyCode::KEY_3:
            s = Giant::create();
            name = "巨人";
            break;
        case EventKeyboard::KeyCode::KEY_4:
            s = WallBreaker::create();
            name = "炸弹人";
            break;
        }

        if (s) {
            s->setPosition(spawnPos);
            // 动态 ZOrder，保证兵在建筑前方或后方显示正确

            Size tileSize = _MainVillageMap->getTileSize();
            Size soldierSize = s->getContentSize();

            float scaleRatio = (tileSize.height * 0.8f) / soldierSize.height;
            s->setScale(scaleRatio);

            _MainVillageMap->addChild(s, 3000 - (int)spawnPos.y);
            CCLOG("调试生成: %s At (%f, %f)", name.c_str(), spawnPos.x, spawnPos.y);
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(debugListener, this);

    return true;
}
void MainVillage::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

void MainVillage::createResourceUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 金币 UI (右上角)
    auto goldIcon = Sprite::create("Gold.png"); 
    goldIcon->setPosition(visibleSize.width - 150, visibleSize.height - 40);
    this->addChild(goldIcon, 9999);

    _goldLabel = Label::createWithSystemFont("0", "Arial", 24);
    _goldLabel->setAnchorPoint(Vec2(0, 0.5));
    _goldLabel->setPosition(visibleSize.width - 120, visibleSize.height - 40);
    this->addChild(_goldLabel, 9999);

    // 2. 圣水 UI (金币下方)
    auto elixirIcon = Sprite::create("Elixir.png"); 
    elixirIcon->setPosition(visibleSize.width - 150, visibleSize.height - 80);
    this->addChild(elixirIcon, 9999);

    _elixirLabel = Label::createWithSystemFont("0", "Arial", 24);
    _elixirLabel->setAnchorPoint(Vec2(0, 0.5));
    _elixirLabel->setPosition(visibleSize.width - 120, visibleSize.height - 80);
    this->addChild(_elixirLabel, 9999);

    // 3. 人口 UI (圣水下方)
    auto peopleIcon = Sprite::create("People.png");
    peopleIcon->setPosition(visibleSize.width - 150, visibleSize.height - 120);
    this->addChild(peopleIcon, 9999);

    _peopleLabel = Label::createWithSystemFont("0", "Arial", 24);
    _peopleLabel->setAnchorPoint(Vec2(0, 0.5));
    _peopleLabel->setPosition(visibleSize.width - 120, visibleSize.height - 120);
    this->addChild(_peopleLabel, 9999);

    // 3. 初始刷新一次
    updateResourceUI();

    // 4. 注册监听器：当 GameManager 发出 "REFRESH_UI" 信号时，自动刷新文字
    auto listener = EventListenerCustom::create("REFRESH_UI", [=](EventCustom* event) {
        this->updateResourceUI();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainVillage::updateResourceUI() {
    int gold = PlayerData::getInstance()->getGold();
    int elixir = PlayerData::getInstance()->getElixir();
    int people = PlayerData::getInstance()->getPeople();

    if (_goldLabel) _goldLabel->setString(std::to_string(gold));
    if (_elixirLabel) _elixirLabel->setString(std::to_string(elixir));
    if (_peopleLabel) _peopleLabel->setString(std::to_string(people));
}

void MainVillage::refreshTotalCapacity()
{
    long long totalGoldCapacity = 20000000;
    long long totalElixirCapacity = 20000000;
    long long totalPeopleCapacity = 0;

    // 1. 遍历容器中的每一个建筑
    for (auto building : _storageList) {
        // 确保是建筑类
        if (building) {

            // 根据类型累加
            if (building->type == BuildingType::GOLD_STORAGE) {
                totalGoldCapacity += building->maxLimit;
            }
            else if (building->type == BuildingType::ELIXIR_STORAGE) {
                totalElixirCapacity += building->maxLimit;
            }
            else if (building->type == BuildingType::BARRACKS) {
                totalPeopleCapacity += building->maxLimit;
            }
        }
    }
    PlayerData::getInstance()->updateMaxLimits(totalGoldCapacity, totalElixirCapacity, totalPeopleCapacity);
    CCLOG("=== 刷新完成 ===");
    CCLOG("当前金币总上限: %lld", totalGoldCapacity);
    CCLOG("当前圣水总上限: %lld", totalElixirCapacity);
    CCLOG("当前人口总上限: %lld", totalPeopleCapacity);
}

void MainVillage::closeBuildingMenu() {
    if (_activeMenuNode) {
        _activeMenuNode->removeFromParent();
        _activeMenuNode = nullptr;
    }
}

void MainVillage::showBuildingMenu(BaseBuilding* building) {
    //1. 如果还有未关闭的菜单
    if (_activeMenuNode != nullptr) {
        closeBuildingMenu();
    }
    if (!building) return;

    // 2. 创建一个节点作为菜单容器
    _activeMenuNode = Node::create();
    // 把菜单放在建筑的头顶 
    _activeMenuNode->setPosition(building->getPosition() + Vec2(0, 50));

    float mapScaleX = _MainVillageMap->getScaleX();
    float mapScaleY = _MainVillageMap->getScaleY();

    _activeMenuNode->setScaleX(1.0f / mapScaleX);
    _activeMenuNode->setScaleY(1.0f / mapScaleY);
    _activeMenuNode->setTag(888); // 特殊Tag，区别于建筑

    // 加到地图上，设置高Z轴，保证盖在所有建筑上面
    _MainVillageMap->addChild(_activeMenuNode, 10000);

    //---------------按钮1：升级功能----------------
    auto labelUp = Label::createWithSystemFont("Upgrade (Lv+1)", "Arial", 24);

    auto btnUpgrade = MenuItemLabel::create(labelUp, [=](Ref* sender) {
        CCLOG("点击了升级按钮");

        // --- 扣费升级逻辑 ---
        int nextLevel = building->level + 1;

        // 大本营等级限制
        // 1. 寻找当前地图上的大本营等级
        int currentTHLevel = 1;
        for (auto child : _MainVillageMap->getChildren()) {
            if (child->getTag() == 999) {
                auto th = dynamic_cast<TownHall*>(child);
                if (th) {
                    currentTHLevel = th->level;
                    break;
                }
            }
        }

        // 2. 检查是否允许升级
        if (!TownHall::isUpgradeAllowed(building->type, nextLevel, currentTHLevel)) {
            CCLOG("升级失败：需要大本营 Lv.%d 才能升级到 Lv.%d", currentTHLevel + 1, nextLevel);
            // 这里最好弹出一个 Label 提示玩家
            return; // 直接拦截，不扣钱
        }

        BuildingStats nextStats = BaseBuilding::getStatsConfig(building->type, nextLevel);//获取升级花费

        int upgradeCostGold = nextStats.costGold; 
        int upgradeCostElixir = nextStats.costElixir;
        if (PlayerData::getInstance()->consumeGold(upgradeCostGold)&& PlayerData::getInstance()->consumeElixir(upgradeCostElixir)) {
            // 扣钱成功，执行升级
            building->upgradeLevel();
            this->updateResourceUI(); // 刷新右上角钱数
            
            //关闭菜单
            this->closeBuildingMenu();
        }
        else {
            CCLOG("金币不足，无法升级！");
        }
        });

    //-------------按钮2：删除功能--------------
    auto labelRem = Label::createWithSystemFont("Remove", "Arial", 24);

    auto btnRemove = MenuItemLabel::create(labelRem, [=](Ref* sender) {
        CCLOG("点击了拆除按钮");

        // 1. 如果是存储建筑
        if (building->type == BuildingType::GOLD_STORAGE ||
            building->type == BuildingType::ELIXIR_STORAGE) {

            auto& list = this->_storageList;
            auto it = std::find(list.begin(), list.end(), building);
            if (it != list.end()) {
                list.erase(it);
                this->refreshTotalCapacity(); // 刷新上限
                CCLOG("Storage removed from list.");
            }
        }

        // 2. 释放地图占用 
        Size tileSize = _MainVillageMap->getTileSize();
        Size mapSize = _MainVillageMap->getMapSize();

        // 获取建筑当前在地图上的坐标
        Vec2 buildingPos = building->getPosition();

        // 计算网格坐标
        int tileX = (int)(buildingPos.x / tileSize.width);
        int tileY = (int)(mapSize.height - (buildingPos.y / tileSize.height));

        std::string key = StringUtils::format("%d_%d", tileX, tileY);

        // 将该位置标记为“未占用”
        _occupiedTiles[key] = false;
        CCLOG("Tile (%d, %d) freed.", tileX, tileY);

        // 3. 移除对象
        building->removeFromParent();
        // 4. 关闭菜单
        this->closeBuildingMenu();
        });

    //----------- 按钮3：显示信息 -------------
    auto labelInfo = Label::createWithSystemFont("Info", "Arial", 20);
    auto btnInfo = MenuItemLabel::create(labelInfo, [=](Ref* sender) {

        // 1. 先关闭原来的操作菜单
        this->closeBuildingMenu();

        // 2. 创立一个菜单根节点
        _activeMenuNode = Node::create();
        // 设置反向缩放 
        float mapScaleX = _MainVillageMap->getScaleX();
        float mapScaleY = _MainVillageMap->getScaleY();
        _activeMenuNode->setScaleX(1.0f / mapScaleX);
        _activeMenuNode->setScaleY(1.0f / mapScaleY);

        // 定位到建筑位置
        _activeMenuNode->setPosition(building->getPosition());

        // 加到地图上，层级极高
        _MainVillageMap->addChild(_activeMenuNode, 20000);

        // 3. 创建半透明背景 
        float bgWidth = 300;
        float bgHeight = 200;
        auto bg = LayerColor::create(Color4B(0, 0, 0, 200), bgWidth, bgHeight);
        bg->setName("InfoBackground");//设置菜单署名
        bg->ignoreAnchorPointForPosition(false);
        bg->setAnchorPoint(Vec2(0, 0.5));
        bg->setPosition(Vec2(60, 0));
        _activeMenuNode->addChild(bg);//将背景板添加到指针

        // 4. 显示通用信息 (名字、等级、HP)
        std::string nameStr = building->name;

        std::string commonText = StringUtils::format(
            "Name: %s\nLevel: %d\nHP: %d / %d",
            nameStr.c_str(),
            building->level,
            (int)building->currentHP,
            (int)building->maxHP 
        );

        auto labelCommon = Label::createWithSystemFont(commonText, "Arial", 24);
        labelCommon->setPosition(bgWidth / 2, bgHeight / 2 + 20); // 靠上
        bg->addChild(labelCommon);

        // 5. 显示特殊信息 (根据类型判断)
        std::string specialText = "";
        BuildingStats stats = BaseBuilding::getStatsConfig(building->type, building->level);//获取建筑信息
        if (building->type == BuildingType::GOLD_MINE || building->type == BuildingType::ELIXIR_PUMP) {
            specialText = StringUtils::format("Production: %d / hour", stats.productionRate);
        }
        if (building->type == BuildingType::CANNON || building->type == BuildingType::ARCHER_TOWER) {
            specialText = StringUtils::format("Damage: %.1lf ", stats.damage);
        }
        else if (building->type != BuildingType::WALL && building->type != BuildingType::TOWN_HALL){
            specialText = StringUtils::format("Capacity: %d", stats.capacity);
        }
        
        if (!specialText.empty()) {
            auto labelSpecial = Label::createWithSystemFont(specialText, "Arial", 24);
            labelSpecial->setColor(Color3B::YELLOW); //以此区分
            labelSpecial->setPosition(bgWidth / 2, bgHeight / 2 -40 );
            bg->addChild(labelSpecial);
        }

        // 6. 添加一个关闭按钮
        auto closeLabel = Label::createWithSystemFont("[ Close ]", "Arial", 26);
        auto closeItem = MenuItemLabel::create(closeLabel, [=](Ref* sender) {
            this->closeBuildingMenu();
            });
        closeItem->setPosition(bgWidth - 60, bgHeight - 20);

        auto menu = Menu::create(closeItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        bg->addChild(menu);
        CCLOG("信息展示：");
        });

    // 定义训练按钮指针 (因为不确定是否是军营，使用指针，初始化为空)
    MenuItemLabel* btnTrain = nullptr;

    // ----------- 按钮4：训练士兵 -------------
    if (building->type == BuildingType::BARRACKS) {

        auto labelTrain = Label::createWithSystemFont("Train Troops", "Arial", 24);
        labelTrain->setColor(Color3B(0, 255, 0));

        btnTrain = MenuItemLabel::create(labelTrain, [=](Ref* sender) {

            // 1. 清空当前菜单
            if (_activeMenuNode) {
                _activeMenuNode->removeAllChildren();
            }
            else {
                return;
            }

            // 2. 创建训练面板背景
            float panelW = 400;
            float panelH = 140;
            auto bg = LayerColor::create(Color4B(0, 0, 0, 220), panelW, panelH);
            bg->setName("TrainPanel");
            bg->setIgnoreAnchorPointForPosition(false);
            bg->setAnchorPoint(Vec2(0.5, 1));
            bg->setPosition(Vec2(0, -200)); // 放在建筑下方
            _activeMenuNode->addChild(bg);

            // 3. 定义辅助函数
            auto createTroopNode = [=](TroopInfo info, int index) -> Node* {

                // --- 容器 ---
                auto container = Node::create();
                container->setContentSize(Size(70, 90));

                // --- A. 人物头像大按钮 ---
                auto iconWrapper = Node::create();
                iconWrapper->setContentSize(Size(60, 60)); // 使用Wrapper强行包裹按钮大小
                iconWrapper->setAnchorPoint(Vec2(0.5, 0.5));

                auto sprite = Sprite::create(info.img);

                float s = 60.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
                sprite->setScale(s);
                sprite->setPosition(30, 30); // 放在 Wrapper 的正中心
                iconWrapper->addChild(sprite);

                auto btnAdd = MenuItemSprite::create(iconWrapper, nullptr, [=](Ref*) {
                    // 先判断容量
                    if (PlayerData::getInstance()->consumeElixir(info.cost) && PlayerData::getInstance()->addPeople(info.weight, info.cost)) {
                        // +1 逻辑
                        PlayerData::getInstance()->addTroop(info.name, 1);

                        // 动画 
                        sprite->stopAllActions();
                        sprite->runAction(Sequence::create(ScaleTo::create(0.05f, s * 1.1f), ScaleTo::create(0.05f, s), nullptr));

                        // 刷新 UI
                        auto countLbl = (Label*)container->getChildByTag(101);
                        auto menu = (Menu*)container->getChildByTag(102);
                        if (countLbl) countLbl->setString(StringUtils::format("x%d", PlayerData::getInstance()->getTroopCount(info.name)));
                        if (menu) {
                            auto minItem = menu->getChildByTag(200);
                            if (minItem) minItem->setVisible(true);
                        }
                        CCLOG("增加: %s", info.name.c_str());

                        // 可视化处理
                        Soldier* newSoldier = nullptr;
                        // 根据名字判断创建哪种兵
                        if (info.name == "Barbarian") {
                            newSoldier = Barbarian::create();
                        }
                        else if (info.name == "Archer") {
                            newSoldier = Archer::create();
                        }
                        else if (info.name == "Giant") {
                            newSoldier = Giant::create();
                        }
                        else if (info.name == "WallBreaker") { // 你的炸弹人名字
                            newSoldier = WallBreaker::create();
                        }

                        if (newSoldier) {
                            // 1. 确定“出生点” (兵营大门位置)
                            // 假设兵营图片的“门”在中心点向下 20 像素的位置
                            Vec2 doorPos = building->getPosition() + Vec2(0, -20);

                            // 2. 计算“目标点” (扇形随机分布)
                            // 我们希望士兵站在兵营下方，呈扇形散开
                            // 角度范围：180度(左) 到 360/0度(右)，即下方半圆。
                            // 为了美观，限制在 210度 ~ 330度 之间 (避免站得太靠水平两侧)

                            // 将角度转换为弧度 (Cocos数学库用弧度)
                            float minAngle = CC_DEGREES_TO_RADIANS(210);
                            float maxAngle = CC_DEGREES_TO_RADIANS(330);

                            // 随机角度
                            float randomAngle = minAngle + (maxAngle - minAngle) * CCRANDOM_0_1();

                            // 随机距离 (半径)：距离兵营中心 50 到 90 像素之间
                            float minRadius = 50.0f;
                            float maxRadius = 90.0f;
                            float randomRadius = minRadius + (maxRadius - minRadius) * CCRANDOM_0_1();

                            // 极坐标转笛卡尔坐标：计算偏移量
                            float offsetX = cos(randomAngle) * randomRadius;
                            float offsetY = sin(randomAngle) * randomRadius;

                            // 最终待机位置
                            Vec2 targetPos = building->getPosition() + Vec2(offsetX, offsetY);

                            // 3. 设置初始状态
                            // 先把兵放在门口，且设为极小（刚训练出来）
                            newSoldier->setPosition(doorPos);
                            newSoldier->setScale(0.01f);

                            // 根据目标点在左边还是右边，调整朝向
                            if (targetPos.x < doorPos.x) {
                                // 如果目标在左边，可能需要翻转 
                                newSoldier->setScaleX(-0.01f); // 用负 Scale 实现翻转
                            }

                            // 4. 添加到地图层 (处理遮挡)
                            // Z轴动态计算：Y越小(越靠下)，Z越大(越靠前)
                            _MainVillageMap->addChild(newSoldier, 3000 - (int)doorPos.y);

                            // 5. 播放“出击”动画
                            // 效果：从门里蹦出来，落地变大
                            float jumpHeight = 20.0f;
                            float duration = 0.5f;

                            // 动作A: 跳跃移动到目标点
                            auto jumpAct = JumpTo::create(duration, targetPos, jumpHeight, 1);

                            // 动作B: 变大恢复正常 (根据素材调整缩放)
                            float targetScale = 0.2f; // 你的士兵标准大小
                            if (targetPos.x < doorPos.x) targetScale = -0.2f; // 保持翻转

                            auto scaleAct = ScaleTo::create(duration, fabs(targetScale), fabs(targetScale)); 

                            // 组合动作
                            auto spawnSeq = Sequence::create(
                                Spawn::create(jumpAct, scaleAct, nullptr),
                                CallFunc::create([=]() {
                                    // 落地后，更新 ZOrder 
                                    newSoldier->setLocalZOrder(3000 - (int)targetPos.y);
                                    }),
                                nullptr
                            );

                            newSoldier->runAction(spawnSeq);

                            // 加入到可视化容器里
                            building->_visualTroops[info.name].push_back(newSoldier);

                            CCLOG("士兵实体 %s 扇形生成在 (%f, %f)", info.name.c_str(), targetPos.x, targetPos.y);
                        }
                    }
                    });
                btnAdd->setPosition(35, 60); // 按钮在容器中的位置

                // --- B. 左上角减号按钮 ---
                auto minusWrapper = Node::create();
                minusWrapper->setContentSize(Size(50, 50)); // 增大点击区域
                minusWrapper->setAnchorPoint(Vec2(0.5, 0.5));

                auto minLbl = Label::createWithSystemFont("-", "Arial", 60);
                minLbl->setColor(Color3B::RED);
                minLbl->enableOutline(Color4B::WHITE, 2);
                minLbl->setPosition(15, 15); // 文字居中
                minusWrapper->addChild(minLbl);

                auto btnMinus = MenuItemSprite::create(minusWrapper, nullptr, [=](Ref*) {
                    // -1 逻辑
                    if (PlayerData::getInstance()->consumeTroop(info.name, 1)) {
                        PlayerData::getInstance()->removePeople(info.weight);// 返还人口容量
                        auto countLbl = (Label*)container->getChildByTag(101);
                        if (countLbl) countLbl->setString(StringUtils::format("x%d", PlayerData::getInstance()->getTroopCount(info.name)));

                        // --- 视觉删除逻辑 ---

                        // 1. 找到该兵种的列表
                        // 引用 building 指针，因为 troops 存在 building 里
                        auto& soldierList = building->_visualTroops[info.name];

                        if (!soldierList.empty()) {
                            // 2. 取出最后生成的那个兵 (LIFO: 后进先出)
                            Node* soldierToRemove = soldierList.back();

                            // 3. 从列表中移除 (这样下次就不会再取到它了)
                            soldierList.pop_back();

                            // 4. 执行删除动画
                            if (soldierToRemove) {
                                soldierToRemove->stopAllActions(); // 停止它当前的动作

                                // 创建“完美删除”动画序列
                                auto deleteSeq = Sequence::create(
                                    // 步骤A: 瞬间变红，提示玩家它被删除了
                                    TintTo::create(0.1f, 255, 0, 0),

                                    // 步骤B: 同时缩小和淡出
                                    Spawn::create(
                                        ScaleTo::create(0.3f, 0.0f), // 缩小到没了
                                        FadeOut::create(0.3f)        // 变透明
                                    ),

                                    // 步骤C: 【彻底移除】 从父节点(_MainVillageMap)上删掉
                                    RemoveSelf::create(),
                                    nullptr
                                );

                                soldierToRemove->runAction(deleteSeq);
                            }
                        }

                        if (PlayerData::getInstance()->getTroopCount(info.name) <= 0) {
                            auto menu = (Menu*)container->getChildByTag(102);
                            auto minItem = menu->getChildByTag(200);
                            if (minItem) minItem->setVisible(false);
                        }
                        CCLOG("减少: %s", info.name.c_str());
                    }
                    });
                // 减号位置：稍微往左上角挪一点，避免和头像重叠
                btnMinus->setPosition(10, 85);
                btnMinus->setTag(200);

                // --- C. 组装 Menu ---
                auto localMenu = Menu::create(btnAdd, btnMinus, nullptr);
                localMenu->setPosition(Vec2::ZERO);
                localMenu->setTag(102);
                container->addChild(localMenu);

                // --- D. 标签 (右上角) ---
                int currentCount = PlayerData::getInstance()->getTroopCount(info.name);
                auto countLbl = Label::createWithSystemFont(StringUtils::format("x%d", currentCount), "Arial", 16);
                countLbl->setColor(Color3B::GREEN);
                countLbl->setAnchorPoint(Vec2(1, 0.5));
                countLbl->setPosition(68, 90);
                countLbl->setTag(101);
                container->addChild(countLbl);

                // 初始隐藏减号
                btnMinus->setVisible(currentCount > 0);

                // --- E. 名字与权重 ---
                auto weightLbl = Label::createWithSystemFont(StringUtils::format("%d", info.weight), "Arial", 14);
                weightLbl->setColor(Color3B::YELLOW);
                weightLbl->setAnchorPoint(Vec2(1, 0));
                weightLbl->setPosition(65, 10);
                container->addChild(weightLbl);

                auto nameLbl = Label::createWithSystemFont(info.name, "Arial", 14);
                nameLbl->setPosition(35, 5);
                container->addChild(nameLbl);

                // --- F. 消耗圣水值 ---
                auto costLbl = Label::createWithSystemFont(StringUtils::format("%d", info.cost), "Arial", 14);
                costLbl->setColor(Color3B::GREEN);
                costLbl->setAnchorPoint(Vec2(1, 0));
                costLbl->setPosition(45, -20);
                container->addChild(costLbl);

                return container;
                };

            // 5. 循环创建并排列 
            float startX = 30;
            float gapX = 90;
            float posY = panelH / 2 - 40;

            for (int i = 0; i < troops.size(); ++i) {
                // 调用上面的 lambda 创建
                auto troopNode = createTroopNode(troops[i], i);
                troopNode->setPosition(startX + i * gapX, posY);
                bg->addChild(troopNode);
            }

            // 7. 添加关闭按钮 (X)
            auto closeLbl = Label::createWithSystemFont("X", "Arial", 26);
            closeLbl->setColor(Color3B::RED);
            auto closeItem = MenuItemLabel::create(closeLbl, [=](Ref*) {
                this->closeBuildingMenu();
                });
            closeItem->setPosition(panelW - 20, panelH - 20);

            auto closeMenu = Menu::create(closeItem, nullptr);
            closeMenu->setPosition(Vec2::ZERO);
            bg->addChild(closeMenu);

            CCLOG("打开了兵营训练面板");
            });
    }

    // ----------- 组装最终的主菜单 (使用 Vector 动态添加) -------------
    Vector<MenuItem*> menuItems;
    menuItems.pushBack(btnUpgrade);
    menuItems.pushBack(btnRemove);

    // 如果是兵营，就把训练按钮加进去
    if (btnTrain != nullptr) {
        menuItems.pushBack(btnTrain);
    }

    menuItems.pushBack(btnInfo);

    auto menu = Menu::createWithArray(menuItems); // 使用数组创建菜单
    menu->alignItemsVerticallyWithPadding(20);
    menu->setName("BuildingMenu");
    menu->setPosition(Vec2::ZERO);
    _activeMenuNode->addChild(menu);
}

void MainVillage::createBuildUI() {

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建底部面板容器 
    _buildMenuNode = Node::create();
    _buildMenuNode->setVisible(false);
    this->addChild(_buildMenuNode, 1100);

    // A. 半透明黑底
    float panelHeight = 280; 
    auto bg = LayerColor::create(Color4B(0, 0, 0, 220), visibleSize.width, panelHeight);
    bg->setName("BuildPanelBG");
    _buildMenuNode->addChild(bg);

    // B. 创建顶部的分类页签 (资源 | 防御)
    // 1. 先把 Label 和 Item 创建出来 
    auto labelRes = Label::createWithSystemFont("Resources", "Arial", 28);
    auto itemRes = MenuItemLabel::create(labelRes, nullptr);

    auto labelDef = Label::createWithSystemFont("Defense", "Arial", 28);
    auto itemDef = MenuItemLabel::create(labelDef, nullptr);

    // 2. 设置 [资源] 按钮的回调
    itemRes->setCallback([=](Ref*) {
        // 视觉变化：自己变绿，别人变灰
        itemRes->setColor(Color3B::GREEN);
        itemDef->setColor(Color3B::GRAY);

        // 逻辑变化
        this->switchBuildCategory(0);
        });

    // 3. 设置 [防御] 按钮的回调
    itemDef->setCallback([=](Ref*) {
        // 视觉变化：自己变绿，别人变灰
        itemDef->setColor(Color3B::GREEN);
        itemRes->setColor(Color3B::GRAY);

        // 逻辑变化
        this->switchBuildCategory(1);
        });

    // 4. 设置初始状态 
    itemRes->setColor(Color3B::GREEN); //(默认选中资源)
    itemDef->setColor(Color3B::GRAY);  //(默认未选中资源)

    // 5. 放入菜单
    auto tabMenu = Menu::create(itemRes, itemDef, nullptr);
    tabMenu->alignItemsHorizontallyWithPadding(100);
    tabMenu->setPosition(visibleSize.width / 2, panelHeight - 30);
    _buildMenuNode->addChild(tabMenu);

    // C. 创建内容容器 (存放建筑图标)
    _iconContainer = Node::create();
    _iconContainer->setPosition(0, 0); 
    _buildMenuNode->addChild(_iconContainer);

    // 默认加载资源类
    switchBuildCategory(0);
}

void MainVillage::switchBuildCategory(int category) {
    if (!_iconContainer) return;

    // 1. 清空当前显示的图标
    _iconContainer->removeAllChildren();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    float panelHeight = 280;

    // 2. 定义辅助 lambda：创建大图标 
    auto createBtn = [&](std::string name, std::string img, BuildingType type) {
        // 创建图
        auto sprite = Sprite::create(img);
        if (!sprite) sprite = Sprite::create(); 

        // 缩放图片到合适大小 
        float s = 80.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
        sprite->setScale(s);

        // 创建字
        auto lbl = Label::createWithSystemFont(name, "Arial", 18);

        // 组合成 MenuItemSprite
        auto nodeNormal = Node::create();
        nodeNormal->setContentSize(Size(120, 150));
        sprite->setPosition(60, 60);
        lbl->setPosition(60, 130);
        nodeNormal->addChild(sprite);
        nodeNormal->addChild(lbl);

        // 点击回调
        return MenuItemSprite::create(nodeNormal, nodeNormal, [=](Ref*) {
            _selectedBuildingType = type;
            CCLOG("选中建筑: %s", name.c_str());
            _buildMenuNode->setVisible(false); // 选完自动关闭
            });
        };

    // 3. 根据分类创建不同的按钮
    Vector<MenuItem*> items;

    if (category == 0) {
        items.pushBack(createBtn("Tower Hall", "TownHall1.png", BuildingType::TOWN_HALL));
        // === 资源类 ===
        items.pushBack(createBtn("Gold Mine", "Gold_Mine1.png", BuildingType::GOLD_MINE));
        items.pushBack(createBtn("Elixir Pump", "Elixir_Pump1.png", BuildingType::ELIXIR_PUMP));
        items.pushBack(createBtn("Gold Store", "Gold_Storage1.png", BuildingType::GOLD_STORAGE));
        items.pushBack(createBtn("Elixir Store", "Elixir_Storage1.png", BuildingType::ELIXIR_STORAGE));
    }
    else if (category == 1) {
        // === 防御类 ===

        items.pushBack(createBtn("Cannon", "Cannon1.png", BuildingType::CANNON));
        items.pushBack(createBtn("Archer Tower", "ArcherTower1.png", BuildingType::ARCHER_TOWER));
        items.pushBack(createBtn("Wall", "Wall1.png", BuildingType::WALL));
        items.pushBack(createBtn("Barracks", "Barracks1.png", BuildingType::BARRACKS));
    }

    // 4. 创建菜单并添加到容器
    auto menu = Menu::createWithArray(items);
    menu->alignItemsHorizontallyWithPadding(20);

    // 放在面板中间稍微偏下 (因为顶部有页签)
    menu->setPosition(visibleSize.width / 2, (panelHeight - 50) / 2);

    _iconContainer->addChild(menu);
}

// 初始化士兵结构体
std::vector<MainVillage::TroopInfo> MainVillage::troops = {
       {"Barbarian",   "Barbarian_head.png",    1,    25},
       {"Archer",      "Archer_head.png",       1,    30},
       {"Giant",       "Giant_head.png",        5,   250},
       {"WallBreaker", "Wall_Breaker_head.png", 2,   100}
};

// 左下角Attack按钮
void MainVillage::createAttackUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建容器 (Wrapper)
    // 设定按钮的逻辑 点击区域
    float boxSize = 130.0f;
    auto wrapper = Node::create();
    wrapper->setContentSize(Size(boxSize, boxSize));
    wrapper->setAnchorPoint(Vec2(0.5, 0.5)); // 中心锚点

    // 2. 创建并处理图片
    std::string iconPath = "Attack_Button.png";
    auto sprite = Sprite::create(iconPath);

    // --- 防崩处理 ---
    if (!sprite) {
        sprite = Sprite::create();
        sprite->setTextureRect(Rect(0, 0, 80, 80));
        sprite->setColor(Color3B::RED);
        auto lbl = Label::createWithSystemFont("ATK", "Arial", 24);
        lbl->setPosition(40, 40);
        sprite->addChild(lbl);
    }

    // --- 缩放逻辑  ---
    float targetIconSize = 100.0f;

    // 获取图片的最长边 (宽或高)
    float contentMax = std::max(sprite->getContentSize().width, sprite->getContentSize().height);

    // 防止除以0
    if (contentMax <= 0) contentMax = 100.0f;

    // 计算缩放比例：目标大小 / 原始大小
    float scale = 1.5 * targetIconSize / contentMax;

    // 应用均匀缩放 
    sprite->setScale(scale);

    // --- 定位 ---
    // 把图片放在容器的正中心
    sprite->setPosition(boxSize / 2, boxSize / 2);

    // --- 放入容器 ---
    wrapper->addChild(sprite);

    // 3. 创建按钮 (使用 wrapper)
    auto btnAttack = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {

        // 点击反馈：让里面的图片缩一下
        sprite->stopAllActions();
        sprite->runAction(Sequence::create(
            ScaleTo::create(0.1f, scale * 0.9f), // 变小
            ScaleTo::create(0.1f, scale),        // 弹回
            nullptr
        ));

        // 点击逻辑
        this->saveVillageData();   // 存档
        this->showLevelSelection();// 选择关卡
        });

    // 4. 设置位置和整体动画
    // 位置：距离左下角 (80, 80)
    btnAttack->setPosition(Vec2(80, 80));

    // 呼吸动画：对整个按钮(wrapper)进行缩放
    btnAttack->runAction(RepeatForever::create(Sequence::create(
        ScaleTo::create(0.8f, 1.1f), // 放大到 1.1 倍
        ScaleTo::create(0.8f, 1.0f), // 恢复 1.0 倍
        nullptr
    )));

    // 5. 添加到菜单
    auto menu = Menu::create(btnAttack, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1000);
}

// 选择关卡弹窗
void MainVillage::showLevelSelection() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建遮罩背景 (防止点击后面的建筑)
    // 使用 LayerColor 创建半透明黑色
    auto layer = LayerColor::create(Color4B(0, 0, 0, 200), visibleSize.width, visibleSize.height);

    // 拦截点击事件 (吞噬触摸)
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, layer);

    this->addChild(layer, 20000); // 最高层级

    // 2. 创建关卡按钮

    // --- 关卡 1 按钮 ---
    auto lblMap1 = Label::createWithSystemFont("Level 1: Goblin Forest", "Arial", 30);
    auto btnMap1 = MenuItemLabel::create(lblMap1, [=](Ref*) {
        CCLOG("进入关卡 1...");
        // 核心跳转逻辑
        // 切换场景到 GameMap，传入地图路径
        auto scene = GameMap::create("map/map1.tmx");
        // 使用过渡动画切换 (淡入淡出)
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
        });

    // --- 关卡 2 按钮 ---
    auto lblMap2 = Label::createWithSystemFont("Level 2: Desert Fortress", "Arial", 30);
    auto btnMap2 = MenuItemLabel::create(lblMap2, [=](Ref*) {
        CCLOG("进入关卡 2...");
        auto scene = GameMap::create("map/map2.tmx");
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
        });

    // --- 关闭/返回 按钮 ---
    auto lblClose = Label::createWithSystemFont("[ Cancel ]", "Arial", 26);
    lblClose->setColor(Color3B::YELLOW);
    auto btnClose = MenuItemLabel::create(lblClose, [=](Ref*) {
        // 移除遮罩层，回到大本营界面
        layer->removeFromParent();
        });

    // 3. 排列菜单
    auto menu = Menu::create(btnMap1, btnMap2, btnClose, nullptr);
    menu->alignItemsVerticallyWithPadding(40);
    menu->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    layer->addChild(menu);
}

// 存档操作
void MainVillage::saveVillageData() {
    auto dataCenter = PlayerData::getInstance();

    // 1. 清空旧存档
    dataCenter->_villageLayout.clear();

    // 2. 遍历地图子节点
    auto& children = _MainVillageMap->getChildren();

    Size tileSize = _MainVillageMap->getTileSize();
    Size mapSize = _MainVillageMap->getMapSize();

    for (const auto& child : children) {
        // 转换类型
        auto building = dynamic_cast<BaseBuilding*>(child);

        // 排除无效建筑和预览虚影
        if (building && building->state != BuildingState::PREVIEW && building->state != BuildingState::DESTROYED) {

            BuildingData data;
            data.type = building->type;
            data.level = building->level;

            // --- 根据位置反算网格坐标 ---
            // X 轴:
            int tX = (int)(building->getPositionX() / tileSize.width);

            // Y 轴:
            int tY = (int)mapSize.height - 1 - (int)(building->getPositionY() / tileSize.height);

            data.tileX = tX;
            data.tileY = tY;

            // 存入单例
            dataCenter->_villageLayout.push_back(data);
        }
    }

    CCLOG("存档完成：保存了 %d 个单格建筑", (int)dataCenter->_villageLayout.size());
}

// 读档操作
void MainVillage::restoreVillageData() {
    auto dataCenter = PlayerData::getInstance();
    Size tileSize = _MainVillageMap->getTileSize();
    Size mapSize = _MainVillageMap->getMapSize();

    // 遍历存档数据
    for (const auto& data : dataCenter->_villageLayout) {

        // 1. 创建建筑
        BaseBuilding* newBuilding = nullptr;

        // 根据类型创建具体的子类 
        if (data.type == BuildingType::GOLD_MINE || data.type == BuildingType::ELIXIR_PUMP) {
            newBuilding = ResourceProducer::create(data.type, data.level);
        }
        else if (data.type == BuildingType::GOLD_STORAGE ||
            data.type == BuildingType::ELIXIR_STORAGE ||
            data.type == BuildingType::BARRACKS) {
            auto s = ResourceStorage::create(data.type, data.level);
            _storageList.push_back(s); // 记得加回存储列表
            newBuilding = s;
        }
        else if (data.type == BuildingType::TOWN_HALL) {
            newBuilding = TownHall::create(data.level);
        }
        else if (data.type == BuildingType::ARCHER_TOWER) {
            newBuilding = ArcherTower::create(data.level);
        }
        else if (data.type == BuildingType::CANNON) {
            newBuilding = Cannon::create(data.level);
        }
        else if (data.type == BuildingType::WALL) {
            newBuilding = Wall::create(data.level);
        }
        else {
            // 兜底
            newBuilding = BaseBuilding::create(data.type, data.level);
        }

        if (newBuilding) {
            // 2. 恢复状态
            newBuilding->changeState(BuildingState::IDLE);
            newBuilding->setTag(999); // 设置 Tag

            // 3. 压缩缩放 
            Size spriteSize = newBuilding->getContentSize();
            if (spriteSize.width > 0 && spriteSize.height > 0) {
                newBuilding->setScaleX(tileSize.width / spriteSize.width);
                newBuilding->setScaleY(tileSize.height / spriteSize.height);
            }

            // 4. 计算坐标 
            float finalX = data.tileX * tileSize.width + tileSize.width / 2;
            float finalY = (mapSize.height - 1 - data.tileY) * tileSize.height + tileSize.height / 2;
            newBuilding->setPosition(Vec2(finalX, finalY));

            // 5. 加回地图
            // Z轴：Y越小 Z越大
            _MainVillageMap->addChild(newBuilding, 3000 - (int)finalY);

            // 6. 恢复占用标记
            std::string key = StringUtils::format("%d_%d", data.tileX, data.tileY);
            _occupiedTiles[key] = true;

#if 0       // 还未实现该功能
            // 7.  如果是兵营，恢复视觉上的小兵 
            if (data.type == BuildingType::BARRACKS) {
                // 逻辑：每种兵按数量画出来
                auto allTroops = PlayerData::getAllTroopConfigs();
                for (auto info : allTroops) {
                    int count = PlayerData::getInstance()->getTroopCount(info.name);
                    for (int k = 0; k < count; ++k) {
                        newBuilding->addVisualTroop(info.name);
                    }
                }
            }
        }
    }
#endif
        }
    }
    // 刷新全局容量上限
    this->refreshTotalCapacity();
    this->updateResourceUI();
}