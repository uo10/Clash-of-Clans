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

    // ====================精灵选择按钮========================
    // 按钮 1
    auto label1 = Label::createWithSystemFont("Select Item A", "Arial", 24);
    auto item1 = MenuItemLabel::create(label1, [=](Ref* sender) {
        _selectedSpritePath = "R-C.jpg";
        CCLOG("Switched to Item A");
        });

    // 按钮 2
    auto label2 = Label::createWithSystemFont("Select Item B", "Arial", 24);
    auto item2 = MenuItemLabel::create(label2, [=](Ref* sender) {
        _selectedSpritePath = "HelloWorld.png"; 
        CCLOG("Switched to Item B");
        });

  
    auto menu = Menu::create(item1, item2, nullptr);// 创建菜单容器

    menu->alignItemsVerticallyWithPadding(20); // 排列按钮 (纵向排列)

    menu->setPosition(Vec2(100, visibleSize.height - 100));    // 设置菜单位置 (屏幕左上角)

    this->addChild(menu, 100);    // 将选择UI添加到 this
	//=============================================================
    auto mouseListener = EventListenerMouse::create();

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

    // ==================  二、鼠标按下  ==================
    mouseListener->onMouseDown = [=](Event* event) {
        EventMouse* e = (EventMouse*)event;
		//1、判断是否点在菜单上
		Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());// 获取鼠标点击位置
		Vec2 localPos = menu->getParent()->convertToNodeSpace(mousePos);// 转换到菜单节点空间
		if (menu->getBoundingBox().containsPoint(localPos)) {// 判断是否点在菜单区域内
            _isDragging = false;
            return; // 点在菜单上了，忽略拖拽
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
        Vec2 localPos = menu->getParent()->convertToNodeSpace(mousePos);
        if (menu->getBoundingBox().containsPoint(localPos)) {
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
                        auto newSprite = Sprite::create(_selectedSpritePath);
                        if (newSprite) {
                            // 适应瓦片大小
                            Size spriteSize = newSprite->getContentSize();
                            newSprite->setScaleX(tileSize.width / spriteSize.width);
                            newSprite->setScaleY(tileSize.height / spriteSize.height);

                            // 居中位置
                            float finalX = tileX * tileSize.width + tileSize.width / 2;
                            float finalY = (mapSize.height - 1 - tileY) * tileSize.height + tileSize.height / 2;
                            newSprite->setPosition(Vec2(finalX, finalY));
							newSprite->setTag(999);// 设置Tag以便后续识别
                            map->addChild(newSprite, 10);
							// 标记该瓦片为已占用
                            std::string key = StringUtils::format("%d_%d", tileX, tileY);
                            _occupiedTiles[key] = true;
                            CCLOG("Sprite placed at tile (%d, %d)", tileX, tileY);
                        }
                    }
                }
            }

            // 结束拖拽状态
            _isDragging = false;
        }
		// 3、如果是右键松开
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {

            // 1、获取点击在地图内部的坐标
            Vec2 nodePos = map->convertToNodeSpace(mousePos);
            int spriteTileX = (int)(nodePos.x / tileSize.width);
            int spriteTileY = (int)(mapSize.height - (nodePos.y / tileSize.height));
            // 2、遍历地图的所有子节点
            auto& children = map->getChildren();
          
            for (const auto& child : children) {

                // 3、筛选：只检测Tag为999的节点 (防止删掉地图图层)
                if (child->getTag() == 999) {

                    // 4、碰撞检测：判断点击点是否在精灵的矩形范围内                   
                    if (child->getBoundingBox().containsPoint(nodePos)) {
                        // 5、移除精灵
                        child->removeFromParent();
						// 更新占用状态
                        std::string key = StringUtils::format("%d_%d", spriteTileX, spriteTileY);
                        _occupiedTiles[key] = false;
                        CCLOG("Sprite deleted!");                     
                        break;
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

    return true;
}
void MainVillage::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}