#ifndef __GAME_UNIT_H__
#define __GAME_UNIT_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" 

enum class UnitType {
	kSoldier,    // 士兵
	kBuildingDefence, // 防御建筑
	kBuildingResource // 资源建筑
};

class GameUnit:public cocos2d::Sprite
{
public:
	/**
	* 创建一个GameUnit对象
	* 
	* @param file_name 对象图片名
	* @param max_hp 对象最大血量
	* @param speed 对象移动速度
	* @param damage 对象伤害
	* @param range 对象攻击范围
	* @param type 对象类型
	* 
	* @return 创建成功，返回对象，失败则返回空指针
	*/
	static GameUnit* Create(const std::string& file_name, float max_hp, float speed, float damage, float range, UnitType type);
	/**
	* 初始化对象
	* 
	* @param file_name 对象图片名
	* @param max_hp 对象最大血量
	* @param speed 对象移动速度
	* @param damage 对象伤害
	* @param range 对象攻击范围
	* @param type 对象类型
	* 
	* @return 初始化成功返回true 否则返回false
	*/
	virtual bool InitUnit(const std::string& file_name, float max_hp, float speed, float damage, float range, UnitType type);
	/**
	* 对象受到伤害，扣血
	* 
	* @param damage 收到的伤害
	*/
	void GetDamage(float damage);
	/**
	* 检查当前对象是否存活
	* 
	* @return 血量大于0 返回true，否则返回false
	*/
	bool IsAlive() const { return current_hp_>0; }
	/**
	* 设置当前对象的名字
	* 
	* @param name 要设置的名字
	*/
	void SetUnitName(const std::string& name) { unit_name_ = name; }
	/**
	* 设置当前对象的攻击速度
	* 
	* @param time 要设置的攻击速度
	*/
	void SetAttackSpeed(float time) { attack_speed_ = time; }
	/**
	* 设置当前对象的攻击目标
	*
	* @param target 要设置的攻击目标
	*/
	void SetTarget(GameUnit* target) { target_ = target; }
	/**
	*获取当前对象的目标
	* 
	* @return 返回当前对象的目标
	*/
	GameUnit* GetTarget() const { return target_; }
	/**
	* 更新每帧的对象操作
	* 
	* @param dt 每帧的时间
	* 
	*/
	void UpdateUnit(float dt); 
	/**
	* 为对象设置当前路径
	* 
	* @param path 计算好的路径
	*/
	void SetPath(const std::vector<cocos2d::Vec2>& path);
	/**
	* 获取下一步落点
	* 
	* @return 返回下一步的路径点
	*/
	cocos2d::Vec2 GetNextStep();
	/**
	* 路径点数增加
	*/
	void AdvancePath() { path_index_++; };
	/**
	* 检查当前是否有路径
	* 
	* @return 如果路径数组为空返回false ，否则返回true
	*/
	bool HasPath() const { return !path_.empty(); }
	/**
	* 清空路径
	*/
	void ClearPath() { path_.clear(); }
	/**
	* 检查当前路径是否完成，即是否到终点
	* 
	* @return 如果到终点则返回true，否则返回false
	*/
	bool IsPathFinished() const { return path_index_ >= path_.size(); }
	/**
	* 获取对象速度
	* 
	* @return 返回当前对象速度
	*/
	float GetSpeed() const { return speed_; }
	/**
	* 获取攻击范围
	* 
	* @return 返回当前对象攻击范围
	*/
	float GetRange() const { return attack_range_; }
	/**
	* 获取当前对象的类型
	*
	* @return 返回当前对象的类型
	*/
	UnitType GetType() const { return type_; }
	/**
	* 获取当前对象的名字
	* 
	* @return 返回当前对象的名字
	*/
	std::string GetUnitName() const { return unit_name_; }
	/**
	* 获取当前对象的攻击伤害
	*
	* @return 返回当前对象的攻击伤害
	*/
	float GetAttackValue() const { return damage_; }
	/**
    * 获取攻击速度
    *
    * @return 返回当前对象攻击速度
	* 
    */
	float GetAttackSpeed() const { return attack_speed_; }
	/**
	* 获取当前对象阵营
	* 
	* @return 1为我方 2为敌方
	*/
	int GetTeam() const { return (type_ == UnitType::kSoldier) ? 1 : 2; } 
	/**
	* @brief 获取当前对象HP
	* return 当前对象(建筑/士兵)HP
	*/
	float GetCurrentHP() const {
		return current_hp_;
	}

//private:
protected:
	float max_hp_, current_hp_, speed_, damage_, attack_range_;//最大血量、当前血量、移动速度、伤害、攻击范围
	float attack_speed_;//攻速
	UnitType type_;//类型
	std::string unit_name_;//名字
	GameUnit* target_;//攻击目标
	cocos2d::ui::LoadingBar* hp_bar_;//血条
	cocos2d::Sprite* hp_bg_ = nullptr; // 血条背景
	std::vector<cocos2d::Vec2> path_;//攻击路径
	int path_index_;//路径下标
	void UpdateHpBar();//更新血条
};
#endif
