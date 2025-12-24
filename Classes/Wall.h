#ifndef __WALL_H__
#define __WALL_H__

#include "base_building.h"

class Wall : public BaseBuilding
{
public:
    static Wall* create(int level);
    virtual bool init(int level);

    // “连成一排自动变样子”的功能，可以在这里扩展
};

#endif