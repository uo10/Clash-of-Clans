#include "GameUnit.h"
USING_NS_CC;

//创建对象
GameUnit* GameUnit::create(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type)
{
    GameUnit* pRet = new(std::nothrow) GameUnit();
    if (pRet && pRet->initUnit(filename, maxHp, speed, damage, range, type)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

//初始化对象
bool GameUnit::initUnit(const std::string& filename, float maxHp, float speed, float damage, float range, UnitType type) {
    if (!this->initWithFile(filename)) return false;

    // 初始化属性
    _maxHp = maxHp;
    _currentHp = maxHp;
    _speed = speed;
    _damage = damage;
    _attackRange = range;
    _type = type;

    _attackTimer = 0.0f;
    _target = nullptr;

    //创建血条
    // 添加背景
    auto barBg = Sprite::create("map/bar_bg.png");
    Size barBgSize = barBg->getContentSize();
    Size spriteSize = this->getContentSize();
    barBg->setScaleX(spriteSize.width / barBgSize.width);
    barBg->setScaleY(spriteSize.width / barBgSize.width);
    barBg->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 10));//适应尺寸
    this->addChild(barBg);

    //设置前景
    _hpBar = cocos2d::ui::LoadingBar::create("map/bar_red.png");//便于做百分比血量
    _hpBar->setDirection(cocos2d::ui::LoadingBar::Direction::LEFT);
    _hpBar->setPercent(100);//初始满血
    Size barSize = _hpBar->getContentSize();
    _hpBar->setScaleX(spriteSize.width / barSize.width);
    _hpBar->setScaleY(spriteSize.width / barSize.width);
    _hpBar->setPosition(barBg->getPosition());
    this->addChild(_hpBar);

    return true;
}

//受伤扣血
void GameUnit::getDamage(float damage) {
    if (!isAlive()) return;

    _currentHp -= damage;
    updateHpBar();

    if (_currentHp <= 0) {
        // 死亡逻辑
        this->runAction(Sequence::create(
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}
//更新血条状态
void GameUnit::updateHpBar() {
    if (_hpBar) _hpBar->setPercent((_currentHp / _maxHp) * 100.0f);
}
//按帧更新对象状态
void GameUnit::updateUnit(float dt) {
    if (!_target || !_target->isAlive()) return;
    _attackTimer += dt;
    if (_attackTimer >= 1.0f) { // 1秒攻击一次
        _attackTimer = 0;
        //炸弹兵特判，对围墙超强伤害
        if (_target->getUnitName() == "Fence" && _unitName=="WallBreaker") {
            _target->getDamage(_damage*40);
        }
        else {
            _target->getDamage(_damage);
        }
        CCLOG("%s attacked %s", _unitName.c_str(), _target->getUnitName().c_str());
    }
}
//设置路径
void GameUnit::setPath(const std::vector<Vec2>& path) {
    _path = path;
    _pathIndex = 0;
    if (!_path.empty()) _pathIndex = 1; // 跳过起点
}
//获取下一步落脚点
Vec2 GameUnit::getNextStep() {
    if (_path.empty() || _pathIndex >= _path.size()) return getPosition();
    return _path[_pathIndex];
}