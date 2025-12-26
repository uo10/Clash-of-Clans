#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Soldier.h"

class Dragon : public Soldier
{
private:
	cocos2d::Sprite* shadow_sprite = nullptr; // 影子
    cocos2d::Sprite* visual_body = nullptr; // 视觉替身
public:
    /*
    * 创建对象
    * @return 创建成功返回对象指针，失败返回空指针
    */
    static Dragon* Create();
    
    /*
    * 初始化对象（加入悬浮和影子）
    * @param file_name 对象图片名
	* @param max_hp 对象最大血量
	* @param speed 对象移动速度
	* @param damage 对象伤害
	* @param range 对象攻击范围
	* @param type 对象类型
	* 
	* @return 初始化成功返回true 否则返回false
    */
    virtual bool InitUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type) override;
    
    /*
	* 更新状态
	* @param dt 每帧的时间
    */
    virtual void update(float dt) override;
    
    /*
    * 获取图标名称
    * @return 图标文件名
    */
    virtual std::string GetIconName() override;

	/* 
	* 获取用于播放动画的节点
	* @return 视觉节点
    */
    virtual cocos2d::Node* GetVisualNode() { return visual_body; }
    
    /*
    * 获取攻击音效文件路径
    * @return 音效文件路径
    */
    virtual std::string GetAttackSound() override { return "Audio/dragon roar.mp3"; }
};

#endif