#include "BaseBuilding.h"
//#include "MapUtils.h"

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
            data.buildTime = 5;// 60;
            data.capacity = 1000;
            data.productionRate = 400;
        }
        else if (level == 3) {
            data.name = "Gold Mine Lv3";
            data.hp = 560;
            data.costElixir = 450;
            data.buildTime = 5;// 120;
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
            data.buildTime = 5;// 10;
            data.capacity = 500;
            data.productionRate = 1000000;
        }
        else if (level == 2) {
            data.name = "Elixir Pump Lv2";
            data.hp = 500;
            data.costGold = 300;
            data.buildTime = 5;//30;
            data.capacity = 1000;
            data.productionRate = 400;
        }
        else if (level == 3) {
            data.name = "Elixir Pump Lv3";
            data.hp = 800;
            data.costGold = 600;
            data.buildTime = 5;// 60;
            data.capacity = 2000;
            data.productionRate = 800;
        }
    }
    // 3. 金库配置
    else if (type == BuildingType::GOLD_STORAGE) {
        data.name = "Gold Storage";
        data.hp = 800 + (level * 200);
        data.costElixir = 500 * level; // 金库通常消耗圣水
        data.buildTime = 5;// 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 4. 圣水瓶配置
    else if (type == BuildingType::ELIXIR_STORAGE) {
        data.name = "Elixir Storage";
        data.hp = 800 + (level * 200);
        data.costGold = 500 * level;
        data.buildTime = 5;// 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 5. 兵营配置
    else if (type == BuildingType::BARRACKS) {
        data.name = "Barracks";
        data.hp = 800 + (level * 200);
        data.costElixir = 500 * level;
        data.buildTime = 5;// 30 + (level * 30);
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
            data.buildTime = 5;// 30;
            data.damage = 8;
        }
        else if (level == 3) {
            data.name = "Cannon Lv3";
            data.hp = 420;
            data.costGold = 4000;
            data.buildTime = 5;// 120;
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
            data.buildTime = 5;//15;
            data.damage = 5.5;
        }
        else if (level == 2) {
            data.name = "Archer Tower Lv2";
            data.hp = 420;
            data.costGold = 2000;
            data.buildTime = 5;//120;
            data.damage = 7.5;
        }
        else if (level == 3) {
            data.name = "Archer Tower Lv3";
            data.hp = 460;
            data.costGold = 5000;
            data.buildTime = 5;//1200;
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
            data.buildTime = 5;//15;
        }
        else if (level == 2) {
            data.name = "Wall Lv2";
            data.hp = 200;
            data.costGold = 1000;
            data.buildTime = 5;//120;
        }
        else if (level == 3) {
            data.name = "Wall Lv3";
            data.hp = 400;
            data.costGold = 5000;
            data.buildTime = 5;//1200;
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
            data.buildTime = 5;//120;
        }
        else if (level == 3) {
            data.name = "Town Hall Lv3";
            data.hp = 1600;
            data.costGold = 4000;
            data.buildTime = 5;//1200;
        }
        }
    return data;
}

bool BaseBuilding::init(BuildingType type, int level) {
    // 【关键】开启帧循环
    this->scheduleUpdate();
    // 1. 基础类初始化
    // 假设 BaseBuilding 继承自 Node (作为容器)，这是最稳妥的写法
    if (!Node::init()) return false;

    // 2. 指针置空 (防止野指针)
    this->hpBar = nullptr;
    this->mainSprite = nullptr;

    // 3. 赋值配置
    this->_stats = getStatsConfig(type, level);

    // 4. 赋值属性
    this->type = type;
    this->level = level;
    this->name = _stats.name;
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp;

    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 5. 初始状态
    this->state = BuildingState::IDLE;

    // 6. 刷新状态 显示图片
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

bool BaseBuilding::takeDamage(float damage) {
    // 1. 状态检查
    if (state == BuildingState::DESTROYED || state == BuildingState::PREVIEW) return false;

    // 2. 扣血
    currentHP -= damage;
    if (currentHP < 0) currentHP = 0;

    // 3. 【关键】刷新 UI (这里会自动处理血条的显示和进度)
    this->updateHPBar();

    // 4. 受击反馈动画 (变红闪烁)
    // 只有当主图存在时才跑动画
    if (mainSprite) {
        // 停止之前的变色动作，防止连续攻击导致颜色卡住
        mainSprite->stopAllActionsByTag(999);

        auto tintRed = TintTo::create(0.1f, 255, 100, 100);
        auto tintBack = TintTo::create(0.1f, 255, 255, 255);
        auto seq = Sequence::create(tintRed, tintBack, nullptr);
        seq->setTag(999); // 设置Tag以便打断
        mainSprite->runAction(seq);
    }

    // 5. 死亡检测
    if (currentHP <= 0) {
        changeState(BuildingState::DESTROYED);

        // 确保血条隐藏
        if (hpBarBg) hpBarBg->setVisible(false);

        // 播放一个缩放消失动画，然后隐藏
        if (mainSprite) {
            mainSprite->runAction(Sequence::create(
                ScaleTo::create(0.2f, 0.0f), // 0.2秒缩放到0
                CallFunc::create([this]() {
                    this->setVisible(false); // 彻底隐藏整个节点
                    }),
                nullptr
            ));
        }
        else {
            this->setVisible(false);
        }

        CCLOG("建筑 ID:%d 被摧毁并消失！", instanceID);
        return true;
    }

    return false;
}

    void BaseBuilding::updateView() {
        // 1. 获取图片名称
        std::string baseTextureName = getTextureName(this->type, this->level);

        /* 【Destroyed 状态的特殊处理】
        // 如果是废墟状态，我们要试图加载废墟图，而不是等级图
        if (this->state == BuildingState::DESTROYED) {
            if (this->type == BuildingType::GOLD_MINE) baseTextureName = "Gold_Mine_Ruin.png";
            else if (this->type == BuildingType::ELIXIR_PUMP) baseTextureName = "Elixir_Pump_Ruin.png";
        }*/

        // 2. 精灵创建与纹理更新
        if (!mainSprite) {
            // --- 首次创建 ---
            mainSprite = Sprite::create(baseTextureName);

            // 【崩溃保护】：如果找不到图片，使用备用图
            if (!mainSprite) {
                CCLOG("严重错误：找不到图片资源 '%s'！尝试加载默认图标...", baseTextureName.c_str());
                mainSprite = Sprite::create("HelloWorld.png"); 
            }

            this->addChild(mainSprite);
        }
        else {
            // --- 已经存在，更新图片 ---
            auto newTexture = Director::getInstance()->getTextureCache()->addImage(baseTextureName);
            if (newTexture) {
                mainSprite->setTexture(newTexture);
            }
            else {
                CCLOG("更新视图失败：找不到图片 '%s'", baseTextureName.c_str());
            }
        }
        if (mainSprite) {
            // 1. 让 mainSprite 居中
            Size size = mainSprite->getContentSize();
            mainSprite->setPosition(size.width / 2, size.height / 2);

            // 2. 设置 BaseBuilding 自身的大小等于图片大小
            this->setContentSize(size);

            // 3. 设置锚点为中心 (0.5, 0.5)，这样缩放时才会从中心缩放
            this->setAnchorPoint(Vec2(0.5f, 0.5f));
        }
        // 3. 状态修饰 (颜色、血条等)
        switch (this->state) {
        case BuildingState::PREVIEW:
            mainSprite->setColor(Color3B(100, 255, 100)); // 绿
            mainSprite->setOpacity(128);
            if (hpBar) hpBar->setVisible(false);
            break;

        case BuildingState::BUILDING:
            mainSprite->setColor(Color3B(150, 150, 150)); // 灰
            mainSprite->setOpacity(255);
            if (hpBar) hpBar->setVisible(false);
            break;

        case BuildingState::IDLE:
        case BuildingState::ATTACKING:
            mainSprite->setColor(Color3B::WHITE); // 正常白
            mainSprite->setOpacity(255);
            if (hpBar) hpBar->setVisible(true);
            break;

        case BuildingState::DESTROYED:
            // 直接隐藏整个建筑节点
            this->setVisible(false);
            if (hpBar) hpBar->setVisible(false);
            break;
            break;
        }
    }

void BaseBuilding::changeState(BuildingState newState) {
    if (this->state == newState) return;

    // 1. 改变状态变量
    this->state = newState;

    // 2. 统一刷新画面
    this->updateView();
}

void BaseBuilding::upgradeLevel() {
    //大于三级直接退出
    if (this->level >= 3) {
        return;
    }
    // 1. 逻辑数据升级
    this->level++;

    // 2. 获取新数值配置
    BuildingStats newStats = getStatsConfig(this->type, this->level);
    this->_stats = newStats;

    // 3. 更新通用基础属性
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp; // 升级通常回满血
    this->name = _stats.name;

    // 补全基础属性更新，保持数据一致性
    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 调用虚函数，通知子类更新它们的特有属性
    // 这会让 ResourceProducer 更新 productionRate，让 ResourceStorage 更新 maxLimit
    this->updateSpecialProperties();

    // 4. 通知系统 (刷新容量等)
    if (type == BuildingType::GOLD_STORAGE || type == BuildingType::ELIXIR_STORAGE || type == BuildingType::BARRACKS) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    // 5. 状态回归
    this->state = BuildingState::IDLE;

    // 【新增】隐藏进度条
    if (_progressNode) {
        _progressNode->setVisible(false);
    }

    // 【新增】播放一个升级完成的特效 (可选)
    // 比如闪一下
    auto action = Sequence::create(
        ScaleTo::create(0.1f, 1.2f),
        ScaleTo::create(0.1f, 1.0f),
        nullptr
    );
    mainSprite->runAction(action);

    // 6. 统一刷新画面
    this->updateView();

    CCLOG("升级完成 Lv.%d, 新名称: %s", level, name.c_str());
}

void BaseBuilding::updateHPBar() {
    // 1. 确保血条已经创建
    if (!hpBar || !hpBarBg) {
        initHPBar();
    }

    // 2. 如果被摧毁了，或者满血，隐藏血条
    if (state == BuildingState::DESTROYED || currentHP >= maxHP) {
        hpBarBg->setVisible(false);
        return;
    }

    // 3. 否则显示血条
    hpBarBg->setVisible(true);

    // 4. 计算百分比
    float percent = 0;
    if (maxHP > 0) {
        percent = currentHP / maxHP;
    }

    // 限制在 0~1 之间
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;

    // 5. 设置进度
    hpBar->setScaleX(percent);

    // 6. 颜色变化 (可选)：血量低时变红
    if (percent < 0.3f) {
        hpBar->setColor(Color3B::RED);
    }
    else {
        hpBar->setColor(Color3B::GREEN);
    }
}

void BaseBuilding::initHPBar() {
    if (hpBarBg) return;

    // 1. 背景 (黑底)
    hpBarBg = Sprite::create();
    hpBarBg->setTextureRect(Rect(0, 0, 64, 10)); // 略大一点做边框
    hpBarBg->setColor(Color3B::BLACK);
    hpBarBg->setPosition(Vec2(0, 60)); // 建筑头顶高度，根据需要调整
    this->addChild(hpBarBg, 100);
    hpBarBg->setVisible(false); // 默认隐藏

    // 2. 血条 (绿条)
    hpBar = Sprite::create();
    hpBar->setTextureRect(Rect(0, 0, 60, 6)); // 比背景略小
    hpBar->setColor(Color3B::GREEN);
    hpBar->setAnchorPoint(Vec2(0, 0.5)); // 锚点设在左侧，这样缩放时会向右延伸
    hpBar->setPosition(2, 5); // 相对背景左对齐，留出2像素边框
    hpBarBg->addChild(hpBar);
}

void BaseBuilding::initBuildUI() {
    if (_progressNode) return;

    _progressNode = Node::create();
    _progressNode->setVisible(false);
    this->addChild(_progressNode, 100);

    // 参数 1: 视觉缩放比例
    float visualScale = 0.4f;

    float sx = this->getScaleX();
    float sy = this->getScaleY();

    // 防崩保护
    if (std::abs(sx) < 0.01f) sx = 1.0f;
    if (std::abs(sy) < 0.01f) sy = 1.0f;

    // 设置大小 (抵消建筑形变 * 视觉缩放)
    _progressNode->setScaleX((1.0f / sx) * visualScale);
    _progressNode->setScaleY((1.0f / sy) * visualScale);

    // =============================================================
    // 【位置与大小校准】(保留这一段即可)
    // =============================================================

    float centerX = 0;
    float centerY = 0; // 改个名字，叫 centerY 更合适

    if (this->mainSprite) {
        // 1. 计算 X 轴中心 (保持不变)
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

    // =============================================================
    // 2. 创建锤子图标 (像气泡一样浮动)
    // =============================================================
    _hammerIcon = Sprite::create("icon_hammer.png"); // 建造Icon

    // 防崩：没图就画个圆
    if (!_hammerIcon) {
        auto draw = DrawNode::create();
        draw->drawSolidCircle(Vec2::ZERO, 15, 0, 10, Color4F::ORANGE);
        _hammerIcon = Sprite::create();
        _hammerIcon->addChild(draw);
    }

    // 放在进度条上方
    _hammerIcon->setPosition(0, 40);
    _progressNode->addChild(_hammerIcon);

    // --- 锤子动画 (上下浮动) ---
    auto moveUp = MoveBy::create(0.6f, Vec2(0, 10)); // 向上飘10像素
    auto moveDown = moveUp->reverse();               // 向下飘回
    auto seq = Sequence::create(moveUp, moveDown, nullptr);
    _hammerIcon->runAction(RepeatForever::create(seq));

    // =============================================================
   // 【新增】 时钟图标 (放在进度条左侧)
   // =============================================================
    auto clockIcon = Sprite::create("icon_clock.png"); // 请确保 Resources 下有这张图

    // --- 设置位置 ---
    // 背景条宽84，一半是42。我们放在 -60 的位置，留点空隙
    clockIcon->setPosition(Vec2(-60, 0));

    // --- 调整大小 ---
    if (clockIcon->getContentSize().width > 25) {
        float scale = 25.0f / clockIcon->getContentSize().width;
        clockIcon->setScale(scale);
    }

    _progressNode->addChild(clockIcon);

    // =============================================================
    // 3. 进度条背景 (黑底) - 保持不变
    // =============================================================
    auto bg = Sprite::create();
    bg->setTextureRect(Rect(0, 0, 84, 14));
    bg->setColor(Color3B::BLACK);
    _progressNode->addChild(bg);

    // =============================================================
    // 4. 进度条 (黄条) - 保持不变
    // =============================================================
    _progBar = Sprite::create();
    _progBar->setTextureRect(Rect(0, 0, 80, 10));
    _progBar->setColor(Color3B(255, 200, 0)); // 金黄
    _progBar->setAnchorPoint(Vec2(0, 0.5f));
    _progBar->setPosition(-40, 0);
    _progressNode->addChild(_progBar);

    // =============================================================
    // 5. 文字 - 保持不变
    // =============================================================
    _timeLabel = Label::createWithSystemFont("0s", "Arial", 16);
    _timeLabel->enableOutline(Color4B::BLACK, 1);
    _timeLabel->setPosition(0, 0); // 放在条中间
    _progressNode->addChild(_timeLabel);
}

void BaseBuilding::update(float dt) {
    // 只有在“建造中”状态才倒计时
    if (this->state == BuildingState::BUILDING) {
        if (_buildLeftTime > 0) {
            _buildLeftTime -= dt;

            // 1. 计算比例 (0.0 ~ 1.0)
            float ratio = 1.0f;
            if (_buildTotalTime > 0) {
                ratio = 1.0f - (_buildLeftTime / _buildTotalTime);
            }

            // 2. 【核心修改】使用 setScaleX 改变长度
            if (_progBar) {
                _progBar->setScaleX(ratio);
            }

            // 3. 刷新文字
            int totalSeconds = (int)ceil(_buildLeftTime);
            if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", totalSeconds));
        }
        else {
            // 时间结束
            _buildLeftTime = 0; // 时间结束
            // 【核心修改】 分流处理
            if (_isUpgradingTarget) {
                // 如果是升级，执行升级逻辑 (level++ 并保存)
                this->upgradeLevel();
            }
            else {
                // 如果是初次建造，直接完工 (level 不变)
               // 1. 状态回归正常
                this->changeState(BuildingState::IDLE);

                // 2. 隐藏进度条
                if (_progressNode) {
                    _progressNode->setVisible(false);
                }

                // 3. 播放完工特效 (比如建筑弹一下)
                auto action = Sequence::create(
                    ScaleTo::create(0.1f, this->getScaleX() * 1.2f, this->getScaleY() * 1.2f),
                    ScaleTo::create(0.1f, this->getScaleX(), this->getScaleY()),
                    nullptr
                );
                // 注意：这里要小心，如果用了 scaleX/Y 分离，最好分别获取
                // 简单起见可以用 runAction(JumpBy...) 或者只播放音效

                // 4. 【重要】如果是存储类建筑，建造完成后需要刷新容量
                if (type == BuildingType::GOLD_STORAGE || type == BuildingType::ELIXIR_STORAGE || type == BuildingType::BARRACKS) {
                    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
                }

                CCLOG("新建筑建造完成！");
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

    // 标记这是升级
    this->_isUpgradingTarget = true;
    this->changeState(BuildingState::BUILDING);

    if (!_progressNode) initBuildUI();
    _progressNode->setVisible(true);

    // =============================================================
    // 【位置与大小校准】(保留这一段即可)
    // =============================================================
    float centerX = 0;
    float centerY = 0; // 改个名字，叫 centerY 更合适

    if (this->mainSprite) {
        // 1. 计算 X 轴中心 (保持不变)
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

    // 3. 重置进度
    if (_progBar) _progBar->setScaleX(0.0f);

    if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", (int)_buildTotalTime));

    CCLOG("开始升级... 需耗时 %.1f 秒", _buildTotalTime);
}

void BaseBuilding::startConstruction() {
    // 1. 获取当前等级 (Lv.1) 的配置
    // 注意：这里取的是 this->level，因为是初次建造
    BuildingStats stats = getStatsConfig(this->type, this->level);

    // 2. 设置时间
    this->_buildTotalTime = (float)stats.buildTime;
    this->_buildLeftTime = this->_buildTotalTime;

    // 3. 【关键】标记这不是升级，不需要 level++
    this->_isUpgradingTarget = false;

    // 4. 切换状态 (会自动触发 updateView 变灰)
    this->changeState(BuildingState::BUILDING);

    // 5. 显示进度条 UI
    if (!_progressNode) initBuildUI();
    _progressNode->setVisible(true);

    // 6. 重新校准 UI 位置 (复用之前的逻辑)
    float centerX = 0;
    float centerY = 0; // 改个名字，叫 centerY 更合适

    if (this->mainSprite) {
        // 1. 计算 X 轴中心 (保持不变)
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

    // 7. 重置进度条显示
    if (_progBar) _progBar->setScaleX(0.0f);
    if (_timeLabel) _timeLabel->setString(StringUtils::format("%ds", (int)_buildTotalTime));

    CCLOG("开始建造 Lv.%d %s... 需耗时 %.1f 秒", this->level, this->name.c_str(), _buildTotalTime);
}