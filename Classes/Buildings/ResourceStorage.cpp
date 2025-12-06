#include "ResourceStorage.h"
#include "PlayerData.h" // 引用单例，用于获取当前资源量来改变外观

USING_NS_CC;

ResourceStorage* ResourceStorage::create(BuildingType type, int level) {
    ResourceStorage* pRet = new(std::nothrow) ResourceStorage();
    if (pRet && pRet->init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ResourceStorage::init(BuildingType type, int level) {
    // 1. 调用父类初始化
    // 父类会自动调用 setupSpecialProperties，将 stats.capacity 赋值给 this->maxLimit
    if (!BaseBuilding::init(type, level)) return false;

    // 2.获取容量信息
    this->maxLimit = _stats.capacity;

    // 3. 开启 Update (可选)
    // 虽然存储建筑不产资源，但如果想实现“金库里的金币随总量变化而升降”的动画，需要 update
    //this->scheduleUpdate();

    return true;
}

void ResourceStorage::updateSpecialProperties() override {
    // 从父类已更新的 _stats 中获取新值
    this->maxLimit = _stats.capacity;
}

void ResourceStorage::update(float dt) {
    // 这里可以做一个高级功能：根据当前仓库饱满度，切换图片
    // 比如：0%用 empty.png, 50%用 half.png, 100%用 full.png

    // 简单的实现逻辑示例：
    // 1. 获取全局资源比例
    float percent = 0.0f;
    auto player = PlayerData::getInstance();

    if (this->type == BuildingType::GOLD_STORAGE) {
        // 防止除以0
        if (player->getGoldSpace() + player->getGold() > 0) {
            percent = (float)player->getGold() / (float)(player->getGold() + player->getGoldSpace());
        }
    }
    else if (this->type == BuildingType::ELIXIR_STORAGE) {
        if (player->getElixirSpace() + player->getElixir() > 0) {
            percent = (float)player->getElixir() / (float)(player->getElixir() + player->getElixirSpace());
        }
    }

    // 2. 根据比例切换 SpriteFrame 
    /*
    if (percent < 0.2f) mainSprite->setSpriteFrame("gold_storage_empty.png");
    else if (percent < 0.6f) mainSprite->setSpriteFrame("gold_storage_half.png");
    else mainSprite->setSpriteFrame("gold_storage_full.png");
    */
}

