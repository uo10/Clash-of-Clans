#ifndef __PLAYER_DATA_H__
#define __PLAYER_DATA_H__

#include "cocos2d.h"

class PlayerData {
private:
    // 私有构造函数，防止外部随便 new
    PlayerData(); 
    static PlayerData* _instance;

    int _totalGold;   // 玩家总金币
    int _totalElixir; // 玩家总圣水

    //资源上限 //需要每一次在对建筑进行操作之后，更新资源上限
    int _maxGold;
    int _maxElixir;
public:
    // 获取单例实例
    static PlayerData* getInstance();//获取上限

    // 1. 设置上限 (由 GameScene 统计后调用)
    void updateMaxLimits(int maxGold, int maxElixir);

    // 2. 获取剩余空间 
    int getGoldSpace() { return _maxGold - _currentGold; }
    int getElixirSpace() { return _maxElixir - _currentElixir; }

    // 金币操作
    int getGold();
    int addGold(int amount);
    bool consumeGold(int amount); // 消费金币(比如造建筑用)

    // 圣水操作
    int getElixir();
    int addElixir(int amount);
    bool consumeElixir(int amount);
};

#endif
