#ifndef __CANNON_H__
#define __CANNON_H__

#include "base_building.h"

class Cannon : public BaseBuilding
{
public:
    /*
    * 创建对象
    * @param level 等级
    * @return 创建成功返回对象指针，失败返回空指针
    */
    static Cannon* Create(int level);

    /*
    * 初始化对象
    * @param level 等级
    * @return 初始化成功返回true 否则返回false
    */
    virtual bool init(int level);
};

#endif