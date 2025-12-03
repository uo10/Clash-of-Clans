#ifndef __RESOURCE_STORAGE_H__
#define __RESOURCE_STORAGE_H__

#include "BaseBuilding.h"

class ResourceStorage : public BaseBuilding {
public:
    int maxLimit; // 最大储存量

    static ResourceStorage* create(BuildingType type, int level);
    virtual bool init(BuildingType type, int level) override;
    //根据资源百分比来调整图片的占比
    void ResourceStorage::update(float dt);
};

#endif