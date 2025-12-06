#include "PlayerData.h"

PlayerData* PlayerData::_instance = nullptr;

PlayerData::PlayerData() {
    _totalGold = 0;   // 初始金币
    _totalElixir = 0; // 初始圣水
    _maxGold = 1000;  // 初始最大值
    _maxElixir = 1000;// 初始最大值
}

void PlayerData::updateMaxLimits(int maxGold, int maxElixir) {
    _maxGold = maxGold;
    _maxElixir = maxElixir;

    // 如果当前资源超过了新上限 进行截断处理
    if (_totalGold > _maxGold) _totalGold = _maxGold;
    if (_totalElixir > _maxElixir) _totalElixir = _maxElixir;

    // 发送UI更新事件
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI_GOLD");
}

PlayerData* PlayerData::getInstance() {
    if (!_instance) {
        _instance = new PlayerData();
    }
    return _instance;
}

int PlayerData::getGold() {
    return _totalGold;
}

int PlayerData::addGold(int amount) {
    int space = getGoldSpace();

    // 如果已经没有可用的空间
    if (space <= 0) return 0;

    // 实际能加的钱 = min(想加的钱, 剩余空间)
    int realAdd = std::min(amount, space);

    _totalGold += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI_GOLD");

    return realAdd; //在collectResource() 里面UI显示收集的大小
}

bool PlayerData::consumeGold(int amount) {
    if (_totalGold >= amount) {
        _totalGold -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI_GOLD");
        return true;
    }
    return false;
}

int PlayerData::getElixir() {
    return _totalElixir;
}

int PlayerData::addElixir(int amount) {
    int space = getElixirSpace();

    // 如果没有空间
    if (space <= 0) return 0;

    // 实际能加的圣水 = min(想加的圣水, 剩余空间)
    int realAdd = std::min(amount, space);

    _totalElixir += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI_ELIXIR");

    return realAdd; // 告诉调用者实际上加了多少
}

bool PlayerData::consumeElixir(int amount) {
    if (_totalElixir >= amount) {
        _totalElixir -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI_ELIXIR");
        return true;
    }
    return false;
}