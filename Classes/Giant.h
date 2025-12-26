#ifndef __GIANT_H__
#define __GIANT_H__

#include "Soldier.h"

class Giant : public Soldier
{
public:
    /*
    * 创建对象
    * @return 创建成功返回对象指针，失败返回空指针
    */
    static Giant* Create();

    /*
    * 获取图标名称
    * @return 图标文件名
    */
    virtual std::string GetIconName() override;

    /*
    * 获取攻击音效文件路径
    * @return 音效文件路径
    */
    virtual std::string GetAttackSound() override { return "Audio/punch.mp3"; }

    /*
    * 获取偏好对象
	* @return 偏好对象类型
    */
    virtual UnitType GetPreferredTargetType() override { return UnitType::kBuildingDefence; }
};

#endif