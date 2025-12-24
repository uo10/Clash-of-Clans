#include "player_data.h"


USING_NS_CC;
using namespace cocos2d::experimental;

PlayerData* PlayerData::instance_ = nullptr;

PlayerData::PlayerData() {
    total_gold_ = 20000000;   // 初始金币
    total_elixir_ = 20000000; // 初始圣水
    total_people_ = 0;    // 初始人口容量
    max_gold_ = 20000000;  // 初始最大值
    max_elixir_ = 20000000;// 初始最大值
    max_people_ = 0;   // 初始最大值
}

void PlayerData::UpdateMaxLimits(int max_gold, int max_elixir, int max_people) {
    // 更新新的最大值
    max_gold_ = max_gold;
    max_elixir_ = max_elixir;
    max_people_ = max_people;

    // 如果当前资源超过了新上限 进行截断处理
    if (total_gold_ > max_gold_) total_gold_ = max_gold_;
    if (total_elixir_ > max_elixir_) total_elixir_ = max_elixir_;
    if (total_people_ > max_people_) total_people_ = max_people_;

    // 发送UI更新事件
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
}

PlayerData* PlayerData::GetInstance() {
    if (!instance_) {
        instance_ = new PlayerData();
    }
    return instance_; // 返回现在的数据
}

int PlayerData::GetGold() {
    return total_gold_;
}

int PlayerData::AddGold(int amount) {
    const int space = GetGoldSpace();

    // 如果已经没有可用的空间
    if (space <= 0) return 0;

    // 实际能加的钱 = min(想加的钱, 剩余空间)
    int realAdd = std::min(amount, space);

    total_gold_ += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");

    return realAdd; //在collectResource() 里面UI显示收集的大小
}

bool PlayerData::ConsumeGold(int amount) {
    if (total_gold_ >= amount) {
        total_gold_ -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI"); 
        return true; // 扣费成功 刷新UI
    }
    return false;// 扣费失败
}

int PlayerData::GetElixir() {
    return total_elixir_;
}

int PlayerData::AddElixir(int amount) {
    const int space = GetElixirSpace();

    // 如果没有空间
    if (space <= 0) return 0;

    // 实际能加的圣水 = min(想加的圣水, 剩余空间)
    int realAdd = std::min(amount, space);

    total_elixir_ += realAdd;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");

    return realAdd; 
}

bool PlayerData::ConsumeElixir(int amount) {
    if (total_elixir_ >= amount) {
        total_elixir_ -= amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return true; // 扣费成功 刷新UI
    }
    return false; // 扣费失败
}

int PlayerData::GetPeople() {
    return total_people_; // 返回现有人数
}

bool PlayerData::AddPeople(int amount,int cost) {
    if (amount + total_people_ <= max_people_) {
        total_people_ += amount;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return true; // 人口充足，增加人口成功
    }
    else {
        total_elixir_ += cost;
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
        return false; // 人口不足，增加失败，同时返还消耗的圣水
    }
}

void PlayerData::RemovePeople(int amount) {
    total_people_ -= amount; // 返还人口
    cocos2d::Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_UI");
}

void PlayerData::AddTroop(std::string name, int count) {
    owned_troops_[name] += count; // 对应士兵的数量增加
    CCLOG("PlayerData: %s 数量变更为 %d", name.c_str(), owned_troops_[name]);
}

bool PlayerData::ConsumeTroop(std::string name, int count) {
    if (owned_troops_.find(name) != owned_troops_.end()) {
        if (owned_troops_[name] >= count) {
            owned_troops_[name] -= count;
            CCLOG("PlayerData: %s 消耗 %d, 剩余 %d", name.c_str(), count, owned_troops_[name]);
            return true; // 消耗对应的士兵的数量
        }
    }
    return false; // 数量不足 没有该士兵
}

int PlayerData::GetTroopCount(std::string name) {
    if (owned_troops_.find(name) != owned_troops_.end()) {
        return owned_troops_[name]; // 获取该士兵数量
    } 
    return 0;
}

void PlayerData::SetMusicVol(float vol) {
    this->music_volume_ = vol; //音量调节为vol

    // 如果当前有音乐在放，实时调整它的音量
    if (current_bgm_id_ != -1) {
        AudioEngine::setVolume(current_bgm_id_, vol);
    }
}

void PlayerData::SetEffectVol(float vol) {
    this->effect_volume_ = vol; //音量调节为vol
}

void PlayerData::PlayBgm(std::string filename,bool opt) {
    // 1. 如果当前有音乐在放，先停掉旧的
    if (current_bgm_id_ != -1) {
        AudioEngine::stop(current_bgm_id_);
    }

    // 2. 播放新的 传入的filename的音乐
    current_bgm_id_ = AudioEngine::play2d(filename, opt, this->music_volume_); 

    CCLOG("正在播放 BGM: %s (ID: %d)", filename.c_str(), current_bgm_id_);
}

void PlayerData::PlayEffect(std::string filen_ame) {
    // 播放时读取 effectVolume
    AudioEngine::play2d(filen_ame, false, this->effect_volume_);
}

void PlayerData::SetLevelStatus(int level_id, bool is_win) {
    if (is_win) {
        // 赢了给三星
        level_stars_[level_id] = 3;
    }
    // 如果输了，什么都不做，保留原样
}

int PlayerData::GetLevelStar(int level_id) {
    // 如果没打过，map默认返回0
    // 如果打过了，返回3
    return level_stars_[level_id];
}

bool PlayerData::IsLevelLocked(int level_id) {
    // 第 1 关永远解锁
    if (level_id <= 1) return false;

    // 检查上一关 (ID - 1)
    int prev_stars = GetLevelStar(level_id - 1);

    // 如果上一关通过了，这关就解锁
    if (prev_stars == 3) {
        return false; // 解锁
    }
    else {
        return true;  // 锁定
    }
}