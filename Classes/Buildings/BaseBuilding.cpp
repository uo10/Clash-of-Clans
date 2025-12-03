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

    // 容错处理：如果图片没找到，搞个色块顶替
    if (!mainSprite) {
        mainSprite = Sprite::create();
        mainSprite->setTextureRect(Rect(0, 0, 64, 64));
        mainSprite->setColor(Color3B::GRAY);
        CCLOG("Error: 找不到图片 %s", filename.c_str());
    }

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
    case BuildingType::GOLD_MINE:     prefix = "gold_mine"; break;
    case BuildingType::ELIXIR_PUMP:   prefix = "elixir_pump"; break;
    case BuildingType::GOLD_STORAGE:  prefix = "gold_storage"; break;
    case BuildingType::ELIXIR_STORAGE: prefix = "elixir_storage"; break;
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

void BaseBuilding::changeState(BuildingState newState) {
    this->state = newState;

    if (newState == BuildingState::DESTROYED) {
        // 最好准备一张废墟图 "rubble.png"
        // 如果没有，可以用变灰来代替
        mainSprite->setColor(Color3B::GRAY);
        if (hpBar) hpBar->setVisible(false);
    }
}

void BaseBuilding::upgradeLevel() {
    // 1. 逻辑升级
    this->level++;

    // 2. 【关键】重新获取新等级的数值配置
    // 调用之前写的 getStatsConfig，传入新的 level
    // 注意：这里需要把 getStatsConfig 稍微改动一下，让它不是 static，
    // 或者把 getStatsConfig 的定义放在 .cpp 前面让这里能调用
    BuildingStats newStats = getStatsConfig(this->type, this->level);

    this->_stats = newStats;

    // 3. 更新属性 (血量上限增加，产出增加等)
    this->name = _stats.name; // 也应该更新名称
    this->maxHP = _stats.hp;
    this->currentHP = _stats.hp; // 或保持比例

    this->buildCostGold = _stats.costGold;
    this->buildCostElixir = _stats.costElixir;
    this->buildTimeSeconds = _stats.buildTime;

    // 4. 通知子类更新它们的特殊属性
    this->updateSpecialProperties(); // 使用虚拟函数通知子类

    // 5. 触发全局容量更新 (如果是一个存储建筑)
    if (this->type == BuildingType::GOLD_STORAGE || this->type == BuildingType::ELIXIR_STORAGE) { //这只是一个触发器，还需要最终额外实现“REFRESH_MAX_CAPACITY”的指令操作
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    // 6. 【关键】更换图片 (换墙纸)
    // 重新生成图片名称，例如 "gold_mine_2.png"
    std::string newTextureName = getTextureName(this->type, this->level);

    // 直接替换 Sprite 的纹理，不需要移除 Sprite 再创建
    mainSprite->setTexture(newTextureName);

    // 7. 播放一个特效 (可选)
    // 比如播放一个 "升级完成" 的音效或烟雾动画
    CCLOG("建筑升级到了 Lv.%d, 图片换成了 %s", this->level, newTextureName.c_str());
}