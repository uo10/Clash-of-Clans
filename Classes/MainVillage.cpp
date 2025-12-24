#include "MainVillage.h"
#include "SimpleAudioEngine.h"
#include "Barbarian.h"   
#include "Archer.h"
#include "Giant.h"
#include "WallBreaker.h" 
#include "game_map.h"


USING_NS_CC;
Scene* MainVillage::createScene()
{
	return MainVillage::create();
}
bool MainVillage::isTileBlock(Vec2 tileCoord)
{
    // 1、获取地图图层 
	//检查背景层
    auto layer = main_village_map_->getLayer("MainBackGround"); 
    if (!layer) return false;
	// 检查前景层
    auto layer1 = main_village_map_->getLayer("MainForeGround");
    if (!layer1) return false;

    // 2、获取该网格坐标下的 GID (全局图块ID)
    int gid = layer->getTileGIDAt(tileCoord);
	//前景层
    int gid1 = layer1->getTileGIDAt(tileCoord);
    // 如果 GID 为 0，说明这地方是空的（没有图块），可通行
    if (gid == 0 && gid1 == 0) return false;

    // 3、查询该 GID 的属性
    Value properties = main_village_map_->getPropertiesForGID(gid);
    //前景层
    Value properties1 = main_village_map_->getPropertiesForGID(gid1);

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
	is_dragging_ = false;// 初始化未在拖拽状态
	is_click_valid_ = false;// 初始化点击无效
    selected_sprite_path_ = "R-C.jpg";//默认选择图片
    //_selectedBuildingType = BuildingType::GOLD_MINE;
    selected_building_type_ = BuildingType::kNone;
    this->scheduleUpdate();

	// ================  设置初始化地图  ======================
    // 
	//获取屏幕可见大小和原点位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    Director::getInstance()->getTextureCache()->removeAllTextures();

	// 1、加载 TMX 地图
     main_village_map_ = TMXTiledMap::create("MainVillage1.tmx");

    Size mapSize = main_village_map_->getMapSize(); // 图块数量
    Size tileSize = main_village_map_->getTileSize(); // 单个图块像素 

    // 2、计算地图实际像素宽度和高度
    float mapPixelWidth = mapSize.width * tileSize.width;
    float mapPixelHeight = mapSize.height * tileSize.height;

    // 3、计算缩放因子
    float scaleX = visibleSize.width / mapPixelWidth;
    float scaleY = visibleSize.height / mapPixelHeight;

    // 4、设置缩放
    main_village_map_->setScaleX(scaleX);
    main_village_map_->setScaleY(scaleY);

    // ==================== UI音乐初始化 =========================
    // 创立Icon右上角 UI
    CreateResourceUi();
    // 建筑选择菜单
    CreateBuildUi();
    // 播放音乐
    PlayerData::GetInstance()->PlayBgm("Audio/bgm_village.mp3", true);

    // ====================最左上角建筑主按钮========================
    auto itemBuild = MenuItemImage::create(
        "Icon/itemBuild.png", 
        "Icon/itemBuild.png", 
        [=](Ref* sender) {

            PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

            // 点击逻辑保持不变
            if (build_menu_node_) {
                bool isVisible = build_menu_node_->isVisible();
                build_menu_node_->setVisible(!isVisible);

                if (!build_menu_node_->isVisible()) {
                    this->CloseBuildingMenu();
                }
            }
        }
    );
    itemBuild->setScale(0.12f);
    auto mainMenu = Menu::create(itemBuild, nullptr);
    mainMenu->setPosition(Vec2(75, visibleSize.height - 70)); // 左上角
    this->addChild(mainMenu, 1000); // 加到 this，层级高

    // ====================最左下角战斗主按钮========================
    this->CreateAttackUi();

    // ===================== 右下角设置按钮 ========================

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
    btnSettings->setPosition(Vec2(visibleSize.width - 50, 50));

    // 5. 添加到菜单
    auto settingsMenu = Menu::create(btnSettings, nullptr);
    settingsMenu->setPosition(Vec2::ZERO);
    this->addChild(settingsMenu, 2000); // UI层级，保证在最上层
    // ==================== 恢复之前的存档 ========================
    this->RestoreVillageData();

	//=============================================================
    auto mouseListener = EventListenerMouse::create();

    // 监听器用来监听刷新最大资源量信号
    auto refreshListener = EventListenerCustom::create("REFRESH_MAX_CAPACITY", [=](EventCustom* event) {
        CCLOG("收到刷新指令，开始统计全局容量...");
        this->RefreshTotalCapacity();
        });

    _eventDispatcher->addEventListenerWithSceneGraphPriority(refreshListener, this);

	// ==================== 鼠标操作  ==================
   
    // =================  一、滚轮缩放  ==================
    mouseListener->onMouseScroll = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
        auto map = main_village_map_;
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
    // ------ 判断鼠标是否点到了右键菜单 ------
    auto isMouseOnMenu = [=](Vec2 mousePos) -> bool {
        if (active_menu_node_ ) {
            // --- 情况 A: 建筑操作菜单 ---
            auto activemenu = active_menu_node_->getChildByName("BuildingMenu");
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
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
                else {
                    // 循环结束了，说明所有按钮都没点中 -> 关闭菜单，并拦截事件
                    this->CloseBuildingMenu();
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
            }
            // --- 情况 B: 建筑信息弹窗 ---
            auto infoBg = active_menu_node_->getChildByName("InfoBackground");
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
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    this->CloseBuildingMenu();
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
            }
            // --- 情况 C: 兵营士兵窗口 ---
            auto info = active_menu_node_->getChildByName("TrainPanel");
            if (info) {
                Vec2 localPos = info->convertToNodeSpace(mousePos);

                Size s = info->getContentSize();
                Rect bgRect = Rect(0, 0, s.width, s.height);

                // 判断鼠标是否在背景板范围内
                if (bgRect.containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
                else {
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    this->CloseBuildingMenu();
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
            }
        }
        // --- 情况D：建筑选择菜单 ---
        if (build_menu_node_ && build_menu_node_->isVisible()) {
            auto bg = build_menu_node_->getChildByName("BuildPanelBG");
            if (bg) {
                Vec2 localPos = bg->convertToNodeSpace(mousePos);
                // 判断鼠标是否在背景板范围内
                if (bg->getBoundingBox().containsPoint(localPos)) {
                    // 点在面板上 -> 拦截
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
                else {
                    // 点在面板外面 -> 关闭面板，并拦截这次点击
                    build_menu_node_->setVisible(false);
                    is_dragging_ = false;
                    is_click_valid_ = false;
                    return true;
                }
            }
        }
        // --- 情况E：有遮护罩(如选择关卡和设置) ---
        if (settings_layer_) {
            is_dragging_ = false;
            is_click_valid_ = false;
            return true; 
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
            is_dragging_ = false;
            return; // 点在菜单上了，忽略拖拽
        }
        // 如果点在右键菜单上，直接结束，_isClickValid = false
        if (isMouseOnMenu(mousePos)) {
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
        auto map = main_village_map_;
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
            if (is_click_valid_) {
                
                if (map) {                   
					// 1、获取点击在地图内部的坐标
                    Vec2 nodePos = map->convertToNodeSpace(mousePos);

					// 2、计算对应的瓦片坐标                
                    int tileX = static_cast<int>(nodePos.x / tileSize.width);
                    int tileY = static_cast<int>(mapSize.height - (nodePos.y / tileSize.height));

                    // 如果当前没有选中要建造的类型，直接返回，不执行建造逻辑
                    if (selected_building_type_ == BuildingType::kNone) {
                        is_dragging_ = false;
                        return;
                    }

                    // 3、边界检查
                    if (tileX >= 0 && tileX < mapSize.width && tileY >= 0 && tileY < mapSize.height) {
						Vec2 targetCoord = Vec2(tileX, tileY);// 目标瓦片坐标
						// 检查该瓦片是否已被占用
                        std::string key = StringUtils::format("%d_%d", tileX, tileY);
                        if (occupied_tiles_.find(key) != occupied_tiles_.end() && occupied_tiles_[key] == true) {
                            CCLOG("Tile is already occupied by another sprite!");
                            is_dragging_ = false;
                            // 直接返回，不执行后面的放置代码
                            return;
                        }
						// 检查该瓦片是否为障碍物
                        if (isTileBlock(targetCoord)) {
                            is_dragging_ = false;
                            CCLOG("Blocked! Cannot place item on water or mountain.");                        
                            return;
                        }
                        // 4、创建精灵
                        BaseBuilding* newBuilding = nullptr; 

                        // 判断建筑类型，如果是矿或采集器，创建 ResourceProducer
                        if (selected_building_type_ == BuildingType::kGoldMine ||
                            selected_building_type_ == BuildingType::kElixirPump) {
                            newBuilding = ResourceProducer::Create(selected_building_type_, 1);
                        }
                        //如果是存储类
                        else if (selected_building_type_ == BuildingType::kGoldStorage ||
                            selected_building_type_ == BuildingType::kElixirStorage ||
                            selected_building_type_ == BuildingType::kBarracks) {
                            // 创建新的存储类指针
                            ResourceStorage* storageBuilding = ResourceStorage::Create(selected_building_type_, 1);
                            // 加入到存储类建筑列表
                            storage_list_.push_back(storageBuilding);
                            // 转化为父类
                            newBuilding = storageBuilding;
                            // 刷新上限
                            this->RefreshTotalCapacity();
                        }
                        //其他建筑类型
                        else {
                                if (selected_building_type_ == BuildingType::kWall) {
                                    newBuilding = Wall::create(1);
                                }
                                if (selected_building_type_ == BuildingType::kCannon) {
                                    newBuilding = Cannon::create(1);
                                }
                                if (selected_building_type_ == BuildingType::kArcherTower) {
                                    newBuilding = ArcherTower::create(1);
                                }
                                if (selected_building_type_ == BuildingType::kTownHall) {
                                    newBuilding = TownHall::create(1); 
                                }
                        }
                        if (newBuilding) {
                            // ================= 一、根据大本营判断是否可以建造 ==============
                            // --- a. 获取当前大本营等级 ---
                            int currentTHLevel = 0;
                            for (auto child : map->getChildren()) {
                                // 因为所有建筑 Tag 都是 999，我们要区分出谁是 TownHall
                                if (child->getTag() == 999) {
                                    // 使用 dynamic_cast 尝试转换
                                    TownHall* th = dynamic_cast<TownHall*>(child);
                                    if (th) {
                                        currentTHLevel = th->level_;
                                        break; // 找到了，退出循环
                                    }
                                }
                            }
                            // 获取该大本营的最大建筑容量
                            int maxLimit = TownHall::getMaxBuildingCount(selected_building_type_, currentTHLevel);

                            // --- b. 统计当前地图上该类建筑的数量 ---
                            int currentCount = 0;
                            for (auto child : map->getChildren()) {
                                if (child->getTag() == 999) {
                                    BaseBuilding* b = dynamic_cast<BaseBuilding*>(child);
                                    // 统计同类型且未被摧毁的建筑
                                    if (b && b->type_ == selected_building_type_ && b->state_ != BuildingState::kDestroyed) {
                                        currentCount++;
                                    }
                                }
                            }

                            // --- c. 判断是否超限 ---
                            if (currentCount >= maxLimit) {
                                CCLOG("建造失败：数量已达上限！当前: %d / %d (大本营 Lv.%d)", currentCount, maxLimit, currentTHLevel);
                                is_dragging_ = false;
                                return; // 拦截，不准建造
                            }

                            // =============== 二、根据造价判断是否可建造 并且执行建造逻辑 ==============
                            // a. 获取该建筑的造价
                            int costGold = newBuilding->build_cost_gold_;
                            int costElixir = newBuilding->build_cost_elixir_;

                            // b. 尝试扣费
                            if (PlayerData::GetInstance()->ConsumeGold(costGold)&& PlayerData::GetInstance()->ConsumeElixir(costElixir) ){
                                // === 扣费成功，执行建造逻辑 ===
								// 播放建造音效
                                PlayerData::GetInstance()->PlayEffect("Audio/plop.mp3");
                                // 刷新 UI 显示扣钱后的结果
                                this->UpdateResourceUi();
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
                                occupied_tiles_[key] = true;

                                // 开始建造流程 (显示进度条，进入 BUILDING 状态)
                                newBuilding->StartConstruction();
                                CCLOG("Sprite placed at tile (%d, %d)", tileX, tileY);
                            }
                            else {
                                // === 钱不够，取消建造 ===

                                CCLOG("资源不足！需要: 金币%d, 圣水%d，但你只有: 金币%d, 圣水%d",
                                    costGold, costElixir,
                                    PlayerData::GetInstance()->GetGold(),
                                    PlayerData::GetInstance()->GetElixir());
                                // 如果刚才 push_back 进了 _storageList，现在要把它弹出来
                                if (selected_building_type_ == BuildingType::kGoldStorage ||
                                    selected_building_type_ == BuildingType::kElixirStorage) {
                                    if (!storage_list_.empty() && storage_list_.back() == newBuilding) {
                                        storage_list_.pop_back(); // 还没建成就不算进容量
                                        this->RefreshTotalCapacity(); // 刷新回滚容量
                                    }
                                }
                            }
                        }
                        if (selected_building_type_ != BuildingType::kWall) { // 除了围墙
                            // 清空非法区域
                            selected_building_type_ = BuildingType::kNone;
                            UpdateOccupiedGridVisual();
                            current_preview_building_ = nullptr; // 取消预览
                        }
                    }
                }
            }

            // 结束拖拽状态
            is_dragging_ = false;
        }
		// 3、如果是右键松开
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {

            // A. 如果有预览建筑，右键表示“取消建造”
            if (current_preview_building_) {
                CCLOG("取消建造");

                // 销毁预览体
                current_preview_building_->removeFromParent();
                current_preview_building_ = nullptr;

                // 重置状态
                selected_building_type_ = BuildingType::kNone;
                UpdateOccupiedGridVisual(); // 关闭非法区域显示

                return; // 直接返回，不触发下面的“查看信息”逻辑
            }

            // B. 如果没有预览建筑      
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
                            if (building->state_ == BuildingState::kIdle) {
                                CCLOG("右键选中了建筑，弹出操作菜单");
                                this->ShowBuildingMenu(building);
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
        EventMouse* e = (EventMouse*)event;
        auto map = main_village_map_;
        // 获取当前鼠标位置
        Vec2 current_mouse_pos = Vec2(e->getCursorX(), e->getCursorY());
        
        // A. --- 处理拖拽状态 （只有当“正在拖拽”状态为 true 时才移动地图）---
        if (is_dragging_) {

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
       // B: --- 建筑预览跟随 --- 
       // 如果当前有正在预览的建筑指针
        if (current_preview_building_) {

            // 1. 转换坐标系：将屏幕坐标转为地图内部节点坐标
            Vec2 node_pos = map->convertToNodeSpace(current_mouse_pos);
            Size tile_size = map->getTileSize();
            Size map_size = map->getMapSize();

            // 2. 计算鼠标当前所在的瓦片索引
            int tile_x = static_cast<int>(node_pos.x / tile_size.width);
            int tile_y = static_cast<int>(map_size.height - (node_pos.y / tile_size.height));

            // 3. 获取建筑尺寸 
            if (current_preview_building_->getContentSize().width <= 0) {
                current_preview_building_->UpdateView();
            }
            // 缩放成一个瓦片的大小
            Size sprite_size = current_preview_building_->getContentSize();
            Size target_size = tile_size; 

            if (sprite_size.width > 0 && sprite_size.height > 0) {
                // 计算宽高比
                float scale_x = target_size.width / sprite_size.width;
                float scale_y = target_size.height / sprite_size.height;
 
                float final_scale = std::min(scale_x, scale_y) * 0.9f; // 比原图略小一些

                current_preview_building_->setScale(final_scale);
            }

            // 4. 计算吸附网格后的像素坐标
            float final_x = tile_x * tile_size.width + tile_size.width / 2.0f;

            float final_y = (map_size.height - 1 - tile_y) * tile_size.height + tile_size.height / 2.0f;

            // 设置位置
            current_preview_building_->setPosition(Vec2(final_x, final_y));
            current_preview_building_->setAnchorPoint(Vec2(0.5f, 0.5f));

            // 5. 颜色检测 (检测是否可以放置)
            bool can_build = true;

            // a. 越界检查
            if (tile_x < 0 || tile_x >= map_size.width || tile_y < 0 || tile_y >= map_size.height) {
                can_build = false;
            }
            // b. 占用检查
            std::string key = StringUtils::format("%d_%d", tile_x, tile_y);
            if (occupied_tiles_.count(key) && occupied_tiles_[key] == true) {
                can_build = false;
            }
            // c. 障碍物检查
            if (isTileBlock(Vec2(tile_x, tile_y))) {
                can_build = false;
            }

            // 6. 设置颜色 (绿/红)
            if (can_build) {
                current_preview_building_->setColor(Color3B(100, 255, 100)); // 绿
            }
            else {
                current_preview_building_->setColor(Color3B(255, 100, 100)); // 红
            }
        }

        };
    // ================== 五、添加监听器捕获鼠标操作 ==================
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

	this->addChild(main_village_map_, 0);
    return true;
}
void MainVillage::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

void MainVillage::CreateResourceUi() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 金币 UI (右上角)
    auto goldIcon = Sprite::create("Icon/Gold.png"); 
    goldIcon->setPosition(visibleSize.width - 200, visibleSize.height - 40);
    this->addChild(goldIcon, 9999);

    gold_label_ = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    gold_label_->setColor(Color3B(255, 215, 0)); //金色
    gold_label_->enableOutline(Color4B::BLACK, 2); 
    gold_label_->setAnchorPoint(Vec2(0, 0.5));
    gold_label_->setPosition(visibleSize.width - 170, visibleSize.height - 40);
    this->addChild(gold_label_, 9999);

    // 2. 圣水 UI (金币下方)
    auto elixirIcon = Sprite::create("Icon/Elixir.png"); 
    elixirIcon->setPosition(visibleSize.width - 200, visibleSize.height - 80);
    this->addChild(elixirIcon, 9999);

    elixir_label_ = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    elixir_label_->setColor(Color3B(255, 80, 255)); // 紫色
    elixir_label_->enableOutline(Color4B::BLACK, 2);
    elixir_label_->setAnchorPoint(Vec2(0, 0.5));
    elixir_label_->setPosition(visibleSize.width - 170, visibleSize.height - 80);
    this->addChild(elixir_label_, 9999);

    // 3. 人口 UI (圣水下方)
    auto peopleIcon = Sprite::create("Icon/People.png");
    peopleIcon->setPosition(visibleSize.width - 200, visibleSize.height - 120);
    this->addChild(peopleIcon, 9999);

    people_label_ = Label::createWithTTF("0", "fonts/GROBOLD.ttf", 24);
    people_label_->enableOutline(Color4B::BLACK, 2);
    people_label_->setAnchorPoint(Vec2(0, 0.5));
    people_label_->setPosition(visibleSize.width - 170, visibleSize.height - 120);
    this->addChild(people_label_, 9999);

    // 3. 初始刷新一次
    UpdateResourceUi();

    // 4. 注册监听器：当全局发出 "REFRESH_UI" 信号时，自动刷新文字
    auto listener = EventListenerCustom::create("REFRESH_UI", [=](EventCustom* event) {
        this->UpdateResourceUi();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainVillage::UpdateResourceUi() {
    // 获取最新资源值
    int gold = PlayerData::GetInstance()->GetGold();
    int elixir = PlayerData::GetInstance()->GetElixir();
    int people = PlayerData::GetInstance()->GetPeople();

    // 更新资源label
    if (gold_label_) gold_label_->setString(std::to_string(gold));
    if (elixir_label_) elixir_label_->setString(std::to_string(elixir));
    if (people_label_) people_label_->setString(std::to_string(people));
}

void MainVillage::RefreshTotalCapacity()
{
    // 设定初始资源值
    long long totalGoldCapacity = 20000000;
    long long totalElixirCapacity = 20000000;
    long long totalPeopleCapacity = 0;

    // 1. 遍历容器中的每一个建筑
    for (auto building : storage_list_) {
        // 确保是建筑类
        if (building) {
            // 根据类型累加对应的种类资源
            if (building->type_ == BuildingType::kGoldStorage) {
                totalGoldCapacity += building->max_limit_;
            }
            else if (building->type_ == BuildingType::kElixirStorage) {
                totalElixirCapacity += building->max_limit_;
            }
            else if (building->type_ == BuildingType::kBarracks) {
                totalPeopleCapacity += building->max_limit_;
            }
        }
    }
    // 2. 更新资源总数
    PlayerData::GetInstance()->UpdateMaxLimits(totalGoldCapacity, totalElixirCapacity, totalPeopleCapacity);
    CCLOG("=== 刷新完成 ===");
    CCLOG("当前金币总上限: %lld", totalGoldCapacity);
    CCLOG("当前圣水总上限: %lld", totalElixirCapacity);
    CCLOG("当前人口总上限: %lld", totalPeopleCapacity);
}

void MainVillage::CloseBuildingMenu() {
    if (active_menu_node_) { // 如果有菜单打开
        active_menu_node_->removeFromParent(); // 关闭当前菜单
        active_menu_node_ = nullptr;
    }
}

void MainVillage::ShowBuildingMenu(BaseBuilding* building) {
    //======================= 1. 如果还有未关闭的菜单 ====================
    if (active_menu_node_ != nullptr) {
        CloseBuildingMenu();
    }
    if (!building) return;

    //======================= 2. 创建一个节点作为菜单容器 ================
    active_menu_node_ = Node::create();
    // 1. 把菜单放在建筑的头顶 
    active_menu_node_->setPosition(building->getPosition() + Vec2(0, 50));

    float map_scale_x = main_village_map_->getScaleX();
    float map_scale_y = main_village_map_->getScaleY();

    active_menu_node_->setScaleX(1.0f / map_scale_x);
    active_menu_node_->setScaleY(1.0f / map_scale_y);
    active_menu_node_->setTag(888); // 特殊Tag，区别于建筑
    main_village_map_->addChild(active_menu_node_, 10000); // 保证最高层

    // 2. 定义函数：创建图标按钮 
    auto createIconBtn = [](std::string img_name, std::string text, const ccMenuCallback& callback) -> MenuItem* {

        // 1. 创建按钮容器  点击区 - 80x80
        auto wrapper = Node::create();
        wrapper->setContentSize(Size(80, 80));
        wrapper->setAnchorPoint(Vec2(0.5, 0.5));

        // 2. 创建建筑图标并放缩
        auto sprite = Sprite::create(img_name);
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

            PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

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
    auto btnInfo = createIconBtn("Icon/btn_info.png", "Info", [=](Ref* sender) {

        // 1. 先关闭原来的操作菜单
        this->CloseBuildingMenu();

        // 2. 创立一个菜单根节点
        active_menu_node_ = Node::create();
        // 设置反向缩放 
        float mapScaleX = main_village_map_->getScaleX();
        float mapScaleY = main_village_map_->getScaleY();
        active_menu_node_->setScaleX(1.0f / mapScaleX);
        active_menu_node_->setScaleY(1.0f / mapScaleY);

        // 定位到建筑位置
        active_menu_node_->setPosition(building->getPosition());

        // 加到地图上，层级极高
        main_village_map_->addChild(active_menu_node_, 20000);

        // 3. 创建背景板 
        float bgWidth = 300;
        float bgHeight = 200;

        // A. 准备图片路径
        std::string bgImg = "Icon/info_panel.png"; 

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

        active_menu_node_->addChild(bg);
        // 4. 显示通用信息 (名字、等级、HP)
        std::string nameStr = building->name_;

        std::string commonText = StringUtils::format(
            "Name: %s\nLevel: %d\nHP: %d / %d",
            nameStr.c_str(),
            building->level_,
            (int)building->current_hp_,
            (int)building->max_hp_
        );

        // 设置通用信息label
        auto labelCommon = Label::createWithTTF(commonText, "fonts/GROBOLD.ttf", 20); 
        labelCommon->setColor(Color3B(65, 105, 225)); // 蓝色
        labelCommon->setPosition(bgWidth / 2, bgHeight / 2 + 20);
        bg->addChild(labelCommon);

        // 5. 显示特殊信息 (根据类型判断)
        std::string specialText = "";
        BuildingStats stats = BaseBuilding::GetStatsConfig(building->type_, building->level_);//获取建筑信息
        if (building->type_ == BuildingType::kGoldMine || building->type_ == BuildingType::kElixirPump) {
            specialText = StringUtils::format("Production: %d / hour", stats.production_rate);
        }
        else if (building->type_ == BuildingType::kCannon || building->type_ == BuildingType::kArcherTower) {
            specialText = StringUtils::format("Damage: %.1lf ", stats.damage);
        }
        else if (building->type_ != BuildingType::kWall && building->type_ != BuildingType::kTownHall) {
            specialText = StringUtils::format("Capacity: %d", stats.capacity);
        }

        if (!specialText.empty()) { // 如果不为空即存在特殊属性
            // 设置特殊属性label
            auto labelSpecial = Label::createWithTTF(specialText, "fonts/GROBOLD.ttf", 20);
            labelSpecial->setColor(Color3B::MAGENTA); // 特殊属性用洋红色
            labelSpecial->setPosition(bgWidth / 2, bgHeight / 2 - 40);
            bg->addChild(labelSpecial);
        }

        // 6. 添加关闭按钮
        auto closeLabel = Label::createWithTTF("[ Close ]", "fonts/GROBOLD.ttf", 24);
        closeLabel->setColor(Color3B(255, 50, 50)); // 红色
        closeLabel->enableOutline(Color4B::BLACK, 1); // 黑边
        auto closeItem = MenuItemLabel::create(closeLabel, [=](Ref* sender) {

            PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

            this->CloseBuildingMenu();
            });
        closeItem->setPosition(bgWidth - 60, bgHeight - 20); // 放置在右上角位置

        closeLabel->setColor(Color3B(255, 50, 50));
        closeLabel->enableBold();         // 描边 

        auto menu = Menu::create(closeItem, nullptr); // 组装进菜单
        menu->setPosition(Vec2::ZERO);
        bg->addChild(menu);

        // 7. 弹出动画
        bg->setScale(0);
        bg->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));

        CCLOG("信息展示：");
        });

    // --- 按钮 2: 升级 (Upgrade) ---
    auto btnUpgrade = createIconBtn("Icon/btn_upgrade.png", "Upgrade", [=](Ref* sender) {
        CCLOG("点击了升级按钮");

        // --- 扣费升级逻辑 ---
        int next_level = building->level_ + 1;

        // 大本营等级限制
        // 1. 寻找当前地图上的大本营等级
        int currentTHLevel = 1;
        for (auto child : main_village_map_->getChildren()) {
            if (child->getTag() == 999) {
                auto th = dynamic_cast<TownHall*>(child);
                if (th) {
                    currentTHLevel = th->level_;
                    break;
                }
            }
        }

        // 2. 检查是否允许升级
        if (!TownHall::isUpgradeAllowed(building->type_, next_level, currentTHLevel)) {
            CCLOG("升级失败：需要大本营 Lv.%d 才能升级到 Lv.%d", currentTHLevel + 1, next_level); // 如果满足限制规定
            return; // 直接拦截，不扣钱
        }

        BuildingStats next_stats = BaseBuilding::GetStatsConfig(building->type_, next_level);//获取下一级配置
        // 获取升级花费
        int upgrade_cost_gold = next_stats.cost_gold;
        int upgrade_cost_elixir = next_stats.cost_elixir;
        if (PlayerData::GetInstance()->ConsumeGold(upgrade_cost_gold) && PlayerData::GetInstance()->ConsumeElixir(upgrade_cost_elixir)) {
            building->StartUpgradeProcess(); // 扣钱成功，开始升级
            this->UpdateResourceUi(); // 刷新右上角钱数

            //关闭菜单
            this->CloseBuildingMenu();
        }
        else {
            CCLOG("金币不足，无法升级！");
        }
        });

    // --- 按钮 3: 训练 (Train) - 仅兵营 ---
    MenuItem* btn_train = nullptr; // 因为不确定是否有兵营 初始设为空
    if (building->type_ == BuildingType::kBarracks) { // 如果是兵营
        btn_train = createIconBtn("Icon/btn_train.png", "Train", [=](Ref* sender) {
            // 1. 清空当前菜单
            if (active_menu_node_) {
                active_menu_node_->removeAllChildren();
            }
            else {
                return;
            }

            // 2. 创建训练面板背景
            float panel_w = 400;
            float panel_h = 140;
            std::string bg_path = "TrainingBg.png";   // 准备图片路径 
            Rect cap_insets = Rect(20, 20, 60, 60); // 创建九宫格背景

            auto bg = ui::Scale9Sprite::create(bg_path);
            bg->setCapInsets(cap_insets); // 使用九宫格设计
            bg->setContentSize(Size(panel_w, panel_h)); // 拉伸到指定大小
            bg->setName("TrainPanel");   // 设定Tag，用于后续鼠标点击测试

            bg->setAnchorPoint(Vec2(0.5, 1));
            bg->setPosition(Vec2(0, -200)); // 放在建筑下方 

            active_menu_node_->addChild(bg); // 放进 _activeMenuNode的孩子节点

            // 3. 生成士兵选择按钮
            auto create_troop_node = [=](TroopInfo info, int index) -> Node* {

                // 创建按钮容器
                auto container = Node::create();
                container->setContentSize(Size(70, 90));

                // --- A. 人物头像大按钮 ---
                // a. 设置按钮容器大小
                auto icon_wrapper = Node::create();
                icon_wrapper->setContentSize(Size(60, 60)); // 使用Wrapper包裹按钮大小
                icon_wrapper->setAnchorPoint(Vec2(0.5, 0.5)); // 放在中心位置

                // b. 设置照片并放缩
                auto sprite = Sprite::create(info.img); // 获取人物照片

                float s = 60.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
                sprite->setScale(s); // 进行放缩
                sprite->setPosition(30, 30); // 放在 Wrapper 的正中心
                icon_wrapper->addChild(sprite);
                // c. 增加按钮逻辑
                auto btn_add = MenuItemSprite::create(icon_wrapper, nullptr, [=](Ref*) {
                    // 判断圣水和人口消耗
                    if (PlayerData::GetInstance()->ConsumeElixir(info.cost) && PlayerData::GetInstance()->AddPeople(info.weight, info.cost)) {
                        // （1）+1 逻辑
                        PlayerData::GetInstance()->AddTroop(info.name, 1);

                        PlayerData::GetInstance()->PlayEffect("Audio/plop.mp3");

                        // （2）动画 
                        sprite->stopAllActions();
                        sprite->runAction(Sequence::create(ScaleTo::create(0.05f, s * 1.1f), ScaleTo::create(0.05f, s), nullptr));

                        // （3）刷新 UI
                        auto count_lab = dynamic_cast<Label*>(container->getChildByTag(101));
                        auto menu = dynamic_cast<Menu*>(container->getChildByTag(102));
                        if (count_lab) count_lab->setString(StringUtils::format("x%d", PlayerData::GetInstance()->GetTroopCount(info.name)));
                        if (menu) {
                            auto min_item = menu->getChildByTag(200);
                            if (min_item) min_item->setVisible(true);
                        }
                        CCLOG("增加: %s", info.name.c_str());

                        // （4）可视化处理
                        Soldier* new_soldier = nullptr;
                        // 根据名字判断创建哪种兵
                        if (info.name == "Barbarian") {
                            new_soldier = Barbarian::create();
                        }
                        else if (info.name == "Archer") {
                            new_soldier = Archer::create();
                        }
                        else if (info.name == "Giant") {
                            new_soldier = Giant::create();
                        }
                        else if (info.name == "WallBreaker") { 
                            new_soldier = WallBreaker::create();
                        }

                        if (new_soldier) {

                            new_soldier->setHomeMode(true); // // 开启游走
                            new_soldier->setHomePosition(building->getPosition());

                            // 1. 确定“出生点” (兵营大门位置)
                            Vec2 door_pos = building->getPosition() + Vec2(0, -20);

                            // 2. 计算“目标点” (扇形随机分布)
                            float min_angle = CC_DEGREES_TO_RADIANS(210);
                            float max_angle = CC_DEGREES_TO_RADIANS(330);

                            // 随机角度
                            float random_angle = min_angle + (max_angle - min_angle) * CCRANDOM_0_1();

                            // 随机距离 (半径)：距离兵营中心 50 到 90 像素之间
                            float min_radius = 50.0f;
                            float max_radius = 90.0f;
                            float random_radius = min_radius + (max_radius - min_radius) * CCRANDOM_0_1();

                            // 极坐标转笛卡尔坐标：计算偏移量
                            float offset_x = cos(random_angle) * random_radius;
                            float offset_y = sin(random_angle) * random_radius;

                            // 最终待机位置
                            Vec2 target_pos = building->getPosition() + Vec2(offset_x, offset_y);

                            // 3. 设置初始状态
                            new_soldier->setPosition(door_pos);    // 先把兵放在门口                      
                            new_soldier->setScale(0.2f);    // 且设为极小（刚训练出来）

                            // 根据目标点在左边还是右边，调整朝向
                            if (target_pos.x < door_pos.x) {
                                // 如果目标在左边，需要翻转 
                               new_soldier->setScaleX(-0.01f); // 用负 Scale 实现翻转
                            }

                            // 4. 添加到地图层 
                            main_village_map_->addChild(new_soldier, 3000 - static_cast<int>(door_pos.y));

                            // 5. 出场动作
                            float jump_height = 20.0f;
                            float duration = 0.5f;

                            // 动作A: 跳跃移动到目标点
                            auto jump_act = JumpTo::create(duration, target_pos, jump_height, 1);

                            // 动作B: 变大恢复正常 
                            float target_scale = 0.2f; // 标准大小
                            if (target_pos.x < door_pos.x) target_scale = -0.2f; // 保持翻转

                            auto scale_act = ScaleTo::create(duration, fabs(target_scale), fabs(target_scale));

                            // 组合动作
                            auto spawn_seq = Sequence::create(
                                Spawn::create(jump_act, scale_act, nullptr),
                                CallFunc::create([=]() {
                                    // 落地后，更新 Z轴高度 防止遮挡
                                    new_soldier->setLocalZOrder(3000 - static_cast<int>(target_pos.y));
                                    }),
                                nullptr
                            );

                            new_soldier->runAction(spawn_seq);

                            // 6. 加入到可视化士兵容器里
                            building->visual_troops_[info.name].push_back(new_soldier);

                            CCLOG("士兵实体 %s 扇形生成在 (%f, %f)", info.name.c_str(), target_pos.x, target_pos.y);
                        }
                    }
                    });
                btn_add->setPosition(35, 60); // 按钮在容器中的位置

                // --- B. 左上角减号按钮 ---
                // a. 设置按钮容器大小
                auto minus_wrapper = Node::create();
                minus_wrapper->setContentSize(Size(50, 50)); // 50*50点击区域
                minus_wrapper->setAnchorPoint(Vec2(0.5, 0.5));

                // b. 减号label
                auto min_lbl = Label::createWithSystemFont("-", "Arial", 60);
                min_lbl->setColor(Color3B::RED);
                min_lbl->enableOutline(Color4B::WHITE, 2);
                min_lbl->setPosition(15, 15); // 文字居中
                minus_wrapper->addChild(min_lbl);

                // c. 按钮回调逻辑
                auto btn_minus = MenuItemSprite::create(minus_wrapper, nullptr, [=](Ref*) {
                    // -1 逻辑
                    PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
                    if (PlayerData::GetInstance()->ConsumeTroop(info.name, 1)) {
                        PlayerData::GetInstance()->RemovePeople(info.weight);// 返还人口容量
                        auto count_lbl = dynamic_cast<Label*>(container->getChildByTag(101));// 更新数量标签
                        if (count_lbl) count_lbl->setString(StringUtils::format("x%d", PlayerData::GetInstance()->GetTroopCount(info.name)));

                        // --- 视觉删除逻辑 ---
                        // 1. 找到该兵种的列表
                        auto& soldier_list = building->visual_troops_[info.name];

                        if (!soldier_list.empty()) {
                            // 2. 取出最后生成的那个兵 
                            Node* soldier_to_remove = soldier_list.back();

                            // 3. 从列表中移除 
                            soldier_list.pop_back();

                            // 4. 执行删除动画
                            if (soldier_to_remove) {
                                soldier_to_remove->stopAllActions(); // 停止它当前的动作

                                auto delete_seq = Sequence::create(
                                    // （1）: 变红，提示玩家它被删除了
                                    TintTo::create(0.1f, 255, 0, 0),  
                                    // （2）: 同时缩小和淡出
                                    Spawn::create(
                                        ScaleTo::create(0.3f, 0.0f), // 缩小到没了
                                        FadeOut::create(0.3f)        // 变透明
                                    ),
                                    // （3）: 从父节点(_MainVillageMap)上删掉
                                    RemoveSelf::create(),
                                    nullptr
                                );

                                soldier_to_remove->runAction(delete_seq); // 执行删除动作
                            }
                        }
                        // 如果士兵的数量为0 隐藏减号
                        if (PlayerData::GetInstance()->GetTroopCount(info.name) <= 0) {
                            auto menu = dynamic_cast<Menu*>(container->getChildByTag(102));
                            auto min_item = menu->getChildByTag(200);
                            if (min_item) min_item->setVisible(false);
                        }
                        CCLOG("减少: %s", info.name.c_str());
                    }
                    });
                // d. 减号位置
                btn_minus->setPosition(10, 85); //人物头像的左上角
                btn_minus->setTag(200);

                // --- C. 组装 Menu ---
                auto local_menu = Menu::create(btn_add, btn_minus, nullptr);
                local_menu->setPosition(Vec2::ZERO);
                local_menu->setTag(102);
                container->addChild(local_menu);

                // --- D. 标签 (右上角) ---
                int current_count = PlayerData::GetInstance()->GetTroopCount(info.name); // 获取该人物的数量
                auto count_lbl = Label::createWithSystemFont(StringUtils::format("x%d", current_count), "Arial", 16);// 创建对应的label并更新 
                count_lbl->setColor(Color3B::GREEN);
                count_lbl->setAnchorPoint(Vec2(1, 0.5));
                count_lbl->setPosition(68, 90); // 右上角位置
                count_lbl->setTag(101);
                container->addChild(count_lbl);

                // 初始隐藏减号
                btn_minus->setVisible(current_count > 0);

                // --- E. 名字与权重 ---
                auto weight_lbl = Label::createWithSystemFont(StringUtils::format("%d", info.weight), "Arial", 14);
                weight_lbl->setColor(Color3B::YELLOW);
                weight_lbl->setAnchorPoint(Vec2(1, 0));
                weight_lbl->setPosition(65, 10);
                container->addChild(weight_lbl);

                auto name_lbl = Label::createWithSystemFont(info.name, "Arial", 14);
                name_lbl->setPosition(35, 5);
                container->addChild(name_lbl);

                // --- F. 消耗圣水值 ---
                auto cost_lbl = Label::createWithSystemFont(StringUtils::format("%d", info.cost), "Arial", 14);
                cost_lbl->setColor(Color3B::GREEN);
                cost_lbl->setAnchorPoint(Vec2(1, 0));
                cost_lbl->setPosition(45, -20);
                container->addChild(cost_lbl);

                return container;
                };

            // 5. 循环创建不同士兵头像 并排列 
            float start_x = 30;
            float gap_x = 90;
            float pos_y = panel_h / 2 - 40;

            for (int i = 0; i < troops_.size(); ++i) { //troop.size确定士兵种数
                // 调用上面的 lambda函数创建
                auto troop_node = create_troop_node(troops_[i], i);
                troop_node->setPosition(start_x + i * gap_x, pos_y); 
                bg->addChild(troop_node);
            }

            // 6. 添加关闭按钮 (X)
            auto close_lbl = Label::createWithSystemFont("X", "Arial", 26);
            close_lbl->setColor(Color3B::RED);
            auto close_item = MenuItemLabel::create(close_lbl, [=](Ref*) {

                PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

                this->CloseBuildingMenu();
                });
            close_item->setPosition(panel_w - 20, panel_h - 20);

            auto close_menu = Menu::create(close_item, nullptr); // 加入菜单
            close_menu->setPosition(Vec2::ZERO);
            bg->addChild(close_menu);

            CCLOG("打开了兵营训练面板");
            });
    }
    // --- 按钮 4: 移除 (Remove) ---
    auto btn_remove = createIconBtn("Icon/btn_remove.png", "Remove", [=](Ref* sender) {
           CCLOG("点击了拆除按钮");

            // 1. 如果是存储建筑 总资源容量会发生改变
           if (building->type_ == BuildingType::kGoldStorage ||
               building->type_ == BuildingType::kElixirStorage ||
               building->type_ == BuildingType::kBarracks) {

                auto& list = this->storage_list_;
                auto it = std::find(list.begin(), list.end(), building);
                if (it != list.end()) {
                    list.erase(it);
                    this->RefreshTotalCapacity(); // 刷新上限
                    CCLOG("Storage removed from list.");
                }
            }

            // 2. 释放地图占用 
            Size tile_size = main_village_map_->getTileSize();
            Size map_size = main_village_map_->getMapSize();

            // 获取建筑当前在地图上的坐标
            Vec2 building_pos = building->getPosition();

            // 计算网格坐标
            int tile_x = static_cast<int>(building_pos.x / tile_size.width);
            int tile_y = static_cast<int>(map_size.height - (building_pos.y / tile_size.height));

            std::string key = StringUtils::format("%d_%d", tile_x, tile_y);

            // 将该位置标记为“未占用”
            occupied_tiles_[key] = false;
            CCLOG("Tile (%d, %d) freed.", tile_x, tile_y);

            // 3. 移除对象
            building->removeFromParent();
            // 4. 关闭菜单
            this->CloseBuildingMenu();
         });

    // ===================== 4.组装横向菜单 =====================
    Vector<MenuItem*> menu_items;

    // 信息 - 升级 - (训练) - 拆除
    menu_items.pushBack(btnInfo);
    menu_items.pushBack(btnUpgrade);
    // 如果有兵营 push进入兵营
    if (btn_train) {
        menu_items.pushBack(btn_train);
    }
    menu_items.pushBack(btn_remove);

    // 生成菜单
    auto menu = Menu::createWithArray(menu_items);
    menu->alignItemsHorizontallyWithPadding(10); //水平排列 间距为10
    menu->setName("BuildingMenu");
    menu->setPosition(Vec2::ZERO);
    active_menu_node_->addChild(menu);

    // 弹出来的入场动作
    active_menu_node_->setScale(0);
    active_menu_node_->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f / map_scale_x, 1.0f / map_scale_y)));
}

void MainVillage::CreateBuildUi() {

    // A. 创建容器背景
    auto visible_size = Director::getInstance()->getVisibleSize();
    // 1. 创建底部面板容器 
    build_menu_node_ = Node::create();
    build_menu_node_->setVisible(false);
    this->addChild(build_menu_node_, 1100);

    // 2. 创建背景
    float panel_height = 280;
    std::string bg_path = "Icon/TrainingBg.png"; // 准备图片路径
    Rect cap_insets = Rect(20, 20, 60, 60); // 设置可放缩区域

    auto bg = ui::Scale9Sprite::create(bg_path); // 九宫格绘制
    bg->setCapInsets(cap_insets);
    bg->setContentSize(Size(visible_size.width, panel_height));  // 宽度设为屏幕宽度，高度设为 panelHeight
    bg->setAnchorPoint(Vec2(0, 0));    // 锚点设为左下角 (0, 0)
    bg->setPosition(Vec2::ZERO);
    bg->setName("BuildPanelBG");    // 设置Tag，用于点击检测

    build_menu_node_->addChild(bg);

    // B. 创建顶部的分类页签 (资源 | 防御)
    // 1. 创建 Label 和 Item  
    auto label_res = Label::createWithTTF("RESOURCES", "fonts/GROBOLD.ttf", 26);
    label_res->enableOutline(Color4B::BLACK, 2); // 黑色描边
    auto item_res = MenuItemLabel::create(label_res, nullptr);

    auto label_def = Label::createWithTTF("DEFENSE", "fonts/GROBOLD.ttf", 26);
    label_def->enableOutline(Color4B::BLACK, 2);
    auto item_def = MenuItemLabel::create(label_def, nullptr);

    // 2. 设置 [资源] 按钮的回调
    item_res->setCallback([=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        // 视觉变化：自己变绿，别人变灰
        item_res->setColor(Color3B::GREEN);
        item_def->setColor(Color3B::GRAY);

        // 打开资源页面
        this->SwitchBuildCategory(0);
        });

    // 3. 设置 [防御] 按钮的回调
    item_def->setCallback([=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        // 视觉变化：自己变绿，别人变灰
        item_def->setColor(Color3B::GREEN);
        item_res->setColor(Color3B::GRAY);

        // 打开防御界面
        this->SwitchBuildCategory(1);
        });

    // 4. 设置初始状态 
    item_res->setColor(Color3B::GREEN); //(默认选中资源)
    item_def->setColor(Color3B::GRAY);  //(默认未选中资源)

    // 5. 放入菜单
    auto tab_menu = Menu::create(item_res, item_def, nullptr);
    tab_menu->alignItemsHorizontallyWithPadding(120);
    tab_menu->setPosition(visible_size.width / 2, panel_height - 35);
    build_menu_node_->addChild(tab_menu);

    // C. 创建内容容器 (存放建筑图标)
    icon_container_ = Node::create();
    icon_container_->setPosition(0, 0); 
    build_menu_node_->addChild(icon_container_);

    // 默认加载资源类
    SwitchBuildCategory(0);
}

void MainVillage::SwitchBuildCategory(int category) {
    if (!icon_container_) return; 

    // 1. 清空当前显示的图标
    icon_container_->removeAllChildren();

    auto visible_size = Director::getInstance()->getVisibleSize();
    float panel_height = 280;

    // 2. 辅助 lambda：创建大图标 
    auto create_btn = [&](std::string name, std::string img, BuildingType type) {
        // a. 创建图片并放缩
        auto sprite = Sprite::create(img);
        if (!sprite) sprite = Sprite::create(); 
        // 缩放图片到合适大小（80*80） 
        float s = 80.0f / MAX(sprite->getContentSize().width, sprite->getContentSize().height);
        sprite->setScale(s);
        sprite->setPosition(60, 65);

        // b. 创建建筑名称
        auto lbl = Label::createWithTTF(name, "fonts/GROBOLD.ttf", 16);
        lbl->enableOutline(Color4B::BLACK, 2);  // 描边
        lbl->setPosition(60, 125);

        // c. 显示造价
        BuildingStats stats = BaseBuilding::GetStatsConfig(type, 1);      // 获取该建筑 1 级的配置数据

        int cost_val = 0;  // 造价
        std::string icon_path = "";  // 图片路径
        Color3B text_color = Color3B::WHITE;

        // (1) 判断是金币还是圣水
        if (stats.cost_gold > 0) {
            cost_val = stats.cost_gold; // 消耗金币
            icon_path = "Icon/Gold.png";          
            text_color = Color3B(255, 220, 0); // 金色
        }
        else {
            cost_val = stats.cost_elixir; // 消耗圣水
            icon_path = "Icon/Elixir.png";        
            text_color = Color3B(220, 0, 220);// 紫色
        }
        auto price_node = Node::create();  // 创建价格容器 
        price_node->setPosition(60, 10); // 放在底部位置
       
        // (2) 创建资源小图标
        auto res_icon = Sprite::create(icon_path);
        float icon_width = 0;
        if (res_icon) {
            float icon_scale = 20.0f / res_icon->getContentSize().height; // 限制高度20
            res_icon->setScale(icon_scale);
            res_icon->setAnchorPoint(Vec2(0, 0.5)); // 左对齐
            price_node->addChild(res_icon);
            icon_width = res_icon->getContentSize().width * icon_scale;
        }

        // (3) 创建造价文字
        auto lbl_cost = Label::createWithTTF(std::to_string(cost_val), "fonts/GROBOLD.ttf", 18);
        lbl_cost->setColor(text_color);
        lbl_cost->enableOutline(Color4B::BLACK, 2); // 描边加粗
        lbl_cost->setAnchorPoint(Vec2(0, 0.5));     // 左对齐

        // (4) 排列图标 文字
        float text_width = lbl_cost->getContentSize().width;   // 排版居中计算
        float gap = 5.0f;  // 设置标签之间的距离
        float total_w = icon_width + gap + text_width;

        // 从左边的起始点开始画
        float start_x = -total_w / 2.0f;
        if (res_icon) {
            res_icon->setPosition(start_x, 0);
            start_x += icon_width + gap;
        }
        lbl_cost->setPosition(start_x, 0);

        // d. 组合成 MenuItemSprite
        auto node_normal = Node::create();
        node_normal->setContentSize(Size(120, 150));
        node_normal->addChild(sprite);
        node_normal->addChild(lbl);
        node_normal->addChild(price_node);
        price_node->addChild(lbl_cost);

        // e. 点击按钮回调
        return MenuItemSprite::create(node_normal, node_normal, [=](Ref*) {
            PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

            // 1. 如果之前手里已经有东西了，先清理掉
            if (current_preview_building_) {
                current_preview_building_->removeFromParent();
                current_preview_building_ = nullptr;
            }
            // 2. 创建新建筑
            // 注意：这里只是创建出来跟随鼠标，还没有真正“种”下去
            current_preview_building_ = BaseBuilding::Create(type, 1);

            // 3. 设置为预览状态 (变绿/半透明，无血条)
            current_preview_building_->ChangeState(BuildingState::kPreview);

            // 4. 加到地图上
            main_village_map_->addChild(current_preview_building_, 99999);

            selected_building_type_ = type; // 设置鼠标建筑对应类型
            this->UpdateOccupiedGridVisual();
            CCLOG("选中建筑: %s", name.c_str());
            build_menu_node_->setVisible(false); // 选完自动关闭

            });
        };

    // 3. 根据分类创建不同的按钮
    Vector<MenuItem*> items;

    if (category == 0) {
        // === 大本营 ==
        items.pushBack(create_btn("Tower Hall", "Buildings/TownHall/TownHall1.png", BuildingType::kTownHall));
        // === 资源类 ===
        items.pushBack(create_btn("Gold Mine", "Buildings/Gold_Mine/Gold_Mine1.png", BuildingType::kGoldMine));
        items.pushBack(create_btn("Elixir Pump", "Buildings/Elixir_Pump/Elixir_Pump1.png", BuildingType::kElixirPump));
        items.pushBack(create_btn("Gold Store", "Buildings/Gold_Storage/Gold_Storage1.png", BuildingType::kGoldStorage));
        items.pushBack(create_btn("Elixir Store", "Buildings/Elixir_Storage/Elixir_Storage1.png", BuildingType::kElixirStorage));
    }
    else if (category == 1) {
        // === 防御类 ===
        items.pushBack(create_btn("Cannon", "Buildings/Cannon/Cannon1.png", BuildingType::kCannon));
        items.pushBack(create_btn("Archer Tower", "Buildings/ArcherTower/ArcherTower1.png", BuildingType::kArcherTower));
        items.pushBack(create_btn("Wall", "Buildings/Wall/Wall1.png", BuildingType::kWall));
        items.pushBack(create_btn("Barracks", "Buildings/Barracks/Barracks1.png", BuildingType::kBarracks));
    }

    // 4. 创建菜单并添加到容器
    auto menu = Menu::createWithArray(items);
    menu->alignItemsHorizontallyWithPadding(20);
    menu->setPosition(visible_size.width / 2, (panel_height - 50) / 2);    // 放在面板中间偏下 

    icon_container_->addChild(menu);
}

// 初始化士兵结构体
std::vector<MainVillage::TroopInfo> MainVillage::troops_ = {
       {"Barbarian",   "Troops/Barbarian_head.png",    1,    25},
       {"Archer",      "Troops/Archer_head.png",       1,    30},
       {"Giant",       "Troops/Giant_head.png",        5,   250},
       {"WallBreaker", "Troops/Wall_Breaker_head.png", 2,   100}
};

// 左下角Attack按钮
void MainVillage::CreateAttackUi() {
    auto visible_size = Director::getInstance()->getVisibleSize();

    // 1. 创建按钮容器
    float box_size = 130.0f; // 设置容器大小 略大于图片
    auto wrapper = Node::create();
    wrapper->setContentSize(Size(box_size, box_size)); // box大小
    wrapper->setAnchorPoint(Vec2(0.5, 0.5)); // 中心锚点

    // 2. 创建并放缩图片
    std::string icon_path = "Icon/Attack_Button.png";
    auto sprite = Sprite::create(icon_path);

    // 进行缩放
    float target_icon_size = 100.0f;
    float content_max = std::max(sprite->getContentSize().width, sprite->getContentSize().height);    // 获取图片的最长边 (宽或高)
    if (content_max <= 0) content_max = 100.0f;    // 防止除以0
    float scale = 1.5 * target_icon_size / content_max;    // 计算缩放比例：目标大小 / 原始大小
    sprite->setScale(scale);    // 应用均匀缩放 
    sprite->setPosition(box_size / 2, box_size / 2);    // 把图片放在容器的正中心
    wrapper->addChild(sprite);    // 放入容器

    // 3. 创建按钮 
    auto btn_attack = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        // 点击收缩反馈
        sprite->stopAllActions();
        sprite->runAction(Sequence::create(
            ScaleTo::create(0.1f, scale * 0.9f), // 变小
            ScaleTo::create(0.1f, scale),        // 弹回
            nullptr
        ));

        // 点击逻辑
        this->SaveVillageData();   // 存档
        this->ShowLevelSelection();// 选择关卡
        });

    // 4. 设置位置和整体动画
    btn_attack->setPosition(Vec2(80, 80)); // 设置位置

    // 呼吸动画：对按钮进行缩放
    btn_attack->runAction(RepeatForever::create(Sequence::create(
        ScaleTo::create(0.8f, 1.1f), // 放大到 1.1 倍
        ScaleTo::create(0.8f, 1.0f), // 恢复 1.0 倍
        nullptr
    )));

    // 5. 添加到菜单
    auto menu = Menu::create(btn_attack, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1000);
}

// 选择关卡弹窗
void MainVillage::ShowLevelSelection() {
    // 防止重复打开
    if (settings_layer_) return;

    auto visible_size = Director::getInstance()->getVisibleSize();

    // ==================== 1. 创建全屏遮罩 =====================
    settings_layer_ = LayerColor::create(Color4B(0, 0, 0, 200), visible_size.width, visible_size.height);
    settings_layer_->setName("LevelSelectionLayer");

    // 拦截点击
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, settings_layer_);

    this->addChild(settings_layer_, 20000);

    // ===================== 2. 创建弹窗背景 ====================
    auto bg = ui::Scale9Sprite::create("Icon/Level_Panel.png"); // 九宫格绘制背景
    if (!bg) {
        auto s = Sprite::create();
        s->setTextureRect(Rect(0, 0, 100, 100));
        s->setColor(Color3B(80, 60, 40));
        bg = ui::Scale9Sprite::createWithSpriteFrame(s->getSpriteFrame());
    }
    bg->setContentSize(Size(500, 550));
    bg->setPosition(visible_size.width / 2, visible_size.height / 2);
    settings_layer_->addChild(bg);

    // 设置标题文字
    auto title = Label::createWithTTF("SELECT BATTLE", "fonts/GROBOLD.ttf", 36);
    title->enableOutline(Color4B::BLACK, 2);
    title->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 50);
    bg->addChild(title);

    // ==================== 3. 辅助函数：创建单个关卡条目 ==================
    auto create_level_btn = [&](std::string name, int stars, bool is_locked, const std::function<void()>& callback) -> MenuItem* {

        // --- A. 创建关卡按钮容器 ---
        auto wrapper = Node::create();
        wrapper->setContentSize(Size(400, 100));
        wrapper->setAnchorPoint(Vec2(0.5, 0.5));

        // --- B. 设置关卡背景 ---
        auto item_bg = ui::Scale9Sprite::create("Icon/Level_Item_Bg.png");
        if (!item_bg) {
            auto s = Sprite::create(); s->setTextureRect(Rect(0, 0, 100, 100)); s->setColor(Color3B(120, 90, 60));
            item_bg = ui::Scale9Sprite::createWithSpriteFrame(s->getSpriteFrame());
        }
        item_bg->setContentSize(Size(400, 100));
        item_bg->setPosition(200, 50);
        wrapper->addChild(item_bg);

        // --- C. 关卡名字 ---
        auto lbl_name = Label::createWithTTF(name, "fonts/GROBOLD.ttf", 24);
        lbl_name->setAnchorPoint(Vec2(0, 0.5));
        lbl_name->setPosition(20, 70);
        lbl_name->enableOutline(Color4B::BLACK, 1);
        wrapper->addChild(lbl_name);

        // --- D. 关卡状态显示 (锁 或者 星星) ---
        if (is_locked) {
            // === 锁定状态：变灰 + 显示锁 ===
            item_bg->setColor(Color3B::GRAY); // 背景变灰

            auto lock_icon = Sprite::create("Icon/icon_lock.png");
            lock_icon->setPosition(350, 50); // 右侧显示"X"
            wrapper->addChild(lock_icon);
        }
        else {
            // === 解锁状态：显示星星 + GO ===

            // (1) 绘制 3 颗星星
            for (int i = 0; i < 3; i++) {
                // 如果 stars=3，则 i<3 恒成立，全是金星
                // 如果 stars=0，则 i<0 恒不成立，全是灰星
                std::string star_img = (i < stars) ? "Icon/star_gold.png" : "Icon/star_gray.png";

                auto star = Sprite::create(star_img);
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

            // (2) 显示 GO 图标 
            auto go_label = Label::createWithTTF("GO", "fonts/GROBOLD.ttf", 30);
            go_label->setColor(Color3B::GREEN);
            go_label->enableOutline(Color4B::BLACK, 2);
            go_label->setPosition(350, 50);
            wrapper->addChild(go_label);
        }

        // --- E. 创建按钮 ---
        auto btn = MenuItemSprite::create(wrapper, nullptr, [=](Ref* sender) {
            
            PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

            // 点击反馈动画
            auto node = dynamic_cast<Node*>(sender);
            node->stopAllActions();
            auto seq = Sequence::create(
                ScaleTo::create(0.1f, 0.95f),// 放大
                ScaleTo::create(0.1f, 1.0f), // 返回
                CallFunc::create(callback), // 执行传入的回调
                nullptr
            );
            node->runAction(seq);
            });

        return btn;
        };

    // ============== 4. 循环生成关卡按钮 ==============

    // A. 关卡配置列表
    struct LevelConfig {
        std::string name; // 关卡名称
        std::string map_file; // 地图名称
    };
    std::vector<LevelConfig> levels = {
        {"1. Goblin Forest",   "map/map1.tmx"},
        {"2. Desert Fortress", "map/map2.tmx"},
        {"3. Dark Castle",     "map/map3.tmx"}
    };

    // B. 创建关卡菜单容器
    Vector<MenuItem*> menu_items; // 存储所有关卡

    for (int i = 0; i < levels.size(); ++i) {
        int level_id = i + 1;

        // a. 从 PlayerData 获取数据
        int stars = PlayerData::GetInstance()->GetLevelStar(level_id);// 如果赢过，stars=3；没赢过，stars=0

        // 判断是否锁定
        bool is_locked = PlayerData::GetInstance()->IsLevelLocked(level_id);  

        // b. 创建按钮
        auto btn = create_level_btn(levels[i].name, stars, is_locked, [=]() {
            if (is_locked) { // 如果还没解锁
                CCLOG("关卡 %d 未解锁！", level_id);
            }
            else {
                CCLOG("进入关卡 %d...", level_id);

                auto scene = GameMap::Create(levels[i].mapFile); // 创建对应地图 
                Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene)); // 切换场景
            }
            });
        // c. 加入容器
        menu_items.pushBack(btn);
    }

    // ================ 5. 组装菜单 ==================
    auto level_menu = Menu::createWithArray(menu_items);
    level_menu->alignItemsVerticallyWithPadding(20); // 竖向排列
    level_menu->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height / 2 - 20);  // 居中放置
    bg->addChild(level_menu);

    // ================ 6. 关闭按钮 ==================
    // a. X按钮标签
    auto lbl_close = Label::createWithSystemFont("X", "Arial", 30);
    lbl_close->setColor(Color3B::RED);
    lbl_close->enableOutline(Color4B::WHITE, 2);

    // b. 按钮回调逻辑
    auto close_btn = MenuItemLabel::create(lbl_close, [=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
        // 关闭设置层
        if (settings_layer_) {
            settings_layer_->removeFromParent();
            settings_layer_ = nullptr;
        }
        });

    close_btn->setPosition(Vec2(220, 250));    // 右上角位置
    // 组装到menu里面 加入bg
    auto close_menu = Menu::create(close_btn, nullptr);
    close_menu->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height / 2);
    bg->addChild(close_menu);

    // =============== 7. 入场动画 ================
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}

// 存档操作
void MainVillage::SaveVillageData() {
    auto data_center = PlayerData::GetInstance(); // 获取现在的所有信息

    // 1. 清空旧存档
    data_center->village_layout_.clear();

    // 2. 遍历地图子节点 获取地图大小
    auto& children = main_village_map_->getChildren();

    Size tile_size = main_village_map_->getTileSize();
    Size map_size = main_village_map_->getMapSize();

    for (const auto& child : children) {
        // a. 转换类型
        auto building = dynamic_cast<BaseBuilding*>(child);

        // 排除未建造和已经摧毁的建筑
        if (building && building->state_ != BuildingState::kPreview && building->state_ != BuildingState::kDestroyed) {
            // b. 存储基本数据
            BuildingData data;
            data.type = building->type_;
            data.level = building->level_;

            // c. 存储位置坐标 根据位置反算网格坐标 
            // X 轴:
            int t_x = static_cast<int>((building->getPositionX() / tile_size.width));
            // Y 轴:
            int t_y = static_cast<int>(map_size.height) - 1 - static_cast<int>((building->getPositionY() / tile_size.height));

            data.tile_x = t_x;
            data.tile_y = t_y;

            // d. 存入建筑存档
            data_center->village_layout_.push_back(data);
        }
    }

    CCLOG("存档完成：保存了 %d 个单格建筑", (int)data_center->village_layout_.size());
}

// 读档操作
void MainVillage::RestoreVillageData() {
    auto data_center = PlayerData::GetInstance();
    Size tile_size = main_village_map_->getTileSize();
    Size map_size = main_village_map_->getMapSize();

    BaseBuilding* the_barracks = nullptr;

    // A. 遍历建筑存档数据
    for (const auto& data : data_center->village_layout_) {

        // 1. 创建建筑
        BaseBuilding* new_building = nullptr;

        // 根据类型创建具体的子类 
        if (data.type == BuildingType::kGoldMine || data.type == BuildingType::kElixirPump) {
            new_building = ResourceProducer::Create(data.type, data.level);
        }
        else if (data.type == BuildingType::kGoldStorage ||
            data.type == BuildingType::kElixirStorage ||
            data.type == BuildingType::kBarracks) {
            auto s = ResourceStorage::Create(data.type, data.level);
            storage_list_.push_back(s); // 加回存储列表
            new_building = s;
        }
        else if (data.type == BuildingType::kTownHall) {
            new_building = TownHall::create(data.level);
        }
        else if (data.type == BuildingType::kArcherTower) {
            new_building = ArcherTower::create(data.level);
        }
        else if (data.type == BuildingType::kCannon) {
            new_building = Cannon::create(data.level);
        }
        else if (data.type == BuildingType::kWall) {
            new_building = Wall::create(data.level);
        }
        else {
            // 兜底
            new_building = BaseBuilding::Create(data.type, data.level);
        }

        if (new_building) {
            // 2. 恢复状态
            new_building->ChangeState(BuildingState::kIdle);
            new_building->setTag(999); // 设置 Tag

            // 3. 压缩缩放 
            Size spriteSize = new_building->getContentSize();
            if (spriteSize.width > 0 && spriteSize.height > 0) {
                new_building->setScaleX(tile_size.width / spriteSize.width);
                new_building->setScaleY(tile_size.height / spriteSize.height);
            }

            // 4. 反向计算坐标 
            float final_x = data.tile_x * tile_size.width + tile_size.width / 2;
            float final_y = (map_size.height - 1 - data.tile_y) * tile_size.height + tile_size.height / 2;
            new_building->setPosition(Vec2(final_x, final_y));

            // 5. 加回地图
            main_village_map_->addChild(new_building, 3000 - (int)final_y);

            // 6. 恢复占用标记
            std::string key = StringUtils::format("%d_%d", data.tile_x, data.tile_y);
            occupied_tiles_[key] = true;

            // 7. 处理兵营
            if (new_building->type_ == BuildingType::kBarracks) {
                the_barracks = new_building;
            }
        }
    }
    // B. 刷新全局容量上限
    this->RefreshTotalCapacity();
    this->UpdateResourceUi();

    // C. 兵营可视化士兵处理
    // a. 没有兵营直接返回
    if (the_barracks == nullptr) return;

    // b. 定义生成函数 (只针对 theBarracks)
    auto spawn_troops_at_barracks = [&](std::string name) {
        Soldier* s = nullptr;
        if (name == "Barbarian") s = Barbarian::create();
        else if (name == "Archer") s = Archer::create();
        else if (name == "Giant") s = Giant::create();
        else if (name == "WallBreaker") s = WallBreaker::create();

        if (s) {
            // 1. 设置模式
            s->setHomeMode(true);
            s->setHomePosition(the_barracks->getPosition());

            // 2. 随机位置 (兵营下方扇形)
            float angle = CC_DEGREES_TO_RADIANS(210 + CCRANDOM_0_1() * 120);
            float radius = 60.0f + CCRANDOM_0_1() * 60.0f;

            Vec2 offset(cos(angle) * radius, sin(angle) * radius);
            Vec2 pos = the_barracks->getPosition() + offset;

            s->setPosition(pos);

            // 3. 统一缩放逻辑
            Size ss = s->getContentSize();
            float scale = 0.2f; 

            s->setScale(scale);
            if (pos.x < the_barracks->getPositionX()) s->setScaleX(-scale);

            main_village_map_->addChild(s, 3000 - static_cast<int>(pos.y));

            // 4. 把恢复出来的兵，重新登记到兵营名册
            the_barracks->visual_troops_[name].push_back(s);
        }
    };

    // c. 遍历库存，生成所有兵
    std::vector<std::string> troopTypes = { "Barbarian", "Archer", "Giant", "WallBreaker" };
    for (const auto& name : troopTypes) {
        int count = data_center->GetTroopCount(name);
        for (int i = 0; i < count; ++i) {
            spawn_troops_at_barracks(name);
        }
    }
    CCLOG("单兵营模式：士兵视觉已恢复");

}

// 绘制非法放置边框
void MainVillage::UpdateOccupiedGridVisual() {

    // 如果手里没有预览建筑，就清空并返回
    if (selected_building_type_ == BuildingType::kNone) {
        if (grid_draw_node_) grid_draw_node_->clear();
        return;
    }
    // 1. 初始化绘制区域 
    if (!grid_draw_node_) {
        grid_draw_node_ = DrawNode::create();
        main_village_map_->addChild(grid_draw_node_, 20000);  //Z轴保证在所有建筑之上
    }

    // 2. 每次刷新前先清空旧的框框
    grid_draw_node_->clear();

    Size tile_size = main_village_map_->getTileSize();
    Size map_size = main_village_map_->getMapSize();

    // 3. 遍历占用表
    for (auto const& item : occupied_tiles_) {
        // item.first 是 key "x_y"
        // item.second 是 bool (是否占用)

        if (item.second == true) {
            // A. 解析 Key 字符串 
            std::string key = item.first; // "10_15"->x = 10, y = 15
            size_t underscore_pos = key.find('_');
            if (underscore_pos == std::string::npos) continue;

            int tile_x = std::stoi(key.substr(0, underscore_pos));
            int tile_y = std::stoi(key.substr(underscore_pos + 1));

            // B. 坐标转换 
            float origin_x = tile_x * tile_size.width;
            float origin_y = (map_size.height - 1 - tile_y) * tile_size.height;

            Vec2 origin(origin_x, origin_y);
            Vec2 dest(origin_x + tile_size.width, origin_y + tile_size.height);

            // C. 绘制 
            // 实心半透明红块 
            grid_draw_node_->drawSolidRect(origin, dest, Color4F(1.0f, 0.0f, 0.0f, 0.3f)); // 红色，30%透明度
            // 空心框
            grid_draw_node_->drawRect(origin, dest, Color4F(1.0f, 0.0f, 0.0f, 0.8f));
        }
    }
}

void MainVillage::ShowSettingsLayer() {
    if (settings_layer_) return;    // 防止重复打开

    auto visible_size = Director::getInstance()->getVisibleSize();
    // ===================== 1. 创建遮罩层 ===================== 
    settings_layer_ = LayerColor::create(Color4B(0, 0, 0, 180), visible_size.width, visible_size.height);
    settings_layer_->setName("SettingsLayer"); // Tag命名 用于点击检测

    // 触摸拦截
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, settings_layer_);
    this->addChild(settings_layer_, 20000); // 最顶层

    // ======================= 2. 设置背景板 ======================
    std::string bg_path = "Icon/setting_panel.png"; // 设置背景图片
    auto bg = ui::Scale9Sprite::create(bg_path); // 九宫格绘制

    // 设置面板大小 
    bg->setContentSize(Size(450, 350)); // (宽450, 高350)
    bg->setPosition(visible_size.width / 2, visible_size.height / 2); // 位于中心位置
    bg->setName("SettingsBackground"); // 用于点击检测

    settings_layer_->addChild(bg);

    // ========================= 3. 标题 ==========================
    auto lbl_title = Label::createWithTTF("SETTINGS", "fonts/GROBOLD.ttf", 36);
    lbl_title->enableOutline(Color4B::BLACK, 3); // 描边加粗
    lbl_title->setPosition(bg->getContentSize().width / 2, bg->getContentSize().height - 40);
    bg->addChild(lbl_title);

    // ================== 4. 音量（Music和Effect）按钮 ===============
    /**
     * @brief 动态创建一行音量控制组件
     * @param title    标题文字 (如 "Music", "Effect")
     * @param icon_path 左侧图标路径 (如 "icon_music.png")
     * @param pos_y     在背景板上的 Y 轴坐标
     * @param get_val   获取当前音量的回调 (返回 0.0 ~ 1.0)
     * @param setVal   设置音量的回调 (传入 0.0 ~ 1.0)
     * @return Vector<MenuItem*> 返回生成的加减按钮，以便添加到 Menu 中
     */
    auto createVolumeControl = [&](std::string title, std::string icon_path, float posY, std::function<float()> get_val, std::function<void(float)> setVal) {
        // a. 根据iconPath来创建小喇叭图标 
        auto icon = Sprite::create(icon_path);

        if (!icon) {
            // 防止找不到图
            auto draw = DrawNode::create();
            draw->drawSolidCircle(Vec2::ZERO, 10, 0, 10, Color4F::YELLOW);
            icon = Sprite::create();
            icon->addChild(draw);
        }
        else {
            // 缩放图标到合适大小 (比如 30x30)
            float target_size = 30.0f;
            float max_side = std::max(icon->getContentSize().width, icon->getContentSize().height);
            if (max_side > 0) icon->setScale(target_size / max_side);
        }
        // 位置：放在最左边位置
        icon->setPosition(50, posY);
        bg->addChild(icon);

        // b. 音量标题（music和effect）
        auto lbl = Label::createWithTTF(title, "fonts/GROBOLD.ttf", 26);
        lbl->setAnchorPoint(Vec2(0, 0.5));
        lbl->enableOutline(Color4B::BLACK, 2); // 描边加粗
        lbl->setPosition(80, posY);
        bg->addChild(lbl);

        // c. 格子容器
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

        // 初始刷新音量
        refresh_bar(get_val());

        // d. 减号按钮 [-]
        auto lbl_minus = Label::createWithTTF("-", "fonts/GROBOLD.ttf", 45); 
        lbl_minus->enableOutline(Color4B::BLACK, 3);
        auto btn_minus = MenuItemLabel::create(lbl_minus, [=](Ref*) {
            float v = get_val();
            int level = static_cast<int>(v * 10 + 0.5f); // 同理现在音量格数
            if (level > 0) {
                level--;  // 音量-1
                setVal(level / 10.0f); // 调用设置音量函数
                refresh_bar(level / 10.0f); // 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
            }
            });
        btn_minus->setPosition(170, posY);

        // e. 加号按钮 [+]
        auto lbl_plus = Label::createWithTTF("+", "fonts/GROBOLD.ttf", 45);
        lbl_plus->enableOutline(Color4B::BLACK, 3);
        auto btn_plus = MenuItemLabel::create(lbl_plus, [=](Ref*) {
            float v = get_val();
            int level = static_cast<int>(v * 10 + 0.5f);// 同理现在音量格数
            if (level < 10) {
                level++;// 音量+1
                setVal(level / 10.0f);// 调用设置音量函数
                refresh_bar(level / 10.0f);// 刷新音量显示
                // 如果是调节音效，播放一下声音
                if (title == "Effect") PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");
            }
            });
        btn_plus->setPosition(190 + 180 + 20, posY);

        // 返回菜单项，以便添加到主菜单
        return Vector<MenuItem*>{btn_minus, btn_plus};
        };

    // f. 使用上面的通用函数，创建两排控制器
    // (1). 音乐控制 (Music) - 放在 Y=220
    auto music_items = createVolumeControl("Music", "Icon/icon_music.png", 220,
        []() { return PlayerData::GetInstance()->music_volume_; }, // 获取
        [](float v) { PlayerData::GetInstance()->SetMusicVol(v); } // 设置
    );

    // (2). 音效控制 (Effect) - 放在 Y=160
    auto effect_items = createVolumeControl("Effect", "Icon/icon_effect.png", 160,
        []() { return PlayerData::GetInstance()->effect_volume_; }, // 获取
        [](float v) { PlayerData::GetInstance()->SetEffectVol(v); } // 设置
    );

    // 把所有按钮加到一个 Menu 里
    auto vol_menu = Menu::create();
    for (auto item : music_items) vol_menu->addChild(item); // 音乐按钮
    for (auto item : effect_items) vol_menu->addChild(item);// 音效按钮

    vol_menu->setPosition(Vec2::ZERO);
    bg->addChild(vol_menu);

    // ===================== 4. 继续游戏 (Resume) =====================
    auto btn_resume_label = Label::createWithTTF("Resume", "fonts/GROBOLD.ttf", 30);
    btn_resume_label->enableOutline(Color4B::BLACK, 2);
    // 按钮回调处理 关闭设置菜单
    auto btn_resume = MenuItemLabel::create(btn_resume_label, [=](Ref*) {

        PlayerData::GetInstance()->PlayEffect("Audio/click.mp3");

        if (settings_layer_) {  // 如果打开设置菜单
            settings_layer_->removeFromParent(); // 关闭设置菜单
            settings_layer_ = nullptr; // 指针置空
        }
        });

    // 放在底部偏上
    btn_resume->setPosition(Vec2(bg->getContentSize().width / 2, 110));

    // ================== 5. 组装btnResume菜单 =======================
    auto bottom_menu = Menu::create(btn_resume, nullptr);
    bottom_menu->setPosition(Vec2::ZERO);
    bg->addChild(bottom_menu);

    // =================== 6. 弹出入场动画 ======================
    bg->setScale(0);
    bg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
}