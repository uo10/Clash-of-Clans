#include "BaseBuilding.h"
#include "MapUtils.h"

USING_NS_CC;

// --- 静态初始化配置函数 ---
static BuildingStats getStatsConfig(BuildingType type, int level) {
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
            data.productionRate = 200;
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
            data.productionRate = 200;
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

// --- 核心修改：init 函数 ---
bool BaseBuilding::init(BuildingType type, int level) {
    if (!Node::init()) return false;

    // 1. 获取配置
    this->_stats = getStatsConfig(type, level); //保存到成员变量

    // 2. 通用属性赋值
    this->type = type; 
    this->level = level; 
    this->name = _stats.name;
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp;

    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 4. 初始化状态
    this->state = BuildingState::IDLE; // 默认为空闲，如果是刚造的可以在外部设为 BUILDING

    // 5. 创建主图片
    std::string filename = getTextureName(type, level);
    mainSprite = Sprite::create(filename);

    // 设置锚点：脚底中心 为了看着像站在中间
    mainSprite->setAnchorPoint(Vec2(0.5f, 0.0f));
    this->addChild(mainSprite);

    // 6. 默认占地 
    //不确定占地，先写死
    // this->width = (type == BuildingType::TOWN_HALL) ? 4 : 3;//大本营大小不同？
    this->width = 3;
    this->height = 3;

    return true;
}

// create 函数
BaseBuilding* BaseBuilding::create(BuildingType type, int level) {
    BaseBuilding* pRet = new(std::nothrow) BaseBuilding();
    if (pRet && pRet->init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}
//利用MapUtils里面的计算公式 来计算出实际上在屏幕上的像素位置
void BaseBuilding::setGridPosition(int x, int y) {
    this->gridX = x;
    this->gridY = y;
    Vec2 screenPos = MapUtils::gridToScreen(x, y);
    this->setPosition(screenPos);
    this->setLocalZOrder(10000 - (int)screenPos.y);
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
    return StringUtils::format("%s_%d.png", prefix.c_str(), level);
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

    // 1. 底图处理 无论什么状态，先确定当前等级对应的图片
    std::string baseTextureName = getTextureName(this->type, this->level);

    // 如果是 PREVIEW 状态，可能还没有 level，默认显示 1 级外观
    if (this->state == BuildingState::PREVIEW && this->level == 0) {
        baseTextureName = getTextureName(this->type, 1);
    }

    if (!mainSprite) {
        mainSprite = Sprite::create(baseTextureName);
        this->addChild(mainSprite);
    }
    else {
        // 如果图片变了，重新设置
        mainSprite->setTexture(baseTextureName);
    }

    // 2. 【状态修饰】根据不同状态调整颜色、透明度、UI组件显隐

    // 辅助函数：根据名字移除图标（避免代码重复）
    auto removeIcon = [&](const std::string& name) {
        if (auto child = this->getChildByName(name)) {
            child->removeFromParent();
        }
        };

    switch (this->state) {
    case BuildingState::PREVIEW:
    {
        // 【预览模式】：半透明，带一点绿色（表示可放置），无血条
        mainSprite->setColor(Color3B(100, 255, 100)); // 偏绿
        mainSprite->setOpacity(128);                  // 半透明

        if (hpBar) hpBar->setVisible(false);
        removeIcon("icon_hammer");
    }
    break;

    case BuildingState::BUILDING:
    {
        // 建造/升级中：建筑变暗，显示施工图标
        mainSprite->setColor(Color3B(150, 150, 150)); // 变暗
        mainSprite->setOpacity(255);

        if (hpBar) hpBar->setVisible(false); // 施工时通常不显示血条

        // 添加一个施工的小锤子图标
        if (!this->getChildByName("icon_hammer")) {
            auto hammer = Sprite::create("hammer.png");
            if (hammer) {
                hammer->setName("icon_hammer");
                hammer->setPosition(Vec2(0, 50)); // 放在头顶
                // 加个简单的动画
                auto action = RepeatForever::create(Sequence::create(
                    RotateTo::create(0.2f, 30),
                    RotateTo::create(0.2f, -30),
                    nullptr
                ));
                hammer->runAction(action);
                this->addChild(hammer, 10);
            }
        }
    }
    break;

    case BuildingState::IDLE:
    {
        // 正常待机：恢复原色，显示血条
        mainSprite->setColor(Color3B::WHITE);
        mainSprite->setOpacity(255);

        if (hpBar) hpBar->setVisible(true);

        // 清理掉施工图标
        removeIcon("icon_hammer");
    }
    break;

    case BuildingState::ATTACKING:
    {
        // 攻击中：视觉上通常和 IDLE 类似，或者是播放攻击动画的起点
        mainSprite->setColor(Color3B::WHITE);
        mainSprite->setOpacity(255);

        if (hpBar) hpBar->setVisible(true);
        removeIcon("icon_hammer");

        // 这里不需要写“发射子弹”的逻辑， updateView 只管静态表现。
        // 攻击逻辑应该在 update(float dt) 里判断 if(state == ATTACKING) loop...
    }
    break;

    case BuildingState::DESTROYED:
    {
        if (this->type == BuildingType::GOLD_MINE) {
            mainSprite = Sprite::create("Gold_Mine_Ruin.png");
            this->addChild(mainSprite);
        }
        else if (this->type == BuildingType::ELIXIR_STORAGE) {
            mainSprite = Sprite::create("Elixir_Pump_Ruin.png");
            this->addChild(mainSprite);
        }
        if (hpBar) hpBar->setVisible(false);
        removeIcon("icon_hammer");
    }
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