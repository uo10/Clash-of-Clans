#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "soldier.h"

class Archer : public Soldier
{
public:
    /*
	* 创建对象
	* @return 创建成功返回对象指针，失败返回空指针
    */
    static Archer* Create();

    /*
	* 获取图标名称
	* @return 图标文件名
    */
    virtual std::string GetIconName() override;

    /*
	* 获取攻击音效文件路径
	* @return 音效文件路径
    */
    virtual std::string GetAttackSound() override { return "Audio/arrow.mp3"; }
};

#endif