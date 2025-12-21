#include "BaseBuilding.h"
#include"PlayerData.h"

USING_NS_CC;

BuildingStats BaseBuilding :: getStatsConfig(BuildingType type, int level) {
    BuildingStats data;

    // 初始化默认值 
    data.name = "Unknown";
    data.hp = 100;
    data.costGold = 0;
    data.costElixir = 0;
    data.buildTime = 0;
    data.capacity = 0;
    data.productionRate = 0;
    data.damage = 0.0f;

    // 1. 金矿配置
    if (type == BuildingType::GOLD_MINE) {
        if (level == 1) {
            data.name = "Gold Mine Lv1";
            data.hp = 400;
            data.costElixir = 150;
            data.buildTime = 10;
            data.capacity = 500;
            data.productionRate = 100000000;
        }
        else if (level == 2) {
            data.name = "Gold Mine Lv2";
            data.hp = 480;
            data.costElixir = 300;
            data.buildTime = 60;
            data.capacity = 1000;
            data.productionRate = 400;
        }
        else if (level == 3) {
            data.name = "Gold Mine Lv3";
            data.hp = 560;
            data.costElixir = 450;
            data.buildTime = 120;
            data.capacity = 2000;
            data.productionRate = 800;
        }
    }
    // 2. 圣水收集器配置
    else if (type == BuildingType::ELIXIR_PUMP) {
        if (level == 1) {
            data.name = "Elixir Pump Lv1";
            data.hp = 350;
            data.costGold = 150;
            data.buildTime = 10;
            data.capacity = 500;
            data.productionRate = 1000000;
        }
        else if (level == 2) {
            data.name = "Elixir Pump Lv2";
            data.hp = 500;
            data.costGold = 300;
            data.buildTime = 30;
            data.capacity = 1000;
            data.productionRate = 400;
        }
        else if (level == 3) {
            data.name = "Elixir Pump Lv3";
            data.hp = 800;
            data.costGold = 600;
            data.buildTime = 60;
            data.capacity = 2000;
            data.productionRate = 800;
        }
    }
    // 3. 金库配置
    else if (type == BuildingType::GOLD_STORAGE) {
        data.name = "Gold Storage";
        data.hp = 800 + (level * 200);
        data.costElixir = 500 * level; // 金库消耗圣水
        data.buildTime = 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 4. 圣水瓶配置
    else if (type == BuildingType::ELIXIR_STORAGE) {
        data.name = "Elixir Storage";
        data.hp = 800 + (level * 200);
        data.costGold = 500 * level;
        data.buildTime = 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 5. 兵营配置
    else if (type == BuildingType::BARRACKS) {
        data.name = "Barracks";
        data.hp = 800 + (level * 200);
        data.costElixir = 500 * level;
        data.buildTime = 30 + (level * 30);
        data.capacity = 30 * level;
    }
    // 6. 加农炮配置
    else if (type == BuildingType::CANNON) {
        if (level == 1) {
            data.name = "Cannon Lv1";
            data.hp = 300;
            data.costGold = 250;
            data.costElixir = 0;
            data.buildTime = 5;
            data.damage = 5.6;
        }
        else if (level == 2) {
            data.name = "Cannon Lv2";
            data.hp = 360;
            data.costGold = 1000;
            data.buildTime = 30;
            data.damage = 8;
        }
        else if (level == 3) {
            data.name = "Cannon Lv3";
            data.hp = 420;
            data.costGold = 4000;
            data.buildTime = 120;
            data.damage = 10.4;
        }
    }
    // 7. 箭塔配置
    else if (type == BuildingType::ARCHER_TOWER) {
        if (level == 1) {
            data.name = "Archer Tower Lv1";
            data.hp = 380;
            data.costGold = 1000;
            data.costElixir = 0;
            data.buildTime = 15;
            data.damage = 5.5;
        }
        else if (level == 2) {
            data.name = "Archer Tower Lv2";
            data.hp = 420;
            data.costGold = 2000;
            data.buildTime = 120;
            data.damage = 7.5;
        }
        else if (level == 3) {
            data.name = "Archer Tower Lv3";
            data.hp = 460;
            data.costGold = 5000;
            data.buildTime = 1200;
            data.damage = 9.5;
        }
    }
    // 8. 墙配置
    else if (type == BuildingType::WALL) {
        if (level == 1) {
            data.name = "Wall Lv1";
            data.hp = 100;
            data.costGold = 0;
            data.costElixir = 0;
            data.buildTime = 15;
        }
        else if (level == 2) {
            data.name = "Wall Lv2";
            data.hp = 200;
            data.costGold = 1000;
            data.buildTime = 120;
        }
        else if (level == 3) {
            data.name = "Wall Lv3";
            data.hp = 400;
            data.costGold = 5000;
            data.buildTime = 1200;
        }
    }
    // 9. 大本营配置
    else if (type == BuildingType::TOWN_HALL) {
        if (level == 1) {
            data.name = "Town Hall Lv1";
            data.hp = 400;
            data.costGold = 0;
            data.costElixir = 0;
            data.buildTime = 0;
        }
        else if (level == 2) {
            data.name = "Town Hall Lv2";
            data.hp = 800;
            data.costGold = 1000;
            data.buildTime = 120;
        }
        else if (level == 3) {
            data.name = "Town Hall Lv3";
            data.hp = 1600;
            data.costGold = 4000;
            data.buildTime = 1200;
        }
        }
    return data;
}

bool BaseBuilding::init(BuildingType type, int level) {
    // 开启帧循环
    this->scheduleUpdate();
    // 基础类初始化
    if (!Node::init()) return false;

    // 指针置空 
    this->mainSprite = nullptr;

    // 赋值配置
    this->_stats = getStatsConfig(type, level);

    // 赋值属性
    this->type = type;
    this->level = level;
    this->name = _stats.name;
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp;

    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 初始状态
    this->state = BuildingState::IDLE;

    // 刷新状态 显示图片
    this->updateView();

    return true;
}

BaseBuilding* BaseBuilding::create(BuildingType type, int level) {
    BaseBuilding* pRet = new(std::nothrow) BaseBuilding();
    if (pRet && pRet->init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 获取建筑图片名字
std::string BaseBuilding::getTextureName(BuildingType type, int level) {
    std::string prefix = "";
    switch (type) {
    case BuildingType::GOLD_MINE:     prefix = "Gold_Mine"; break;
    case BuildingType::ELIXIR_PUMP:   prefix = "Elixir_Pump"; break;
    case BuildingType::GOLD_STORAGE:  prefix = "Gold_Storage"; break;
    case BuildingType::ELIXIR_STORAGE:prefix = "Elixir_Storage"; break;
    case BuildingType::BARRACKS:      prefix = "Barracks"; break;
    case BuildingType::WALL:      prefix = "Wall"; break;
    case BuildingType::CANNON:      prefix = "Cannon"; break;
    case BuildingType::ARCHER_TOWER:      prefix = "ArcherTower"; break;
    case BuildingType::TOWN_HALL:      prefix = "TownHall"; break;
    default: prefix = "building"; break;
    }
    return StringUtils::format("%s%d.png", prefix.c_str(), level);
}

    void BaseBuilding::updateView() {
        // 获取图片名称
        std::string baseTextureName = getTextureName(this->type, this->level);

        // 创建精灵 或者更新照片
        if (!mainSprite) {
            // 如果是首次创建
            mainSprite = Sprite::create(baseTextureName);
            this->addChild(mainSprite);
        }
        else {
            // 如果是已经存在，更新图片
            auto newTexture = Director::getInstance()->getTextureCache()->addImage(baseTextureName);
            if (newTexture) {
                mainSprite->setTexture(newTexture);
            }
        }
        if (mainSprite) {
            // 设置更新后的精灵的位置
            Size size = mainSprite->getContentSize();
            mainSprite->setPosition(size.width / 2, size.height / 2);
            // 设置 BaseBuilding 自身的大小等于图片大小
            this->setContentSize(size);
            // 设置锚点为中心 (0.5, 0.5)
            this->setAnchorPoint(Vec2(0.5f, 0.5f));
        }
        // 更换建筑的颜色和状态
        switch (this->state) {
        case BuildingState::PREVIEW:
            mainSprite->setColor(Color3B(100, 255, 100)); // 预览情况下 绿色
            mainSprite->setOpacity(128);
            break;

        case BuildingState::BUILDING:
            mainSprite->setColor(Color3B(150, 150, 150)); // 建造情况下 灰色
            mainSprite->setOpacity(255);
            break;

        case BuildingState::IDLE:
        case BuildingState::ATTACKING:
            mainSprite->setColor(Color3B::WHITE); // 正常情况下 白色
            mainSprite->setOpacity(255);
            break;

        case BuildingState::DESTROYED:
            this->setVisible(false);          // 被摧毁 直接隐藏建筑
            break;
        }
    }

void BaseBuilding::changeState(BuildingState newState) {
    if (this->state == newState) return;

    // 改变状态变量
    this->state = newState;
    // 刷新建筑画面
    this->updateView();
}

void BaseBuilding::upgradeLevel() {
    //大于三级直接退出
    if (this->level >= 3) {
        return;
    }
    // 升级
    this->level++;

    // 获取升级之后的数据
    BuildingStats newStats = getStatsConfig(this->type, this->level);
    this->_stats = newStats;

    // 更新基础属性
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp; // 升级回满血
    this->name = _stats.name;
    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 调用虚函数，通知子类更新它们的特有属性
    this->updateSpecialProperties();

    // 通知系统 (刷新总的资源容量)
    if (type == BuildingType::GOLD_STORAGE || type == BuildingType::ELIXIR_STORAGE || type == BuildingType::BARRACKS) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    // 升级结束回到正常状态
    this->state = BuildingState::IDLE;

    // 隐藏建造进度条
    if (_progressNode) {
        _progressNode->setVisible(false);
    }

    // 闪一下表示升级完成
    auto action = Sequence::create(
        ScaleTo::create(0.1f, 1.2f),
        ScaleTo::create(0.1f, 1.0f),
        nullptr
    );
    mainSprite->runAction(action);

    PlayerData::getInstance()->playEffect("Audio/plop.mp3");

    // 刷新升级后的画面
    this->updateView();

    CCLOG("升级完成 Lv.%d, 新名称: %s", level, name.c_str());
}

void BaseBuilding::initBuildUI() {
    if (_progressNode) return;

    _progressNode = Node::create();
    _progressNode->setVisible(false);
    this->addChild(_progressNode, 100);

    // 进行视觉缩放比例
    float visualScale = 0.4f;

    float sx = this->getScaleX();
    float sy = this->getScaleY();

    // 防止找不到图片崩溃
    if (std::abs(sx) < 0.01f) sx = 1.0f;
    if (std::abs(sy) < 0.01f) sy = 1.0f;

    // 设置大小 
    _progressNode->setScaleX((1.0f / sx) * visualScale);
    _progressNode->setScaleY((1.0f / sy) * visualScale);
    // 设置建筑位置
    float centerX = 0;
    float centerY = 0;

    if (this->mainSprite) {
        // 计算 X 轴中心 
        centerX = this->mainSprite->getPositionX();

        // 计算 Y 轴中心
        centerY = this->mainSprite->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type == BuildingType::GOLD_MINE || this->type == BuildingType::ELIXIR_STORAGE || this->type == BuildingType::GOLD_STORAGE) {
        centerY += 50;
    }
    else if (this->type == BuildingType::BARRACKS || this->type == BuildingType::WALL || this->type == BuildingType::ELIXIR_PUMP) {
        centerY += 100;
    }
    else {
        centerY += 150;
    }
    _progressNode->setPosition(Vec2(centerX, centerY));

    // 创建锤子图标 
    _hammerIcon = Sprite::create("icon_hammer.png");

    // 放在进度条上方
    _hammerIcon->setPosition(0, 40);
    _progressNode->addChild(_hammerIcon);

    // 进行浮动效果
    auto moveUp = MoveBy::create(0.6f, Vec2(0, 10)); // 向上飘10像素
    auto moveDown = moveUp->reverse();               // 向下飘回
    auto seq = Sequence::create(moveUp, moveDown, nullptr);
    _hammerIcon->runAction(RepeatForever::create(seq));

    // 时钟图标 (放在进度条左侧)
    auto clockIcon = Sprite::create("icon_clock.png");

    // 设置位置
    clockIcon->setPosition(Vec2(-60, 0));

    // 调整时钟的大小
    if (clockIcon->getContentSize().width > 25) {
        float scale = 25.0f / clockIcon->getContentSize().width;
        clockIcon->setScale(scale);
    }

    _progressNode->addChild(clockIcon);

    // 进度条背景
    auto bg = Sprite::create();
    bg->setTextureRect(Rect(0, 0, 84, 14));
    bg->setColor(Color3B::BLACK);   // 黑色背景板
    _progressNode->addChild(bg);

    // 进度条
    _progBar = Sprite::create();
    _progBar->setTextureRect(Rect(0, 0, 80, 10));
    _progBar->setColor(Color3B(255, 200, 0)); // 黄色
    _progBar->setAnchorPoint(Vec2(0, 0.5f));
    _progBar->setPosition(-40, 0);
    _progressNode->addChild(_progBar);

    // 显示剩余时间
    _timeLabel = Label::createWithSystemFont("0s", "Arial", 16);
    _timeLabel->enableOutline(Color4B::BLACK, 1);
    _timeLabel->setPosition(0, 0); // 放在中心位置
    _progressNode->addChild(_timeLabel);

    // * 加速按钮 在进度条最右侧

    auto speedUpWrapper = Node::create();   // 准备按钮容器 
    speedUpWrapper->setContentSize(Size(30, 30)); // 设定为 30x30 的正方形
    speedUpWrapper->setAnchorPoint(Vec2(0.5, 0.5));

    auto btnSprite = Sprite::create("btn_finish.png");    // 按钮照片

    float btnScale = 24.0f / std::max(btnSprite->getContentSize().width, btnSprite->getContentSize().height); //(24.0f)
    btnSprite->setScale(btnScale);    // 缩放适配容器
    btnSprite->setPosition(15, 15); // 居中
    speedUpWrapper->addChild(btnSprite);

    // 创建按钮逻辑
    auto btnSpeedUp = MenuItemSprite::create(speedUpWrapper, nullptr, [=](Ref* sender) {

        // 计算当前加速所需金币
        int cost = (int)(_buildLeftTime * 10);  // 每剩余 1 秒需要 10 金币

        // 扣费与结算
        if (PlayerData::getInstance()->consumeGold(cost)) {
            // PlayerData::getInstance()->playEffect("finish_now.mp3");            // 播放音效
            CCLOG("加速成功！消耗金币: %d", cost);
            _buildLeftTime = 0.0f;            // 直接将时间归零
            // 立即执行完成逻辑 
            if (_isUpgradingTarget) {
                this->upgradeLevel(); // 如果是升级 执行升级逻辑
            }
            else {
                // 初次建造：直接调用完工函数
                this->constructionFinished();
            }
        }
        else {
            CCLOG("加速失败：金币不足！需要 %d", cost);
            // 播放失败音效或飘字提示
        }
        });
    btnSpeedUp->setPosition(Vec2(60, 0)); // 设置位置在最右侧

    // 在按钮下方显示花费的金币
    _speedUpCostLabel = Label::createWithSystemFont("0", "Arial", 12);
    _speedUpCostLabel->setColor(Color3B::YELLOW);
    _speedUpCostLabel->enableOutline(Color4B::BLACK, 1);
    _speedUpCostLabel->setPosition(Vec2(60, -20)); // 放在按钮下面
    _progressNode->addChild(_speedUpCostLabel);

    // * 添加到菜单
    auto menu = Menu::create(btnSpeedUp, nullptr);
    menu->setPosition(Vec2::ZERO);
    _progressNode->addChild(menu); // 加到 _progressNode 上，保证跟随建筑缩放
}

void BaseBuilding::update(float dt) {
    // 只有在“建造中”状态才倒计时
    if (this->state == BuildingState::BUILDING) {
        if (_buildLeftTime > 0) {
            _buildLeftTime -= dt; // 每帧减掉dt

            // 计算进度条的比例 
            float ratio = 1.0f;
            if (_buildTotalTime > 0) {
                ratio = 1.0f - (_buildLeftTime / _buildTotalTime);
            }

            // 改变进度条的长度
            if (_progBar) {
                _progBar->setScaleX(ratio);
            }

            // 刷新剩余时间
            int totalSeconds = (int)ceil(_buildLeftTime);
            if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", totalSeconds));

            // 刷新剩余金币
            if (_speedUpCostLabel) {
                // 保持和按钮逻辑里一样的公式
                int cost = (int)(_buildLeftTime * 10);
                if (cost < 1) cost = 1;
                _speedUpCostLabel->setString(std::to_string(cost) + " G");
            }
        }
       
        else {
            // 时间结束
            _buildLeftTime = 0; 
            // 根据是初始建造还是升级 进行分流处理
            if (_isUpgradingTarget) {
                // 如果是升级，执行升级逻辑 (level++ 并保存)
                this->upgradeLevel();
            }
            else {
                // 初次建造：直接调用完工函数
                this->constructionFinished();
            }
        }
    }
}

void BaseBuilding::startUpgradeProcess() {

    if (this->state != BuildingState::IDLE) return;

    if (this->level >= 3) {
        CCLOG("已达最高等级");
        return;
    }

    BuildingStats nextStats = getStatsConfig(this->type, this->level + 1);

    this->_buildTotalTime = (float)nextStats.buildTime;
    this->_buildLeftTime = this->_buildTotalTime;

    this->_isUpgradingTarget = true;    // 标记此次是升级操作
    this->changeState(BuildingState::BUILDING); // 将状态改成Building状态

    if (!_progressNode) initBuildUI(); // 显示建筑时间进度条
    _progressNode->setVisible(true);

    // 计算放置位置
    float centerX = 0;
    float centerY = 0;

    if (this->mainSprite) {
        // 计算 X 轴中心 
        centerX = this->mainSprite->getPositionX();

        // 计算 Y 轴中心
        centerY = this->mainSprite->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type == BuildingType::GOLD_MINE || this->type == BuildingType::ELIXIR_STORAGE || this->type == BuildingType::GOLD_STORAGE) {
        centerY += 50;
    }
    else if (this->type == BuildingType::BARRACKS || this->type == BuildingType::WALL || this->type == BuildingType::ELIXIR_PUMP) {
        centerY += 100;
    }
    else {
        centerY += 150;
    }
    _progressNode->setPosition(Vec2(centerX, centerY));

    if (_progBar) _progBar->setScaleX(0.0f);    // 重置进度条长度为0

    if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", (int)_buildTotalTime));

    CCLOG("开始升级... 需耗时 %.1f 秒", _buildTotalTime);
}

void BaseBuilding::startConstruction() {
    // 初次建造获取建筑一级的配置
    BuildingStats stats = getStatsConfig(this->type, this->level);

    this->_buildTotalTime = (float)stats.buildTime;    // 设置总时间
    this->_buildLeftTime = this->_buildTotalTime;    // 设置剩余时间

    this->_isUpgradingTarget = false;           // 标记一下不是升级，不需要level++

    this->changeState(BuildingState::BUILDING);    // 4. 切换状态到Building

    if (!_progressNode) initBuildUI();    //  显示建造时间进度条
    _progressNode->setVisible(true);

    // 校准进度条的位置
    float centerX = 0;
    float centerY = 0; 

    if (this->mainSprite) {
        // 1. 计算 X 轴中心 
        centerX = this->mainSprite->getPositionX();

        // 2. 计算 Y 轴中心
        centerY = this->mainSprite->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type == BuildingType::GOLD_MINE || this->type == BuildingType::ELIXIR_STORAGE || this->type == BuildingType::GOLD_STORAGE) {
        centerY += 50;
    }
    else if (this->type == BuildingType::BARRACKS || this->type == BuildingType::WALL || this->type == BuildingType::ELIXIR_PUMP) {
        centerY += 100;
    }
    else {
        centerY += 150;
    }
    _progressNode->setPosition(Vec2(centerX, centerY));


    if (_progBar) _progBar->setScaleX(0.0f);    // 重置进度条长度为0
    if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", (int)_buildTotalTime));

    CCLOG("开始建造 Lv.%d %s... 需耗时 %.1f 秒", this->level, this->name.c_str(), _buildTotalTime);
}

void BaseBuilding::takeDamage(int damage) {}

void BaseBuilding::constructionFinished() {
    // 如果是初次建造，直接完工 (level 不变)
             // 状态变为正常
    this->changeState(BuildingState::IDLE);
    PlayerData::getInstance()->playEffect("Audio/plop.mp3");
    // 隐藏剩余时间进度条
    if (_progressNode) {
        _progressNode->setVisible(false);
    }

    // 完成放大一下表示完成
    auto action = Sequence::create(
        ScaleTo::create(0.1f, this->getScaleX() * 1.2f, this->getScaleY() * 1.2f), // 进行放大1.2
        ScaleTo::create(0.1f, this->getScaleX(), this->getScaleY()), // 回到正常
        nullptr
    );
    // 如果是存储类建筑，建造完成后需要刷新容量
    if (type == BuildingType::GOLD_STORAGE || type == BuildingType::ELIXIR_STORAGE || type == BuildingType::BARRACKS) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    CCLOG("新建筑建造完成！");
}