#ifndef __GAME_UNIT_H__
#define __GAME_UNIT_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" 

enum class UnitType {
	SOLDIER,    // 士兵
	BUILDING_DEFENSE, // 防御建筑
	BUILDING_RESOURCE // 资源建筑
};

class GameUnit:public cocos2d::Sprite
{
public:
	/**
	* 创建一个GameUnit对象
	* 
	* @param filename 对象图片名
	* @param maxHp 对象最大血量
	* @param speed 对象移动速度
	* @param damage 对象伤害
	* @param range 对象攻击范围
	* @param type 对象类型
	* 
	* @return 创建成功，返回对象，失败则返回空指针
	*/
	static GameUnit* create(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type);
	/**
	* 初始化对象
	* 
	* @param filename 对象图片名
	* @param maxHp 对象最大血量
	* @param speed 对象移动速度
	* @param damage 对象伤害
	* @param range 对象攻击范围
	* @param type 对象类型
	* 
	* @return 初始化成功返回true 否则返回false
	*/
	virtual bool initUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type);
	/**
	* 对象受到伤害，扣血
	* 
	* @param damage 收到的伤害
	*/
	void getDamage(float damage);
	/**
	* 检查当前对象是否存活
	* 
	* @return 血量大于0 返回true，否则返回false
	*/
	bool isAlive() const { return _currentHp>0; }
	/**
	* 设置当前对象的名字
	* 
	* @param name 要设置的名字
	*/
	void setUnitName(const std::string& name) { _unitName = name; }
	/**
	* 设置当前对象的攻击速度
	* 
	* @param time 要设置的攻击速度
	*/
	void setAttackTime(float time) { _attackTimer = time; }
	/**
	* 设置当前对象的攻击目标
	*
	* @param target 要设置的攻击目标
	*/
	void setTarget(GameUnit* target) { _target = target; }
	/**
	*获取当前对象的目标
	* 
	* @return 返回当前对象的目标
	*/
	GameUnit* getTarget() const { return _target; }
	/**
	* 更新每帧的对象操作
	* 
	* @param dt 每帧的时间
	* 
	*/
	void updateUnit(float dt); 
	/**
	* 为对象设置当前路径
	* 
	* @param path 计算好的路径
	*/
	void setPath(const std::vector<cocos2d::Vec2>& path);
	/**
	* 获取下一步落点
	* 
	* @return 返回下一步的路径点
	*/
	cocos2d::Vec2 getNextStep();
	/**
	* 路径点数增加
	*/
	void advancePath() { _pathIndex++; };
	/**
	* 检查当前是否有路径
	* 
	* @return 如果路径数组为空返回false ，否则返回true
	*/
	bool hasPath() const { return !_path.empty(); }
	/**
	* 清空路径
	*/
	void clearPath() { _path.clear(); }
	/**
	* 检查当前路径是否完成，即是否到终点
	* 
	* @return 如果到终点则返回true，否则返回false
	*/
	bool isPathFinished() const { return _pathIndex >= _path.size(); }
	/**
	* 获取对象速度
	* 
	* @return 返回当前对象速度
	*/
	float getSpeed() const { return _speed; }
	/**
	* 获取攻击范围
	* 
	* @return 返回当前对象攻击范围
	*/
	float getRange() const { return _attackRange; }
	/**
	* 获取当前对象的类型
	*
	* @return 返回当前对象的类型
	*/
	UnitType getType() const { return _type; }
	/**
	* 获取当前对象的名字
	* 
	* @return 返回当前对象的名字
	*/
	std::string getUnitName() const { return _unitName; }
	/**
    * 获取攻击速度
    *
    * @return 返回当前对象攻击速度
	* +
    */
	float getAttackTime() const { return _attackTimer; }
	/**
	* 获取当前对象阵营
	* 
	* @return 1为我方 2为敌方
	*/
	int getTeam() const { return (_type == UnitType::SOLDIER) ? 1 : 2; } 
	/**
	* @brief 获取当前对象HP
	* return 当前对象(建筑/士兵)HP
	*/
	float getCurrentHP() const {
		return _currentHp;
	}

private:
	float _maxHp, _currentHp, _speed, _damage, _attackRange;//最大血量、当前血量、移动速度、伤害、攻击范围
	float _attackTimer;//攻速
	UnitType _type;//类型
	std::string _unitName;//名字
	GameUnit* _target;//攻击目标
	cocos2d::ui::LoadingBar* _hpBar;//血条
	std::vector<cocos2d::Vec2> _path;//攻击路径
	int _pathIndex;//路径下标
	void updateHpBar();//更新血条
};
#endif
