#include "resource_storage.h"
#include "player_data.h" // 引用单例，用于获取当前资源量来改变外观

USING_NS_CC;

ResourceStorage* ResourceStorage::Create(BuildingType type, int level) {
    ResourceStorage* pRet = new(std::nothrow) ResourceStorage();
    if (pRet && pRet->Init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ResourceStorage::Init(BuildingType type, int level) {
    // 1. 调用父类初始化
    // 父类会自动调用 setupSpecialProperties，将 stats.capacity 赋值给 this->maxLimit
    if (!BaseBuilding::Init(type, level)) return false;

    // 2.获取容量信息
    this->max_limit_ = stats_.capacity;

    return true;
}

void ResourceStorage::UpdateSpecialProperties()  {
    // 从父类已更新的 _stats 中获取新值
    this->max_limit_ = stats_.capacity;
}

