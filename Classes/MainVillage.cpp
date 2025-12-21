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
    this->scheduleUpdate();

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

            PlayerData::getInstance()->playEffect("Audio/click.mp3");

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

    this->createCancelUI();

    // ====================最左下角战斗主按钮========================
    this->createAttackUI();

    // ===================== 右下角设置按钮 ========================

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

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

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
    btnSettings->setPosition(Vec2(visibleSize.width - 50, 50));

    // 5. 添加到菜单
    auto settingsMenu = Menu::create(btnSettings, nullptr);
    settingsMenu->setPosition(Vec2::ZERO);
    this->addChild(settingsMenu, 2000); // UI层级，保证在最上层
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
        // --- 情况E：有遮护罩(如选择关卡和设置) ---
        if (_settingsLayer) {
            _isDragging = false;
            _isClickValid = false;
            return true; 
        }

        // --- 情况F：建筑取消按钮 ---
        if (_cancelBuildMenu && _cancelBuildMenu->isVisible()) {
            // 将鼠标坐标转换为菜单内的相对坐标
            Vec2 localPos = _cancelBuildMenu->convertToNodeSpace(mousePos);

            for (auto child : _cancelBuildMenu->getChildren()) {
                if (child->getBoundingBox().containsPoint(localPos)) {
                    _isDragging = false;
                    _isClickValid = false;
                    return true; // 点到了取消按钮！拦截！
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

        if (_cancelBuildMenu && _cancelBuildMenu->isVisible()) {

            // 将鼠标坐标转换到 Menu 的坐标系中
            Vec2 menuSpacePos = _cancelBuildMenu->convertToNodeSpace(mousePos);

            // 遍历 Menu 里的子节点 (也就是我们的那个文本框按钮)
            for (auto child : _cancelBuildMenu->getChildren()) {

                // 判定矩形范围
                if (child->getBoundingBox().containsPoint(menuSpacePos)) {

                    CCLOG("拦截成功：点击了取消文本框");

                    // A. 停止地图拖拽
                    _isDragging = false;

                    // B. 执行取消逻辑 (手动执行，以防 MenuItem 回调没触发)
                    _selectedBuildingType = BuildingType::NONE;
                    this->updateOccupiedGridVisual();
                    _cancelBuildMenu->setVisible(false);

                    // C. 绝对返回，不执行下面的 map 放置逻辑
                    return;
                }
            }
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
								// 播放建造音效
                                PlayerData::getInstance()->playEffect("Audio/plop.mp3");
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

                                // 开始建造流程 (显示进度条，进入 BUILDING 状态)
                                newBuilding->startConstruction();
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
                        if (_selectedBuildingType != BuildingType::WALL){
                            _selectedBuildingType = BuildingType::NONE;
                            if (_cancelBuildMenu) {
                                _cancelBuildMenu->setVisible(false);
                            }
                        }
                        // 清空非法区域
                        updateOccupiedGridVisual();
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
                            // 只有当状态是 IDLE 时才执行菜单逻辑
                            if (building->state == BuildingState::IDLE) {
                                CCLOG("右键选中了建筑，弹出操作菜单");
                                this->showBuildingMenu(building);
                            }
                            else {
                                // 如果不是 IDLE (比如正在建造)，打印日志
                                CCLOG("建筑正在施工或被摧毁，无法打开菜单");
                            }

                            // 只要点中了建筑，无论是否打开菜单，都应该 break (停止检测重叠的物体)
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

    // 播放音乐
    PlayerData::getInstance()->playBGM("bgm_village.mp3",true);
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
    goldIcon->setPosition(visibleSize.width - 200, visibleSize.height - 40);
    this->addChild(goldIcon, 9999);

    _goldLabel = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    _goldLabel->setColor(Color3B(255, 215, 0)); //金色
    _goldLabel->enableOutline(Color4B::BLACK, 2); 
    _goldLabel->setAnchorPoint(Vec2(0, 0.5));
    _goldLabel->setPosition(visibleSize.width - 170, visibleSize.height - 40);
    this->addChild(_goldLabel, 9999);

    // 2. 圣水 UI (金币下方)
    auto elixirIcon = Sprite::create("Elixir.png"); 
    elixirIcon->setPosition(visibleSize.width - 200, visibleSize.height - 80);
    this->addChild(elixirIcon, 9999);

    _elixirLabel = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    _elixirLabel->setColor(Color3B(255, 80, 255)); // 紫色
    _elixirLabel->enableOutline(Color4B::BLACK, 2);
    _elixirLabel->setAnchorPoint(Vec2(0, 0.5));
    _elixirLabel->setPosition(visibleSize.width - 170, visibleSize.height - 80);
    this->addChild(_elixirLabel, 9999);

    // 3. 人口 UI (圣水下方)
    auto peopleIcon = Sprite::create("People.png");
    peopleIcon->setPosition(visibleSize.width - 200, visibleSize.height - 120);
    this->addChild(peopleIcon, 9999);

    _peopleLabel = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    _peopleLabel->enableOutline(Color4B::BLACK, 2);
    _peopleLabel->setAnchorPoint(Vec2(0, 0.5));
    _peopleLabel->setPosition(visibleSize.width - 170, visibleSize.height - 120);
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
    // 设定初始资源值
    long long totalGoldCapacity = 20000000;
    long long totalElixirCapacity = 20000000;
    long long totalPeopleCapacity = 0;

    // 1. 遍历容器中的每一个建筑
    for (auto building : _storageList) {
        // 确保是建筑类
        if (building) {

            // 根据类型累加对应的种类资源
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
        _activeMenuNode->removeFromParent(); // 关闭当前菜单
        _activeMenuNode = nullptr;
    }
}

void MainVillage::showBuildingMenu(BaseBuilding* building) {
    //======================= 1. 如果还有未关闭的菜单 ====================
    if (_activeMenuNode != nullptr) {
        closeBuildingMenu();
    }
    if (!building) return;

    //======================= 2. 创建一个节点作为菜单容器 ================
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

    // 定义函数：创建图标按钮 
    auto createIconBtn = [](std::string imgName, std::string text, const ccMenuCallback& callback) -> MenuItem* {

        // 1. 创建容器  点击区 - 80x80
        auto wrapper = Node::create();
        wrapper->setContentSize(Size(80, 80));
        wrapper->setAnchorPoint(Vec2(0.5, 0.5));

        // 2. 创建建筑图标
        auto sprite = Sprite::create(imgName);
        float s = 70.0f / std::max(sprite->getContentSize().width, sprite->getContentSize().height);// 缩放：限制在 70x70 以内

        sprite->setScale(s);
        sprite->setPosition(40, 40); // 居中
        wrapper->addChild(sprite);

        // 3. 建筑名称 放在图标下侧
        if (!text.empty()) {
            auto lbl = Label::createWithSystemFont(text, "Arial", 16);
            lbl->enableOutline(Color4B::BLACK, 1);
            lbl->setPosition(40, -15); // 图标下方
            wrapper->addChild(lbl);
        }

        // 4. 创立按钮回调操作
        auto btn = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {

            PlayerData::getInstance()->playEffect("Audio/click.mp3");

            sprite->stopAllActions(); // 进行一个放缩处理
            sprite->runAction(Sequence::create( 
                ScaleTo::create(0.1f, s * 0.9f),
                ScaleTo::create(0.1f, s),
                nullptr
            ));

            // 执行不同按钮的回调逻辑
            if (callback) callback(sender);
            });

        return btn;
        };

    //======================== 3. 创建各个功能按钮 =======================

    // --- 按钮 1: 信息 (Info) ---
    // 请替换 "btn_info.png" 为你的实际文件名
    auto btnInfo = createIconBtn("btn_info.png", "Info", [=](Ref* sender) {

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

        // 3. 创建背景板 
        float bgWidth = 300;
        float bgHeight = 200;

        // A. 准备图片路径
        std::string bgImg = "info_panel.png"; 

        // B. 定义九宫格坐标
        Rect capInsets = Rect(25, 25, 50, 50);

        // C. 创建九宫格精灵
        auto bg = ui::Scale9Sprite::create(bgImg);

        if (bg) {
            // 如果图片加载成功，设置拉伸区域
            bg->setCapInsets(capInsets);
        }
        else {
           // 未找到照片，防止崩溃
            auto sprite = Sprite::create();
            sprite->setTextureRect(Rect(0, 0, 100, 100));
            sprite->setColor(Color3B(0, 0, 0));
            sprite->setOpacity(200);
            bg = ui::Scale9Sprite::createWithSpriteFrame(sprite->getSpriteFrame());
        }

        // D. 设置信息背景板大小 
        bg->setContentSize(Size(bgWidth, bgHeight)); // (300x200)

        // E. 设置属性 
        bg->setName("InfoBackground");     // 用于鼠标点击检测
        bg->setAnchorPoint(Vec2(0, 0.5f)); // 左侧垂直居中锚点
        bg->setPosition(Vec2(60, 0));      // 放在建筑右侧

        _activeMenuNode->addChild(bg);
        // 4. 显示通用信息 (名字、等级、HP)
        std::string nameStr = building->name;

        std::string commonText = StringUtils::format(
            "Name: %s\nLevel: %d\nHP: %d / %d",
            nameStr.c_str(),
            building->level,
            (int)building->currentHP,
            (int)building->maxHP
        );


        auto labelCommon = Label::createWithTTF(commonText, "fonts/GROBOLD.ttf", 20); 
        labelCommon->setColor(Color3B(65, 105, 225)); // 蓝色
        labelCommon->setPosition(bgWidth / 2, bgHeight / 2 + 20);
        bg->addChild(labelCommon);

        // 5. 显示特殊信息 (根据类型判断)
        std::string specialText = "";
        BuildingStats stats = BaseBuilding::getStatsConfig(building->type, building->level);//获取建筑信息
        if (building->type == BuildingType::GOLD_MINE || building->type == BuildingType::ELIXIR_PUMP) {
            specialText = StringUtils::format("Production: %d / hour", stats.productionRate);
        }
        else if (building->type == BuildingType::CANNON || building->type == BuildingType::ARCHER_TOWER) {
            specialText = StringUtils::format("Damage: %.1lf ", stats.damage);
        }
        else if (building->type != BuildingType::WALL && building->type != BuildingType::TOWN_HALL) {
            specialText = StringUtils::format("Capacity: %d", stats.capacity);
        }

        if (!specialText.empty()) { // 如果不为空即存在特殊属性
            auto labelSpecial = Label::createWithTTF(specialText, "fonts/GROBOLD.ttf", 20);
            labelSpecial->setColor(Color3B::MAGENTA); // 特殊属性用洋红色
            labelSpecial->setPosition(bgWidth / 2, bgHeight / 2 - 40);
            bg->addChild(labelSpecial);
        }

        // 6. 添加一个关闭按钮
        auto closeLabel = Label::createWithTTF("[ Close ]", "fonts/GROBOLD.ttf", 24);
        closeLabel->setColor(Color3B(255, 50, 50)); // 红色
        closeLabel->enableOutline(Color4B::BLACK, 1); // 黑边
        auto closeItem = MenuItemLabel::create(closeLabel, [=](Ref* sender) {

            PlayerData::getInstance()->playEffect("Audio/click.mp3");

            this->closeBuildingMenu();
            });
        closeItem->setPosition(bgWidth - 60, bgHeight - 20); // 放置在右上角位置

        closeLabel->setColor(Color3B(255, 50, 50));
        closeLabel->enableBold();         // 描边 

        auto menu = Menu::create(closeItem, nullptr); // 组装进菜单
        menu->setPosition(Vec2::ZERO);
        bg->addChild(menu);

        // 弹出动画
        bg->setScale(0);
        bg->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));

        CCLOG("信息展示：");
        });

    // --- 按钮 2: 升级 (Upgrade) ---
    auto btnUpgrade = createIconBtn("btn_upgrade.png", "Upgrade", [=](Ref* sender) {
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
            CCLOG("升级失败：需要大本营 Lv.%d 才能升级到 Lv.%d", currentTHLevel + 1, nextLevel); // 如果满足限制规定
            return; // 直接拦截，不扣钱
        }

        BuildingStats nextStats = BaseBuilding::getStatsConfig(building->type, nextLevel);//获取升级花费

        int upgradeCostGold = nextStats.costGold;
        int upgradeCostElixir = nextStats.costElixir;
        if (PlayerData::getInstance()->consumeGold(upgradeCostGold) && PlayerData::getInstance()->consumeElixir(upgradeCostElixir)) {
            building->startUpgradeProcess(); // 扣钱成功，开始升级
            this->updateResourceUI(); // 刷新右上角钱数

            //关闭菜单
            this->closeBuildingMenu();
        }
        else {
            CCLOG("金币不足，无法升级！");
        }
        });

    // --- 按钮 3: 训练 (Train) - 仅兵营 ---
    MenuItem* btnTrain = nullptr; // 因为不确定是否有兵营 初始设为空
    if (building->type == BuildingType::BARRACKS) { // 如果是兵营
        btnTrain = createIconBtn("btn_train.png", "Train", [=](Ref* sender) {
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
            std::string bgPath = "TrainingBg.png";   // 准备图片路径 
            Rect capInsets = Rect(20, 20, 60, 60); // 创建九宫格背景

            auto bg = ui::Scale9Sprite::create(bgPath);
            bg->setCapInsets(capInsets); // 使用九宫格设计
            bg->setContentSize(Size(panelW, panelH)); // 拉伸到指定大小
            bg->setName("TrainPanel");   // 设定Tag，用于后续鼠标点击测试

            bg->setAnchorPoint(Vec2(0.5, 1));
            bg->setPosition(Vec2(0, -200)); // 放在建筑下方 

            _activeMenuNode->addChild(bg); // 放进 _activeMenuNode的孩子节点

            // 3. 生成士兵选择按钮
            auto createTroopNode = [=](TroopInfo info, int index) -> Node* {

                // 创建容器
                auto container = Node::create();
                container->setContentSize(Size(70, 90));

                // --- A. 人物头像大按钮 ---
                auto iconWrapper = Node::create();
                iconWrapper->setContentSize(Size(60, 60)); // 使用Wrapper包裹按钮大小
                iconWrapper->setAnchorPoint(Vec2(0.5, 0.5)); // 放在中心位置

                auto sprite = Sprite::create(info.img); // 获取人物照片

                float s = 60.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
                sprite->setScale(s); // 进行放缩
                sprite->setPosition(30, 30); // 放在 Wrapper 的正中心
                iconWrapper->addChild(sprite);

                auto btnAdd = MenuItemSprite::create(iconWrapper, nullptr, [=](Ref*) {
                    // 先判断容量
                    if (PlayerData::getInstance()->consumeElixir(info.cost) && PlayerData::getInstance()->addPeople(info.weight, info.cost)) {
                        // +1 逻辑
                        PlayerData::getInstance()->addTroop(info.name, 1);

                        PlayerData::getInstance()->playEffect("Audio/plop.mp3");

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
                        else if (info.name == "WallBreaker") { 
                            newSoldier = WallBreaker::create();
                        }

                        if (newSoldier) {

                            newSoldier->setHomeMode(true); // // 开启游走
                            newSoldier->setHomePosition(building->getPosition());

                            // 1. 确定“出生点” (兵营大门位置)
                            // 假设兵营图片的“门”在中心点向下 20 像素的位置
                            Vec2 doorPos = building->getPosition() + Vec2(0, -20);

                            // 2. 计算“目标点” (扇形随机分布)
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

                            newSoldier->setPosition(doorPos);    // 先把兵放在门口                      
                            newSoldier->setScale(0.2f);    // 且设为极小（刚训练出来）

                            // 根据目标点在左边还是右边，调整朝向
                            if (targetPos.x < doorPos.x) {
                                // 如果目标在左边，需要翻转 
                               newSoldier->setScaleX(-0.01f); // 用负 Scale 实现翻转
                            }

                            // 4. 添加到地图层 (处理遮挡)
                            _MainVillageMap->addChild(newSoldier, 3000 - (int)doorPos.y);

                            // 5. 出场动作
                            float jumpHeight = 20.0f;
                            float duration = 0.5f;

                            // 动作A: 跳跃移动到目标点
                            auto jumpAct = JumpTo::create(duration, targetPos, jumpHeight, 1);

                            // 动作B: 变大恢复正常 
                            float targetScale = 0.2f; // 标准大小
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

                            // 加入到可视化士兵容器里
                            building->_visualTroops[info.name].push_back(newSoldier);

                            CCLOG("士兵实体 %s 扇形生成在 (%f, %f)", info.name.c_str(), targetPos.x, targetPos.y);
                        }
                    }
                    });
                btnAdd->setPosition(35, 60); // 按钮在容器中的位置

                // --- B. 左上角减号按钮 ---
                auto minusWrapper = Node::create();
                minusWrapper->setContentSize(Size(50, 50)); // 50*50点击区域
                minusWrapper->setAnchorPoint(Vec2(0.5, 0.5));

                auto minLbl = Label::createWithSystemFont("-", "Arial", 60);
                minLbl->setColor(Color3B::RED);
                minLbl->enableOutline(Color4B::WHITE, 2);
                minLbl->setPosition(15, 15); // 文字居中
                minusWrapper->addChild(minLbl);

                auto btnMinus = MenuItemSprite::create(minusWrapper, nullptr, [=](Ref*) {
                    // -1 逻辑

                    PlayerData::getInstance()->playEffect("Audio/click.mp3");

                    if (PlayerData::getInstance()->consumeTroop(info.name, 1)) {
                        PlayerData::getInstance()->removePeople(info.weight);// 返还人口容量
                        auto countLbl = (Label*)container->getChildByTag(101);
                        if (countLbl) countLbl->setString(StringUtils::format("x%d", PlayerData::getInstance()->getTroopCount(info.name)));

                        // --- 视觉删除逻辑 ---
                        // 1. 找到该兵种的列表
                        auto& soldierList = building->_visualTroops[info.name];

                        if (!soldierList.empty()) {
                            // 2. 取出最后生成的那个兵 
                            Node* soldierToRemove = soldierList.back();

                            // 3. 从列表中移除 
                            soldierList.pop_back();

                            // 4. 执行删除动画
                            if (soldierToRemove) {
                                soldierToRemove->stopAllActions(); // 停止它当前的动作

                                auto deleteSeq = Sequence::create(
                                    // 步骤A: 变红，提示玩家它被删除了
                                    TintTo::create(0.1f, 255, 0, 0),  

                                    // 步骤B: 同时缩小和淡出
                                    Spawn::create(
                                        ScaleTo::create(0.3f, 0.0f), // 缩小到没了
                                        FadeOut::create(0.3f)        // 变透明
                                    ),

                                    // 步骤C: 从父节点(_MainVillageMap)上删掉
                                    RemoveSelf::create(),
                                    nullptr
                                );

                                soldierToRemove->runAction(deleteSeq); // 执行删除动作
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
                // 减号位置
                btnMinus->setPosition(10, 85); //人物头像的左上角
                btnMinus->setTag(200);

                // --- C. 组装 Menu ---
                auto localMenu = Menu::create(btnAdd, btnMinus, nullptr);
                localMenu->setPosition(Vec2::ZERO);
                localMenu->setTag(102);
                container->addChild(localMenu);

                // --- D. 标签 (右上角) ---
                int currentCount = PlayerData::getInstance()->getTroopCount(info.name); // 获取该人物的数量
                auto countLbl = Label::createWithSystemFont(StringUtils::format("x%d", currentCount), "Arial", 16);// 创建对应的label并更新 
                countLbl->setColor(Color3B::GREEN);
                countLbl->setAnchorPoint(Vec2(1, 0.5));
                countLbl->setPosition(68, 90); // 右上角位置
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

            // 5. 循环创建不同士兵头像 并排列 
            float startX = 30;
            float gapX = 90;
            float posY = panelH / 2 - 40;

            for (int i = 0; i < troops.size(); ++i) { //troop.size确定士兵种数
                // 调用上面的 lambda函数创建
                auto troopNode = createTroopNode(troops[i], i);
                troopNode->setPosition(startX + i * gapX, posY); 
                bg->addChild(troopNode);
            }

            // 7. 添加关闭按钮 (X)
            auto closeLbl = Label::createWithSystemFont("X", "Arial", 26);
            closeLbl->setColor(Color3B::RED);
            auto closeItem = MenuItemLabel::create(closeLbl, [=](Ref*) {

                PlayerData::getInstance()->playEffect("Audio/click.mp3");

                this->closeBuildingMenu();
                });
            closeItem->setPosition(panelW - 20, panelH - 20);

            auto closeMenu = Menu::create(closeItem, nullptr); // 加入菜单
            closeMenu->setPosition(Vec2::ZERO);
            bg->addChild(closeMenu);

            CCLOG("打开了兵营训练面板");
            });
    }
    // --- 按钮 4: 移除 (Remove) ---
    auto btnRemove = createIconBtn("btn_remove.png", "Remove", [=](Ref* sender) {
           CCLOG("点击了拆除按钮");

            // 1. 如果是存储建筑 总资源容量会发生改变
           if (building->type == BuildingType::GOLD_STORAGE ||
               building->type == BuildingType::ELIXIR_STORAGE ||
               building->type == BuildingType::BARRACKS) {

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

    // ===================== 4.组装横向菜单 =====================
    Vector<MenuItem*> menuItems;

    // 信息 - 升级 - (训练) - 拆除
    menuItems.pushBack(btnInfo);
    menuItems.pushBack(btnUpgrade);

    if (btnTrain) {
        menuItems.pushBack(btnTrain);
    }

    menuItems.pushBack(btnRemove);

    auto menu = Menu::createWithArray(menuItems);

    menu->alignItemsHorizontallyWithPadding(10); //水平排列 间距为10

    menu->setName("BuildingMenu");
    menu->setPosition(Vec2::ZERO);
    _activeMenuNode->addChild(menu);

    // 弹出来的入场动作
    _activeMenuNode->setScale(0);
    _activeMenuNode->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f / mapScaleX, 1.0f / mapScaleY)));
}

void MainVillage::createBuildUI() {

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建底部面板容器 
    _buildMenuNode = Node::create();
    _buildMenuNode->setVisible(false);
    this->addChild(_buildMenuNode, 1100);

    // A. 半透明黑底

    float panelHeight = 280;
    std::string bgPath = "TrainingBg.png"; // 准备图片路径
    Rect capInsets = Rect(20, 20, 60, 60);

    // 3. 创建背景
    auto bg = ui::Scale9Sprite::create(bgPath);
    bg->setCapInsets(capInsets);
    bg->setContentSize(Size(visibleSize.width, panelHeight));  // 宽度设为屏幕宽度，高度设为 panelHeight
    bg->setAnchorPoint(Vec2(0, 0));    // 锚点设为左下角 (0, 0)
    bg->setPosition(Vec2::ZERO);
    bg->setName("BuildPanelBG");    // 设置Tag，用于点击检测

    _buildMenuNode->addChild(bg);

    // B. 创建顶部的分类页签 (资源 | 防御)
    // 1. 创建 Label 和 Item  
    auto labelRes = Label::createWithTTF("RESOURCES", "fonts/GROBOLD.ttf", 26);
    labelRes->enableOutline(Color4B::BLACK, 2); // 黑色描边
    auto itemRes = MenuItemLabel::create(labelRes, nullptr);

    auto labelDef = Label::createWithTTF("DEFENSE", "fonts/GROBOLD.ttf", 26);
    labelDef->enableOutline(Color4B::BLACK, 2);
    auto itemDef = MenuItemLabel::create(labelDef, nullptr);

    // 2. 设置 [资源] 按钮的回调
    itemRes->setCallback([=](Ref*) {

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

        // 视觉变化：自己变绿，别人变灰
        itemRes->setColor(Color3B::GREEN);
        itemDef->setColor(Color3B::GRAY);

        // 逻辑变化
        this->switchBuildCategory(0);
        });

    // 3. 设置 [防御] 按钮的回调
    itemDef->setCallback([=](Ref*) {

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

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
    tabMenu->alignItemsHorizontallyWithPadding(120);
    tabMenu->setPosition(visibleSize.width / 2, panelHeight - 35);
    _buildMenuNode->addChild(tabMenu);

    // C. 创建内容容器 (存放建筑图标)
    _iconContainer = Node::create();
    _iconContainer->setPosition(0, 0); 
    _buildMenuNode->addChild(_iconContainer);

    // 默认加载资源类
    switchBuildCategory(0);
}

void MainVillage::createCancelUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建文字标签
    auto label = Label::createWithTTF("CANCEL BUILD", "fonts/GROBOLD.ttf", 24);
    label->enableOutline(Color4B::BLACK, 2);    // 加黑色描边，增加清晰度
    label->enableShadow(Color4B::BLACK, Size(2, -2), 0);    // 稍微加点阴影

    // 2. 创建背景框
    float width = label->getContentSize().width + 40;
    float height = 50;

    auto bg = LayerColor::create(Color4B(200, 0, 0, 200), width, height);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5, 0.5)); // 中心锚点

    // 3. 创建一个容器节点把它俩装起来
    auto container = Node::create();
    container->setContentSize(Size(width, height)); // 设置尺寸，以便检测点击
    container->setAnchorPoint(Vec2(0.5, 0.5));

    // 居中放置背景和文字
    bg->setPosition(width / 2, height / 2);
    label->setPosition(width / 2, height / 2);

    container->addChild(bg);
    container->addChild(label);

    // 4. 创建按钮逻辑
    auto btnItem = MenuItemSprite::create(container, container, [=](Ref* sender) {
        
        PlayerData::getInstance()->playEffect("Audio/click.mp3");
        
        CCLOG("UI回调：取消建造");

        // 恢复状态
        _selectedBuildingType = BuildingType::NONE;

        // 清除辅助红绿网格
        this->updateOccupiedGridVisual();

        // 隐藏自己
        _cancelBuildMenu->setVisible(false);
        });

    // 5. 创建菜单
    _cancelBuildMenu = Menu::create(btnItem, nullptr);

    // 6. 设置位置
    _cancelBuildMenu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 80));
    _cancelBuildMenu->setVisible(false); // 默认隐藏

    // 7. 添加到场景
    this->addChild(_cancelBuildMenu, 9999);
}

void MainVillage::switchBuildCategory(int category) {
    if (!_iconContainer) return;

    // 1. 清空当前显示的图标
    _iconContainer->removeAllChildren();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    float panelHeight = 280;

    // 2. 辅助 lambda：创建大图标 
    auto createBtn = [&](std::string name, std::string img, BuildingType type) {
        // 创建图
        auto sprite = Sprite::create(img);
        if (!sprite) sprite = Sprite::create(); 

        // 缩放图片到合适大小（80*80） 
        float s = 80.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
        sprite->setScale(s);

        // 创建建筑名称
        auto lbl = Label::createWithTTF(name, "fonts/GROBOLD.ttf", 16);
        lbl->enableOutline(Color4B::BLACK, 2);  // 描边

        // 组合成 MenuItemSprite
        auto nodeNormal = Node::create();
        nodeNormal->setContentSize(Size(120, 150));
        sprite->setPosition(60, 65);
        lbl->setPosition(60, 125);
        nodeNormal->addChild(sprite);
        nodeNormal->addChild(lbl);

        // 显示造价 (底部)
        BuildingStats stats = BaseBuilding::getStatsConfig(type, 1);      // 1. 获取该建筑 1 级的配置数据

        int costVal = 0;  // 造价
        std::string iconPath = "";  // 图片路径
        Color3B textColor = Color3B::WHITE;

        // 判断是金币还是圣水
        if (stats.costGold > 0) {
            costVal = stats.costGold; // 消耗金币
            iconPath = "Gold.png";          
            textColor = Color3B(255, 220, 0); // 金色
        }
        else {
            costVal = stats.costElixir; // 消耗圣水
            iconPath = "Elixir.png";        
            textColor = Color3B(220, 0, 220);// 紫色
        }

        auto priceNode = Node::create();  // 创建价格容器 
        priceNode->setPosition(60, 10); // 放在底部位置
        nodeNormal->addChild(priceNode);

        // 创建资源小图标
        auto resIcon = Sprite::create(iconPath);
        float iconWidth = 0;
        if (resIcon) {
            float iconScale = 20.0f / resIcon->getContentSize().height; // 限制高度20
            resIcon->setScale(iconScale);
            resIcon->setAnchorPoint(Vec2(0, 0.5)); // 左对齐
            priceNode->addChild(resIcon);
            iconWidth = resIcon->getContentSize().width * iconScale;
        }

        // 创建造价文字
        auto lblCost = Label::createWithTTF(std::to_string(costVal), "fonts/GROBOLD.ttf", 18);
        lblCost->setColor(textColor);
        lblCost->enableOutline(Color4B::BLACK, 2); // 描边加粗
        lblCost->setAnchorPoint(Vec2(0, 0.5));     // 左对齐
        priceNode->addChild(lblCost);

        float textWidth = lblCost->getContentSize().width;   // 排版居中计算
        float gap = 5.0f;  // 设置标签之间的距离
        float totalW = iconWidth + gap + textWidth;

        // 从左边的起始点开始画
        float startX = -totalW / 2.0f;

        if (resIcon) {
            resIcon->setPosition(startX, 0);
            startX += iconWidth + gap;
        }
        lblCost->setPosition(startX, 0);

        // 点击按钮回调
        return MenuItemSprite::create(nodeNormal, nodeNormal, [=](Ref*) {

            PlayerData::getInstance()->playEffect("Audio/click.mp3");

            _selectedBuildingType = type; // 设置建筑对应类型
            this->updateOccupiedGridVisual();
            CCLOG("选中建筑: %s", name.c_str());
            _buildMenuNode->setVisible(false); // 选完自动关闭

            if (_cancelBuildMenu) {// 同时打开取消建造按钮
                _cancelBuildMenu->setVisible(true);
                _cancelBuildMenu->setScale(0);
                _cancelBuildMenu->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
            }

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
    menu->setPosition(visibleSize.width / 2, (panelHeight - 50) / 2);    // 放在面板中间偏下 

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

    // 1. 创建容器 (Wrapper)点击区域
    float boxSize = 130.0f; // 根据图片设置大小
    auto wrapper = Node::create();
    wrapper->setContentSize(Size(boxSize, boxSize)); // box大小
    wrapper->setAnchorPoint(Vec2(0.5, 0.5)); // 中心锚点

    // 2. 创建并处理图片
    std::string iconPath = "Attack_Button.png";
    auto sprite = Sprite::create(iconPath);

    // 进行略微缩放
    float targetIconSize = 100.0f;
    float contentMax = std::max(sprite->getContentSize().width, sprite->getContentSize().height);    // 获取图片的最长边 (宽或高)
    if (contentMax <= 0) contentMax = 100.0f;    // 防止除以0
    float scale = 1.5 * targetIconSize / contentMax;    // 计算缩放比例：目标大小 / 原始大小
    sprite->setScale(scale);    // 应用均匀缩放 
    sprite->setPosition(boxSize / 2, boxSize / 2);    // 把图片放在容器的正中心
    wrapper->addChild(sprite);    // 放入容器

    // 3. 创建按钮 (使用 wrapper)
    auto btnAttack = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

        // 点击收缩反馈
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
    btnAttack->setPosition(Vec2(80, 80)); // 设置位置

    // 呼吸动画：对按钮进行缩放
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
    // 0. 防止重复打开
    if (_settingsLayer) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ==================== 1. 创建全屏遮罩 =====================
    _settingsLayer = LayerColor::create(Color4B(0, 0, 0, 200), visibleSize.width, visibleSize.height);
    _settingsLayer->setName("LevelSelectionLayer");

    // 拦截点击
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _settingsLayer);

    this->addChild(_settingsLayer, 20000);

    // ===================== 2. 创建弹窗背景 ====================
    auto bg = ui::Scale9Sprite::create("Level_Panel.png");
    if (!bg) {
        auto s = Sprite::create();
        s->setTextureRect(Rect(0, 0, 100, 100));
        s->setColor(Color3B(80, 60, 40));
        bg = ui::Scale9Sprite::createWithSpriteFrame(s->getSpriteFrame());
    }
    bg->setContentSize(Size(500, 550));
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    _settingsLayer->addChild(bg);

    // 标题
    auto title = Label::createWithTTF("SELECT BATTLE", "fonts/GROBOLD.ttf", 36);
    title->enableOutline(Color4B::BLACK, 2);
    title->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 50);
    bg->addChild(title);

    // ==================== 3. 辅助函数：创建单个关卡条目 ==================
    auto createLevelBtn = [&](std::string name, int stars, bool isLocked, const std::function<void()>& callback) -> MenuItem* {

        // --- A. 创建容器 ---
        auto wrapper = Node::create();
        wrapper->setContentSize(Size(400, 100));
        wrapper->setAnchorPoint(Vec2(0.5, 0.5));

        // --- B. 关卡背景 ---
        auto itemBg = ui::Scale9Sprite::create("Level_Item_Bg.png");
        if (!itemBg) {
            auto s = Sprite::create(); s->setTextureRect(Rect(0, 0, 100, 100)); s->setColor(Color3B(120, 90, 60));
            itemBg = ui::Scale9Sprite::createWithSpriteFrame(s->getSpriteFrame());
        }
        itemBg->setContentSize(Size(400, 100));
        itemBg->setPosition(200, 50);
        wrapper->addChild(itemBg);

        // --- C. 关卡名字 ---
        auto lblName = Label::createWithTTF(name, "fonts/GROBOLD.ttf", 24);
        lblName->setAnchorPoint(Vec2(0, 0.5));
        lblName->setPosition(20, 70);
        lblName->enableOutline(Color4B::BLACK, 1);
        wrapper->addChild(lblName);

        // --- D. 关卡状态显示 (锁 或者 星星) ---
        if (isLocked) {
            // === 锁定状态：变灰 + 显示锁 ===
            itemBg->setColor(Color3B::GRAY); // 背景变灰

            auto lockIcon = Sprite::create("icon_lock.png");
            lockIcon->setPosition(350, 50); // 右侧显示锁
            wrapper->addChild(lockIcon);
        }
        else {
            // === 解锁状态：显示星星 + GO ===

            // 绘制 3 颗星星
            for (int i = 0; i < 3; i++) {
                // 如果 stars=3，则 i<3 恒成立，全是金星
                // 如果 stars=0，则 i<0 恒不成立，全是灰星
                std::string starImg = (i < stars) ? "star_gold.png" : "star_gray.png";

                auto star = Sprite::create(starImg);
                if (!star) {
                    star = Sprite::create(); star->setTextureRect(Rect(0, 0, 20, 20));
                    star->setColor((i < stars) ? Color3B::YELLOW : Color3B::GRAY);
                }
                else {
                    star->setScale(0.5f);
                }
                star->setPosition(40 + i * 40, 30);   // 排列星星 (放在名字下方)
                wrapper->addChild(star);
            }

            // 显示 GO 图标 
            auto goLabel = Label::createWithTTF("GO", "fonts/GROBOLD.ttf", 30);
            goLabel->setColor(Color3B::GREEN);
            goLabel->enableOutline(Color4B::BLACK, 2);
            goLabel->setPosition(350, 50);
            wrapper->addChild(goLabel);
        }

        // --- E. 创建按钮 ---
        auto btn = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {
            
            PlayerData::getInstance()->playEffect("Audio/click.mp3");

            // 点击反馈动画
            auto node = dynamic_cast<Node*>(sender);
            node->stopAllActions();
            auto seq = Sequence::create(
                ScaleTo::create(0.1f, 0.95f),
                ScaleTo::create(0.1f, 1.0f),
                CallFunc::create(callback), // 执行传入的回调
                nullptr
            );
            node->runAction(seq);
            });

        return btn;
        };

    // ============== 4. 循环生成关卡按钮 ==============

    // 关卡配置列表
    struct LevelConfig {
        std::string name;
        std::string mapFile;
    };
    std::vector<LevelConfig> levels = {
        {"1. Goblin Forest",   "map/map1.tmx"},
        {"2. Desert Fortress", "map/map2.tmx"},
        {"3. Dark Castle",     "map/map3.tmx"}
    };

    Vector<MenuItem*> menuItems;

    for (int i = 0; i < levels.size(); ++i) {
        int levelID = i + 1;

        // 1. 从 PlayerData 获取数据
        int stars = PlayerData::getInstance()->getLevelStar(levelID);// 如果赢过，stars=3；没赢过，stars=0

        bool isLocked = PlayerData::getInstance()->isLevelLocked(levelID);   // 判断是否锁定

        // 2. 创建按钮
        auto btn = createLevelBtn(levels[i].name, stars, isLocked, [=]() {
            if (isLocked) { // 如果还没解锁
                CCLOG("关卡 %d 未解锁！", levelID);
                // 播放一个“禁止”音效
                // PlayerData::getInstance()->playEffect("error.mp3");
            }
            else {
                CCLOG("进入关卡 %d...", levelID);

                auto scene = GameMap::create(levels[i].mapFile); // 创建对应地图 
                Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene)); // 切换场景
            }
            });

        menuItems.pushBack(btn);
    }

    // ================ 5. 组装菜单 ==================
    auto levelMenu = Menu::createWithArray(menuItems);
    levelMenu->alignItemsVerticallyWithPadding(20);
    // 居中放置
    levelMenu->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height / 2 - 20);
    bg->addChild(levelMenu);

    // ================ 6. 关闭按钮 ==================
    auto lblClose = Label::createWithSystemFont("X", "Arial", 30);
    lblClose->setColor(Color3B::RED);
    lblClose->enableOutline(Color4B::WHITE, 2);

    auto closeBtn = MenuItemLabel::create(lblClose, [=](Ref*) {

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

        if (_settingsLayer) {
            _settingsLayer->removeFromParent();
            _settingsLayer = nullptr;
        }
        });

    closeBtn->setPosition(Vec2(220, 250));    // 右上角位置

    auto closeMenu = Menu::create(closeBtn, nullptr);
    closeMenu->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height / 2);
    bg->addChild(closeMenu);

    // 入场动画
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}

// 存档操作
void MainVillage::saveVillageData() {
    auto dataCenter = PlayerData::getInstance(); // 获取现在的所有信息

    // 1. 清空旧存档
    dataCenter->_villageLayout.clear();

    // 2. 遍历地图子节点
    auto& children = _MainVillageMap->getChildren();

    Size tileSize = _MainVillageMap->getTileSize();
    Size mapSize = _MainVillageMap->getMapSize();

    for (const auto& child : children) {
        // 转换类型
        auto building = dynamic_cast<BaseBuilding*>(child);

        // 排除未建造和已经摧毁的建筑
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

            // 存入建筑存档
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

    BaseBuilding* theBarracks = nullptr;

    // 遍历建筑存档数据
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
            _MainVillageMap->addChild(newBuilding, 3000 - (int)finalY);

            // 6. 恢复占用标记
            std::string key = StringUtils::format("%d_%d", data.tileX, data.tileY);
            _occupiedTiles[key] = true;

            if (newBuilding->type == BuildingType::BARRACKS) {
                theBarracks = newBuilding;
            }
        }
    }
    // 刷新全局容量上限
    this->refreshTotalCapacity();
    this->updateResourceUI();

    // 如果地图上连一个兵营都没有，那兵也没地方去
    if (theBarracks == nullptr) return;

    // 定义生成函数 (只针对 theBarracks)
    auto spawnTroopsAtBarracks = [&](std::string name) {
        Soldier* s = nullptr;
        if (name == "Barbarian") s = Barbarian::create();
        else if (name == "Archer") s = Archer::create();
        else if (name == "Giant") s = Giant::create();
        else if (name == "WallBreaker") s = WallBreaker::create();

        if (s) {
            // 1. 设置模式
            s->setHomeMode(true);
            s->setHomePosition(theBarracks->getPosition());

            // 2. 随机位置 (兵营下方扇形)
            float angle = CC_DEGREES_TO_RADIANS(210 + CCRANDOM_0_1() * 120);
            float radius = 60.0f + CCRANDOM_0_1() * 60.0f;

            Vec2 offset(cos(angle) * radius, sin(angle) * radius);
            Vec2 pos = theBarracks->getPosition() + offset;

            s->setPosition(pos);

            //统一缩放逻辑
            Size ss = s->getContentSize();
            float scale = 0.2f; 

            s->setScale(scale);
            if (pos.x < theBarracks->getPositionX()) s->setScaleX(-scale);

            _MainVillageMap->addChild(s, 3000 - (int)pos.y);

            //把恢复出来的兵，重新登记到兵营名册
            theBarracks->_visualTroops[name].push_back(s);
        }
    };

    // 遍历库存，生成所有兵
    std::vector<std::string> troopTypes = { "Barbarian", "Archer", "Giant", "WallBreaker" };
    for (const auto& name : troopTypes) {
        int count = dataCenter->getTroopCount(name);
        for (int i = 0; i < count; ++i) {
            spawnTroopsAtBarracks(name);
        }
    }

    CCLOG("单兵营模式：士兵视觉已恢复");

}

// 绘制非法放置边框
void MainVillage::updateOccupiedGridVisual() {

    // 如果手里没有预览建筑，就清空并返回
    if (_selectedBuildingType == BuildingType::NONE) {
        if (_gridDrawNode) _gridDrawNode->clear();
        return;
    }
    // 初始化绘制区域 
    if (!_gridDrawNode) {
        _gridDrawNode = DrawNode::create();
        _MainVillageMap->addChild(_gridDrawNode, 20000);    //Z轴保证在所有建筑之上
    }

    // 2. 每次刷新前先清空旧的框框
    _gridDrawNode->clear();

    Size tileSize = _MainVillageMap->getTileSize();
    Size mapSize = _MainVillageMap->getMapSize();

    // 3. 遍历占用表
    for (auto const& item : _occupiedTiles) {
        // item.first 是 key "x_y"
        // item.second 是 bool (是否占用)

        if (item.second == true) {
            // A. 解析 Key 字符串 
            std::string key = item.first; // "10_15"->x = 10, y = 15
            size_t underscorePos = key.find('_');
            if (underscorePos == std::string::npos) continue;

            int tileX = std::stoi(key.substr(0, underscorePos));
            int tileY = std::stoi(key.substr(underscorePos + 1));

            // B. 坐标转换 

            float originX = tileX * tileSize.width;
            float originY = (mapSize.height - 1 - tileY) * tileSize.height;

            Vec2 origin(originX, originY);
            Vec2 dest(originX + tileSize.width, originY + tileSize.height);

            // --- C. 绘制 ---
            // 实心半透明红块 
            _gridDrawNode->drawSolidRect(origin, dest, Color4F(1.0f, 0.0f, 0.0f, 0.3f)); // 红色，30%透明度

            // 空心框
            _gridDrawNode->drawRect(origin, dest, Color4F(1.0f, 0.0f, 0.0f, 0.8f));
        }
    }
}

void MainVillage::showSettingsLayer() {
    if (_settingsLayer) return;    // 防止重复打开

    auto visibleSize = Director::getInstance()->getVisibleSize();
    // 1. 创建遮罩层 
    _settingsLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    _settingsLayer->setName("SettingsLayer"); // Tag命名 用于点击检测

    // 触摸拦截
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _settingsLayer);
    this->addChild(_settingsLayer, 20000); // 最顶层

    // 2. 设置背景板
    std::string bgPath = "setting_panel.png";
    auto bg = ui::Scale9Sprite::create(bgPath);

    // 设置面板大小 
    bg->setContentSize(Size(450, 350)); // (宽450, 高350)
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2); // 位于中心位置
    bg->setName("SettingsBackground"); // 用于点击检测

    _settingsLayer->addChild(bg);

    // 3. 标题
    auto lblTitle = Label::createWithTTF("SETTINGS", "fonts/GROBOLD.ttf", 36);
    lblTitle->enableOutline(Color4B::BLACK, 3); // 描边加粗
    lblTitle->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 40);
    bg->addChild(lblTitle);

    /**
     * @brief 动态创建一行音量控制组件
     * @param title    标题文字 (如 "Music", "Effect")
     * @param iconPath 左侧图标路径 (如 "icon_music.png")
     * @param posY     在背景板上的 Y 轴坐标
     * @param getVal   获取当前音量的回调 (返回 0.0 ~ 1.0)
     * @param setVal   设置音量的回调 (传入 0.0 ~ 1.0)
     * @return Vector<MenuItem*> 返回生成的加减按钮，以便添加到 Menu 中
     */
    auto createVolumeControl = [&](std::string title, std::string iconPath, float posY, std::function<float()> getVal, std::function<void(float)> setVal) {
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
            // 缩放图标到合适大小 (比如 30x30)
            float targetSize = 30.0f;
            float maxSide = std::max(icon->getContentSize().width, icon->getContentSize().height);
            if (maxSide > 0) icon->setScale(targetSize / maxSide);
        }
        // 位置：放在最左边位置
        icon->setPosition(50, posY);
        bg->addChild(icon);

        // 1. 标题
        auto lbl = Label::createWithTTF(title, "fonts/GROBOLD.ttf", 26);
        lbl->setAnchorPoint(Vec2(0, 0.5));
        lbl->enableOutline(Color4B::BLACK, 2); // 描边加粗
        lbl->setPosition(80, posY);
        bg->addChild(lbl);

        // 2. 格子容器
        auto barNode = Node::create();
        barNode->setPosition(200, posY);
        bg->addChild(barNode);

        // 刷新格子的辅助函数
        auto refreshBar = [=](float percent) {
            barNode->removeAllChildren();
            int level = (int)(percent * 10 + 0.5f); // 确定目前的音量格子数 +0.5是为了保证int强制转型避免截断

            for (int i = 0; i < 10; i++) { // 创建十个音量块
                auto block = Sprite::create();
                block->setTextureRect(Rect(0, 0, 15, 20));
                block->setColor(i < level ? Color3B(0, 255, 0) : Color3B(50, 50, 50)); // 小于level的部分亮色用绿色，大于的部分暗色用深灰
                block->setPosition(i * 18, 0);
                barNode->addChild(block);
            }
            };

        // 初始刷新
        refreshBar(getVal());

        // 3. 减号按钮 [-]
        auto lblMinus = Label::createWithTTF("-", "fonts/GROBOLD.ttf", 45); 
        lblMinus->enableOutline(Color4B::BLACK, 3);
        auto btnMinus = MenuItemLabel::create(lblMinus, [=](Ref*) {
            float v = getVal();
            int level = (int)(v * 10 + 0.5f); // 同理现在音量格数
            if (level > 0) {
                level--;  // 音量-1
                setVal(level / 10.0f); // 调用设置音量函数
                refreshBar(level / 10.0f); // 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::getInstance()->playEffect("Audio/click.mp3");
            }
            });
        btnMinus->setPosition(170, posY);

        // 4. 加号按钮 [+]
        auto lblPlus = Label::createWithTTF("+", "fonts/GROBOLD.ttf", 45);
        lblPlus->enableOutline(Color4B::BLACK, 3);
        auto btnPlus = MenuItemLabel::create(lblPlus, [=](Ref*) {
            float v = getVal();
            int level = (int)(v * 10 + 0.5f);// 同理现在音量格数
            if (level < 10) {
                level++;// 音量+1
                setVal(level / 10.0f);// 调用设置音量函数
                refreshBar(level / 10.0f);// 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::getInstance()->playEffect("Audio/click.mp3");
            }
            });
        btnPlus->setPosition(190 + 180 + 20, posY);

        // 返回菜单项，以便添加到主菜单
        return Vector<MenuItem*>{btnMinus, btnPlus};
        };

    // 使用上面的通用函数，创建两排控制器
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

    // 把所有按钮加到一个 Menu 里
    auto volMenu = Menu::create();
    for (auto item : musicItems) volMenu->addChild(item); // 音乐按钮
    for (auto item : effectItems) volMenu->addChild(item);// 音效按钮

    volMenu->setPosition(Vec2::ZERO);
    bg->addChild(volMenu);

    // 4. 继续游戏 (Resume)
    auto btnResumeLabel = Label::createWithTTF("Resume", "fonts/GROBOLD.ttf", 30);
    btnResumeLabel->enableOutline(Color4B::BLACK, 2);
    // 按钮回调处理 关闭设置菜单
    auto btnResume = MenuItemLabel::create(btnResumeLabel, [=](Ref*) {

        PlayerData::getInstance()->playEffect("Audio/click.mp3");

        if (_settingsLayer) {  // 如果打开设置菜单
            _settingsLayer->removeFromParent(); // 关闭设置菜单
            _settingsLayer = nullptr; // 指针置空
        }
        });

    // 放在底部偏上
    btnResume->setPosition(Vec2(bg->getContentSize().width / 2, 110));

    // 组装btnResume菜单
    auto bottomMenu = Menu::create(btnResume, nullptr);
    bottomMenu->setPosition(Vec2::ZERO);
    bg->addChild(bottomMenu);

    // 弹出入场动画
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}