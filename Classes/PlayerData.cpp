#include "PlayerData.h"


USING_NS_CC;
using namespace cocos2d::experimental;

PlayerData* PlayerData::_instance = nullptr;

PlayerData::PlayerData() {
    _totalGold = 20000000;   // 初始金币
    _totalElixir = 20000000; // 初始圣水
    _totalPeople = 0;    // 初始人口容量
    _maxGold = 20000000;  // 初始最大值
    _maxElixir = 20000000;// 初始最大值
    _maxPeople = 0;   // 初始最大值
}

void PlayerData::updateMaxLimits(int maxGold, int maxElixir, int maxPeople) {
    _maxGold = maxGold;
    _maxElixir = maxElixir;
    _maxPeople = maxPeople;

    // 如果当前资源超过了新上限 进行截断处理
    if (_totalGold > _maxGold) _totalGold = _maxGold;
    if (_totalElixir > _maxElixir) _totalElixir = _maxElixir;
    if (_totalPeople > _maxPeople) _totalPeople = _maxPeople;

    // 发送UI更新事件
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
}

PlayerData* PlayerData::getInstance() {
    if (!_instance) {
        _instance = new PlayerData();
    }
    return _instance; // 返回现在的数据
}

int PlayerData::getGold() {
    return _totalGold;
}

int PlayerData::addGold(int amount) {
    const int space = getGoldSpace();

    // 如果已经没有可用的空间
    if (space <= 0) return 0;

    // 实际能加的钱 = min(想加的钱, 剩余空间)
    int realAdd = std::min(amount, space);

    _totalGold += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");

    return realAdd; //在collectResource() 里面UI显示收集的大小
}

bool PlayerData::consumeGold(int amount) {
    if (_totalGold >= amount) {
        _totalGold -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return true;
    }
    return false;
}

int PlayerData::getElixir() {
    return _totalElixir;
}

int PlayerData::addElixir(int amount) {
    const int space = getElixirSpace();

    // 如果没有空间
    if (space <= 0) return 0;

    // 实际能加的圣水 = min(想加的圣水, 剩余空间)
    int realAdd = std::min(amount, space);

    _totalElixir += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");

    return realAdd; 
}

bool PlayerData::consumeElixir(int amount) {
    if (_totalElixir >= amount) {
        _totalElixir -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return true;
    }
    return false;
}

int PlayerData::getPeople() {
    return _totalPeople;
}

bool PlayerData::addPeople(int amount,int cost) {
    if (amount + _totalPeople <= _maxPeople) {
        _totalPeople += amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return true;
    }
    else {
        _totalElixir += cost;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return 0;
    }
}

void PlayerData::removePeople(int amount) {
    _totalPeople -= amount;
    cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");

}

void PlayerData::addTroop(std::string name, int count) {
    _ownedTroops[name] += count;
    CCLOG("PlayerData: %s 数量变更为 %d", name.c_str(), _ownedTroops[name]);
}

bool PlayerData::consumeTroop(std::string name, int count) {
    if (_ownedTroops.find(name) != _ownedTroops.end()) {
        if (_ownedTroops[name] >= count) {
            _ownedTroops[name] -= count;
            CCLOG("PlayerData: %s 消耗 %d, 剩余 %d", name.c_str(), count, _ownedTroops[name]);
            return true;
        }
    }
    return false;
}

int PlayerData::getTroopCount(std::string name) {
    if (_ownedTroops.find(name) != _ownedTroops.end()) {
        return _ownedTroops[name];
    }
    return 0;
}

void PlayerData::setMusicVol(float vol) {
    this->musicVolume = vol; //音量调节为vol

    // 如果当前有音乐在放，实时调整它的音量
    if (_currentBgmID != -1) {
        AudioEngine::setVolume(_currentBgmID, vol);
    }
}

void PlayerData::setEffectVol(float vol) {
    this->effectVolume = vol; //音量调节为vol
}

void PlayerData::playBGM(std::string filename) {
    // 1. 如果当前有音乐在放，先停掉旧的
    if (_currentBgmID != -1) {
        AudioEngine::stop(_currentBgmID);
    }

    // 2. 播放新的 传入的filename的音乐
    _currentBgmID = AudioEngine::play2d(filename, true, this->musicVolume); 

    CCLOG("正在播放 BGM: %s (ID: %d)", filename.c_str(), _currentBgmID);
}

void PlayerData::playEffect(std::string filename) {
    // 播放时读取 effectVolume
    AudioEngine::play2d(filename, false, this->effectVolume);
}

void PlayerData::setLevelStatus(int levelID, bool isWin) {
    if (isWin) {
        // 赢了给三星
        _levelStars[levelID] = 3;
    }
    // 如果输了，什么都不做，保留原样
}

int PlayerData::getLevelStar(int levelID) {
    // 如果没打过，map默认返回0
    // 如果打过了，返回3
    return _levelStars[levelID];
}

bool PlayerData::isLevelLocked(int levelID) {
    // 第 1 关永远解锁
    if (levelID <= 1) return false;

    // 检查上一关 (ID - 1)
    int prevStars = getLevelStar(levelID - 1);

    // 如果上一关通过了，这关就解锁
    if (prevStars == 3) {
        return false; // 解锁
    }
    else {
        return true;  // 锁定
    }
}