#include "MainVillage.h"
#include "SimpleAudioEngine.h"


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
    _selectedBuildingType = BuildingType::GOLD_MINE;

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
                        //其他未定义建筑类型
                        else {
                            newBuilding = BaseBuilding::create(_selectedBuildingType, 1);
                        }
                        if (newBuilding) {
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
    long long totalGoldCapacity = 1000;
    long long totalElixirCapacity = 1000;
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
        else {
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
            auto createTroopNode = [&](TroopInfo info, int index) -> Node* {

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
                    if ( PlayerData::getInstance()->consumeElixir(info.cost)&& PlayerData::getInstance()->addPeople(info.weight)) {
                        // +1 逻辑
                        _trainingQueue[info.name]++;

                        // 动画 
                        sprite->stopAllActions();
                        sprite->runAction(Sequence::create(ScaleTo::create(0.05f, s * 1.1f), ScaleTo::create(0.05f, s), nullptr));

                        // 刷新 UI
                        auto countLbl = (Label*)container->getChildByTag(101);
                        auto menu = (Menu*)container->getChildByTag(102);
                        if (countLbl) countLbl->setString(StringUtils::format("x%d", _trainingQueue[info.name]));
                        if (menu) {
                            auto minItem = menu->getChildByTag(200);
                            if (minItem) minItem->setVisible(true);
                        }
                        CCLOG("增加: %s", info.name.c_str());
                    }
                    });
                btnAdd->setPosition(35, 60); // 按钮在容器中的位置

                // --- B. 左上角减号按钮 ---
                auto minusWrapper = Node::create();
                minusWrapper->setContentSize(Size(30, 30)); // 增大点击区域
                minusWrapper->setAnchorPoint(Vec2(0.5, 0.5));

                auto minLbl = Label::createWithSystemFont("-", "Arial", 60);
                minLbl->setColor(Color3B::RED);
                minLbl->enableOutline(Color4B::WHITE, 2);
                minLbl->setPosition(15, 15); // 文字居中
                minusWrapper->addChild(minLbl);

                auto btnMinus = MenuItemSprite::create(minusWrapper, nullptr, [=](Ref*) {
                    // -1 逻辑
                    if (_trainingQueue[info.name] > 0) {
                        _trainingQueue[info.name]--;
                        PlayerData::getInstance()->removePeople(info.weight);// 返还人口容量
                        auto countLbl = (Label*)container->getChildByTag(101);
                        if (countLbl) countLbl->setString(StringUtils::format("x%d", _trainingQueue[info.name]));

                        if (_trainingQueue[info.name] <= 0) {
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
                int currentCount = _trainingQueue[info.name];
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
    this->addChild(_buildMenuNode, 1000);

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
        // === 资源类 ===
        items.pushBack(createBtn("Gold Mine", "Gold_Mine1.png", BuildingType::GOLD_MINE));
        items.pushBack(createBtn("Elixir Pump", "Elixir_Pump1.png", BuildingType::ELIXIR_PUMP));
        items.pushBack(createBtn("Gold Store", "Gold_Storage1.png", BuildingType::GOLD_STORAGE));
        items.pushBack(createBtn("Elixir Store", "Elixir_Storage1.png", BuildingType::ELIXIR_STORAGE));
    }
    else if (category == 1) {
        // === 防御类 ===

        items.pushBack(createBtn("Cannon", "Cannon1.png", BuildingType::GOLD_MINE));
        items.pushBack(createBtn("Archer Tower", "Archer_Tower1.png", BuildingType::GOLD_MINE));
        items.pushBack(createBtn("Wall", "Wall1.png", BuildingType::GOLD_MINE));
        items.pushBack(createBtn("Barracks", "Barracks1.png", BuildingType::BARRACKS));
    }

    // 4. 创建菜单并添加到容器
    auto menu = Menu::createWithArray(items);
    menu->alignItemsHorizontallyWithPadding(20);

    // 放在面板中间稍微偏下 (因为顶部有页签)
    menu->setPosition(visibleSize.width / 2, (panelHeight - 50) / 2);

    _iconContainer->addChild(menu);
}