#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Soldier.h"

class Barbarian : public Soldier
{
public:
    /*
    * 创建对象
    * @return 创建成功返回对象指针，失败返回空指针
    */
    static Barbarian* Create();

    /*
    * 获取图标名称
    * @return 图标文件名
    */
    virtual std::string GetIconName() override;

    /*
    * 获取攻击音效文件路径
    * @return 音效文件路径
    */
    virtual std::string GetAttackSound() override { return "Audio/sword_hit1.mp3"; }
};

#endif