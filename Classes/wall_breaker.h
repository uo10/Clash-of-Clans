#ifndef __WALL_BREAKER_H__
#define __WALL_BREAKER_H__

#include "soldier.h"

class WallBreaker : public Soldier
{
public:
    /*
    * 创建对象
    * @return 创建成功返回对象指针，失败返回空指针
    */
    static WallBreaker* Create();
    
    /*
    * 获取图标名称
    * @return 图标文件名
    */
    virtual std::string GetIconName() override;

    /*
    * 获取偏好对象
    * @return 偏好对象类型
    */
    virtual UnitType GetPreferredTargetType() override { return UnitType::kWall; }

    /*
	* 更新状态（加入自爆）
	* @param dt 每帧的时间
    */
    virtual void UpdateUnit(float dt) override;
};

#endif