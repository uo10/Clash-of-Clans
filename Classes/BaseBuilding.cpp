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
        data.costElixir = 500 * level; // 金库通常消耗圣水
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

    return data;
}

bool BaseBuilding::init(BuildingType type, int level) {
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
    case BuildingType::ELIXIR_STORAGE: prefix = "Elixir_Storage"; break;
    default: prefix = "building"; break;
    }
    return StringUtils::format("%s%d.png", prefix.c_str(), level);
}

bool BaseBuilding::takeDamage(float damage) {
    if (state == BuildingState::DESTROYED) return true;

    currentHP -= damage;
    if (currentHP < 0) currentHP = 0;

    if (hpBar) {
        hpBar->setVisible(true);
        float percent = (currentHP / maxHP) * 100.0f;
        hpBar->setPercent(percent);
    }

    auto tintRed = TintTo::create(0.1f, 255, 100, 100);
    auto tintBack = TintTo::create(0.1f, 255, 255, 255);
    mainSprite->runAction(Sequence::create(tintRed, tintBack, nullptr));

    if (currentHP <= 0) {
        changeState(BuildingState::DESTROYED);
        CCLOG("建筑 ID:%d 被摧毁了！", instanceID);
        return true;
    }

    return false;
}

    void BaseBuilding::updateView() {
        // 1. 获取图片名称
        std::string baseTextureName = getTextureName(this->type, this->level);

        // 【Destroyed 状态的特殊处理】
        // 如果是废墟状态，我们要试图加载废墟图，而不是等级图
        if (this->state == BuildingState::DESTROYED) {
            if (this->type == BuildingType::GOLD_MINE) baseTextureName = "Gold_Mine_Ruin.png";
            else if (this->type == BuildingType::ELIXIR_PUMP) baseTextureName = "Elixir_Pump_Ruin.png";
        }

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
            // 已经在上面处理了图片更换 (Ruin.png)
            mainSprite->setColor(Color3B::BLACK);
            mainSprite->setOpacity(200);
            if (hpBar) hpBar->setVisible(false);
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
    if (type == BuildingType::GOLD_STORAGE || type == BuildingType::ELIXIR_STORAGE) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    // 5. 状态回归
    this->state = BuildingState::IDLE;

    // 6. 统一刷新画面
    this->updateView();

    CCLOG("升级完成 Lv.%d, 新名称: %s", level, name.c_str());
}