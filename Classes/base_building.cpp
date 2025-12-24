#include "base_building.h"
#include"player_data.h"

USING_NS_CC;

BuildingStats BaseBuilding :: GetStatsConfig(BuildingType type, int level) {
    BuildingStats data;

    // 初始化默认值 
    data.name = "Unknown";
    data.hp = 100;
    data.cost_gold = 0;
    data.cost_elixir = 0;
    data.build_time = 0;
    data.capacity = 0;
    data.production_rate = 0;
    data.damage = 0.0f;

    // 1. 金矿配置
    if (type == BuildingType::kGoldMine) {
        if (level == 1) {
            data.name = "Gold Mine Lv1";
            data.hp = 400;
            data.cost_elixir = 150;
            data.build_time = 10;
            data.capacity = 500;
            data.production_rate = 100000000;
        }
        else if (level == 2) {
            data.name = "Gold Mine Lv2";
            data.hp = 480;
            data.cost_elixir = 300;
            data.build_time = 60;
            data.capacity = 1000;
            data.production_rate = 400;
        }
        else if (level == 3) {
            data.name = "Gold Mine Lv3";
            data.hp = 560;
            data.cost_elixir = 450;
            data.build_time = 120;
            data.capacity = 2000;
            data.production_rate = 800;
        }
    }
    // 2. 圣水收集器配置
    else if (type == BuildingType::kElixirPump) {
        if (level == 1) {
            data.name = "Elixir Pump Lv1";
            data.hp = 350;
            data.cost_gold = 150;
            data.build_time = 10;
            data.capacity = 500;
            data.production_rate = 1000000;
        }
        else if (level == 2) {
            data.name = "Elixir Pump Lv2";
            data.hp = 500;
            data.cost_gold = 300;
            data.build_time = 30;
            data.capacity = 1000;
            data.production_rate = 400;
        }
        else if (level == 3) {
            data.name = "Elixir Pump Lv3";
            data.hp = 800;
            data.cost_gold = 600;
            data.build_time = 60;
            data.capacity = 2000;
            data.production_rate = 800;
        }
    }
    // 3. 金库配置
    else if (type == BuildingType::kGoldStorage) {
        data.name = "Gold Storage";
        data.hp = 800 + (level * 200);
        data.cost_elixir = 500 * level; // 金库消耗圣水
        data.build_time = 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 4. 圣水瓶配置
    else if (type == BuildingType::kElixirStorage) {
        data.name = "Elixir Storage";
        data.hp = 800 + (level * 200);
        data.cost_gold = 500 * level;
        data.build_time = 30 + (level * 30);
        data.capacity = 1500 * level;
    }
    // 5. 兵营配置
    else if (type == BuildingType::kBarracks) {
        data.name = "Barracks";
        data.hp = 800 + (level * 200);
        data.cost_elixir = 500 * level;
        data.build_time = 30 + (level * 30);
        data.capacity = 30 * level;
    }
    // 6. 加农炮配置
    else if (type == BuildingType::kCannon) {
        if (level == 1) {
            data.name = "Cannon Lv1";
            data.hp = 300;
            data.cost_gold = 250;
            data.cost_elixir = 0;
            data.build_time = 5;
            data.damage = 5.6;
        }
        else if (level == 2) {
            data.name = "Cannon Lv2";
            data.hp = 360;
            data.cost_gold = 1000;
            data.build_time = 30;
            data.damage = 8;
        }
        else if (level == 3) {
            data.name = "Cannon Lv3";
            data.hp = 420;
            data.cost_gold = 4000;
            data.build_time = 120;
            data.damage = 10.4;
        }
    }
    // 7. 箭塔配置
    else if (type == BuildingType::kArcherTower) {
        if (level == 1) {
            data.name = "Archer Tower Lv1";
            data.hp = 380;
            data.cost_gold = 1000;
            data.cost_elixir = 0;
            data.build_time = 15;
            data.damage = 5.5;
        }
        else if (level == 2) {
            data.name = "Archer Tower Lv2";
            data.hp = 420;
            data.cost_gold = 2000;
            data.build_time = 120;
            data.damage = 7.5;
        }
        else if (level == 3) {
            data.name = "Archer Tower Lv3";
            data.hp = 460;
            data.cost_gold = 5000;
            data.build_time = 1200;
            data.damage = 9.5;
        }
    }
    // 8. 墙配置
    else if (type == BuildingType::kWall) {
        if (level == 1) {
            data.name = "Wall Lv1";
            data.hp = 100;
            data.cost_gold = 0;
            data.cost_elixir = 0;
            data.build_time = 15;
        }
        else if (level == 2) {
            data.name = "Wall Lv2";
            data.hp = 200;
            data.cost_gold = 1000;
            data.build_time = 120;
        }
        else if (level == 3) {
            data.name = "Wall Lv3";
            data.hp = 400;
            data.cost_gold = 5000;
            data.build_time = 1200;
        }
    }
    // 9. 大本营配置
    else if (type == BuildingType::kTownHall) {
        if (level == 1) {
            data.name = "Town Hall Lv1";
            data.hp = 400;
            data.cost_gold = 0;
            data.cost_elixir = 0;
            data.build_time = 0;
        }
        else if (level == 2) {
            data.name = "Town Hall Lv2";
            data.hp = 800;
            data.cost_gold = 1000;
            data.build_time = 120;
        }
        else if (level == 3) {
            data.name = "Town Hall Lv3";
            data.hp = 1600;
            data.cost_gold = 4000;
            data.build_time = 1200;
        }
        }
    return data;
}

bool BaseBuilding::Init(BuildingType type, int level) {
    // 开启帧循环
    this->scheduleUpdate();
    // 基础类初始化
    if (!Node::init()) return false;

    // 指针置空 
    this->main_sprite_ = nullptr;

    // 赋值配置
    this->stats_ = GetStatsConfig(type, level);

    // 赋值属性
    this->type_ = type;
    this->level_ = level;
    this->name_ = stats_.name;
    this->max_hp_ = stats_.hp;
    this->current_hp_ = stats_.hp;

    this->build_cost_gold_ = stats_.cost_gold;
    this->build_cost_elixir_ = stats_.cost_elixir;
    this->build_time_seconds_ = stats_.build_time;

    // 初始状态
    this->state_ = BuildingState::kIdle;

    // 刷新状态 显示图片
    this->UpdateView();

    return true;
}

BaseBuilding* BaseBuilding::Create(BuildingType type, int level) {
    BaseBuilding* pRet = new(std::nothrow) BaseBuilding();
    if (pRet && pRet->Init(type, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 获取建筑图片名字
std::string BaseBuilding::GetTextureName(BuildingType type, int level) {
    std::string prefix = "";
    switch (type) {
    case BuildingType::kGoldMine:     prefix = "Gold_Mine"; break;
    case BuildingType::kElixirPump:   prefix = "Elixir_Pump"; break;
    case BuildingType::kGoldStorage:  prefix = "Gold_Storage"; break;
    case BuildingType::kElixirStorage:prefix = "Elixir_Storage"; break;
    case BuildingType::kBarracks:      prefix = "Barracks"; break;
    case BuildingType::kWall:      prefix = "Wall"; break;
    case BuildingType::kCannon:      prefix = "Cannon"; break;
    case BuildingType::kArcherTower:      prefix = "ArcherTower"; break;
    case BuildingType::kTownHall:      prefix = "TownHall"; break;
    default: prefix = "building"; break;
    }
    return StringUtils::format("Buildings/%s/%s%d.png", prefix.c_str(),prefix.c_str(), level);
}

    void BaseBuilding::UpdateView() {
        // 1. 获取图片名称
        std::string base_texture_name = GetTextureName(this->type_, this->level_);

        // 2. 创建精灵 或者更新照片
        if (!main_sprite_) {
            // 如果是首次创建
            main_sprite_ = Sprite::create(base_texture_name);
            this->addChild(main_sprite_);
        }
        else {
            // 如果是已经存在，更新图片
            auto new_texture = Director::getInstance()->getTextureCache()->addImage(base_texture_name);
            if (new_texture) {
                main_sprite_->setTexture(new_texture);
            }
        }
        if (main_sprite_) {
            // 设置更新后的精灵的位置
            Size size = main_sprite_->getContentSize();
            main_sprite_->setPosition(size.width / 2, size.height / 2);
            // 设置 BaseBuilding 自身的大小等于图片大小
            this->setContentSize(size);
            // 设置锚点为中心 (0.5, 0.5)
            this->setAnchorPoint(Vec2(0.5f, 0.5f));
        }
        // 3. 更换建筑的颜色和状态
        switch (this->state_) {
        case BuildingState::kPreview:
            main_sprite_->setColor(Color3B(100, 255, 100)); // 预览情况下 绿色
            main_sprite_->setOpacity(128);
            break;

        case BuildingState::kBuilding:
            main_sprite_->setColor(Color3B(150, 150, 150)); // 建造情况下 灰色
            main_sprite_->setOpacity(255);
            break;

        case BuildingState::kIdle:
        case BuildingState::kAttacking:
            main_sprite_->setColor(Color3B::WHITE); // 受击情况下 白色
            main_sprite_->setOpacity(255);
            break;

        case BuildingState::kDestroyed:
            this->setVisible(false);          // 被摧毁 直接隐藏建筑
            break;
        }
    }

void BaseBuilding::ChangeState(BuildingState new_state) {
    if (this->state_ == new_state) return;

    // 改变状态变量
    this->state_ = new_state;
    // 刷新建筑画面
    this->UpdateView();
}

void BaseBuilding::UpgradeLevel() {
    //大于三级直接退出
    if (this->level_ >= 3) {
        return;
    }
    // 1. 升级
    this->level_++;

    // 获取升级之后的数据
    BuildingStats new_stats = GetStatsConfig(this->type_, this->level_);
    this->stats_ = new_stats;

    // 更新基础属性
    this->max_hp_ = stats_.hp;
    this->current_hp_ = stats_.hp; // 升级回满血
    this->name_ = stats_.name;
    this->build_cost_gold_ = stats_.cost_gold;
    this->build_cost_elixir_ = stats_.cost_elixir;
    this->build_time_seconds_ = stats_.build_time;

    // 调用虚函数，通知子类更新它们的特有属性
    this->UpdateSpecialProperties();

    // 2. 通知系统 (刷新总的资源容量)
    if (type_ == BuildingType::kGoldStorage || type_ == BuildingType::kElixirStorage || type_ == BuildingType::kBarracks) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    // 升级结束回到正常状态
    this->state_ = BuildingState::kIdle;

    // 3. 隐藏建造进度条
    if (progress_node_) {
        progress_node_->setVisible(false);
    }

    // 闪一下表示升级完成
    auto action = Sequence::create(
        ScaleTo::create(0.1f, 1.2f),
        ScaleTo::create(0.1f, 1.0f),
        nullptr
    );
    main_sprite_->runAction(action);

    PlayerData::GetInstance()->PlayEffect("Audio/plop.mp3");

    // 4. 刷新升级后的画面
    this->UpdateView();

    CCLOG("升级完成 Lv.%d, 新名称: %s", level_, name_.c_str());
}

void BaseBuilding::InitBuildUI() {
    if (progress_node_) return;

    progress_node_ = Node::create();
    progress_node_->setVisible(false);
    this->addChild(progress_node_, 100);

    // 1. 进行视觉缩放比例 同时设置大小
    float visual_scale = 0.4f;

    float s_x = this->getScaleX();
    float s_y = this->getScaleY();

    // 防止找不到图片崩溃
    if (std::abs(s_x) < 0.01f) s_x = 1.0f;
    if (std::abs(s_y) < 0.01f) s_y = 1.0f;

    // 设置大小 
    progress_node_->setScaleX((1.0f / s_x) * visual_scale);
    progress_node_->setScaleY((1.0f / s_y) * visual_scale);
    // 设置建筑位置
    float center_x = 0;
    float center_y = 0;

    if (this->main_sprite_) {
        // 计算 X 轴中心 
        center_x = this->main_sprite_->getPositionX();

        // 计算 Y 轴中心
        center_y = this->main_sprite_->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type_ == BuildingType::kGoldMine || this->type_ == BuildingType::kElixirStorage || this->type_ == BuildingType::kGoldStorage) {
        center_y += 50;
    }
    else if (this->type_ == BuildingType::kBarracks || this->type_ == BuildingType::kWall || this->type_ == BuildingType::kElixirPump) {
        center_y += 100;
    }
    else {
        center_y += 150;
    }
    progress_node_->setPosition(Vec2(center_x, center_y));

    // 2. 创建锤子图标 
    hammer_icon_ = Sprite::create("Icon/icon_hammer.png");

    // 放在进度条上方
    hammer_icon_->setPosition(0, 40);
    progress_node_->addChild(hammer_icon_);

    // 进行浮动效果
    auto move_up = MoveBy::create(0.6f, Vec2(0, 10)); // 向上飘10像素
    auto move_down = move_up->reverse();               // 向下飘回
    auto seq = Sequence::create(move_up, move_down, nullptr);
    hammer_icon_->runAction(RepeatForever::create(seq));

    // 3. 时钟图标 (放在进度条左侧)
    auto clock_icon = Sprite::create("Icon/icon_clock.png");

    // 设置位置
    clock_icon->setPosition(Vec2(-60, 0));

    // 调整时钟的大小
    if (clock_icon->getContentSize().width > 25) {
        float scale = 25.0f / clock_icon->getContentSize().width;
        clock_icon->setScale(scale);
    }

    progress_node_->addChild(clock_icon);

    // 4. 进度条背景
    auto bg = Sprite::create();
    bg->setTextureRect(Rect(0, 0, 84, 14));
    bg->setColor(Color3B::BLACK);   // 黑色背景板
    progress_node_->addChild(bg);

    // 进度条
    prog_bar_ = Sprite::create();
    prog_bar_->setTextureRect(Rect(0, 0, 80, 10));
    prog_bar_->setColor(Color3B(255, 200, 0)); // 黄色
    prog_bar_->setAnchorPoint(Vec2(0, 0.5f));
    prog_bar_->setPosition(-40, 0);
    progress_node_->addChild(prog_bar_);

    // 显示剩余时间
    time_label_ = Label::createWithSystemFont("0s", "Arial", 16);
    time_label_->enableOutline(Color4B::BLACK, 1);
    time_label_->setPosition(0, 0); // 放在中心位置
    progress_node_->addChild(time_label_);

    // 5. 加速按钮 在进度条最右侧

    auto speed_up_wrapper = Node::create();   // 准备按钮容器 
    speed_up_wrapper->setContentSize(Size(30, 30)); // 设定为 30x30 的正方形
    speed_up_wrapper->setAnchorPoint(Vec2(0.5, 0.5));

    auto btn_sprite = Sprite::create("Icon/btn_finish.png");    // 按钮照片

    float btn_scale = 24.0f / std::max(btn_sprite->getContentSize().width, btn_sprite->getContentSize().height); //(24.0f)
    btn_sprite->setScale(btn_scale);    // 缩放适配容器
    btn_sprite->setPosition(15, 15); // 居中
    speed_up_wrapper->addChild(btn_sprite);

    // 创建按钮逻辑
    auto btn_speed_up = MenuItemSprite::create(speed_up_wrapper, nullptr, [=](Ref* sender) {

        // 计算当前加速所需金币
        int cost = static_cast<int>((build_left_time_ * 10));  // 每剩余 1 秒需要 10 金币

        // 扣费与结算
        if (PlayerData::GetInstance()->ConsumeGold(cost)) {
            CCLOG("加速成功！消耗金币: %d", cost);
            build_left_time_ = 0.0f;            // 直接将时间归零
            // 立即执行完成逻辑 
            if (is_upgrading_target_) {
                this->UpgradeLevel(); // 如果是升级 执行升级逻辑
            }
            else {
                // 初次建造：直接调用完工函数
                this->ConstructionFinished();
            }
        }
        else {
            CCLOG("加速失败：金币不足！需要 %d", cost);
        }
        });
    btn_speed_up->setPosition(Vec2(60, 0)); // 设置位置在最右侧

    // 在按钮下方显示花费的金币
    speed_up_cost_label_ = Label::createWithSystemFont("0", "Arial", 12);
    speed_up_cost_label_->setColor(Color3B::YELLOW);
    speed_up_cost_label_->enableOutline(Color4B::BLACK, 1);
    speed_up_cost_label_->setPosition(Vec2(60, -20)); // 放在按钮下面
    progress_node_->addChild(speed_up_cost_label_);

    // 6. 添加到菜单
    auto menu = Menu::create(btn_speed_up, nullptr);
    menu->setPosition(Vec2::ZERO);
    progress_node_->addChild(menu); // 加到 _progressNode 上，保证跟随建筑缩放
}

void BaseBuilding::update(float dt) {
    // 只有在“建造中”状态才倒计时
    if (this->state_ == BuildingState::kBuilding) {
        if (build_left_time_ > 0) {
            build_left_time_ -= dt; // 每帧减掉dt

            // 1. 计算进度条的比例 
            float ratio = 1.0f;
            if (build_total_time_ > 0) {
                ratio = 1.0f - (build_left_time_ / build_total_time_);
            }

            // 2. 改变进度条的长度
            if (prog_bar_) {
                prog_bar_->setScaleX(ratio);
            }

            // 3. 刷新剩余时间
            int total_seconds = static_cast<int>(ceil(build_left_time_));
            if (time_label_) time_label_->setString(StringUtils::format("%ds", total_seconds));

            // 4. 刷新剩余金币
            if (speed_up_cost_label_) {
                // 保持和按钮逻辑里一样的公式
                int cost = static_cast<int>(build_left_time_ * 10);
                if (cost < 1) cost = 1;
                speed_up_cost_label_->setString(std::to_string(cost) + " G");
            }
        }
       
        else {
            // 时间结束
            build_left_time_ = 0; 
            // 根据是初始建造还是升级 进行分流处理
            if (is_upgrading_target_) {
                // 1. 如果是升级，执行升级逻辑 (level++ 并保存)
                this->UpgradeLevel();
            }
            else {
                // 2. 初次建造：直接调用完工函数
                this->ConstructionFinished();
            }
        }
    }
}

void BaseBuilding::StartUpgradeProcess() {

    if (this->state_ != BuildingState::kIdle) return;

    if (this->level_ >= 3) {
        CCLOG("已达最高等级");
        return;
    }
    // 1. 获取下一级配置
    BuildingStats next_stats = GetStatsConfig(this->type_, this->level_ + 1);

    this->build_total_time_ = static_cast<float>(next_stats.build_time);
    this->build_left_time_ = this->build_total_time_;

    this->is_upgrading_target_ = true;    // 标记此次是升级操作
    this->ChangeState(BuildingState::kBuilding); // 将状态改成Building状态

    if (!progress_node_) InitBuildUI(); // 显示建筑时间进度条
    progress_node_->setVisible(true);

    // 2. 计算放置位置
    float center_x = 0;
    float center_y = 0;

    if (this->main_sprite_) {
        // 计算 X 轴中心 
        center_x = this->main_sprite_->getPositionX();

        // 计算 Y 轴中心
        center_y = this->main_sprite_->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type_ == BuildingType::kGoldMine || this->type_ == BuildingType::kElixirStorage || this->type_ == BuildingType::kGoldStorage) {
        center_y += 50;
    }
    else if (this->type_ == BuildingType::kBarracks || this->type_ == BuildingType::kWall || this->type_ == BuildingType::kElixirPump) {
        center_y += 100;
    }
    else {
        center_y += 150;
    }
    progress_node_->setPosition(Vec2(center_x, center_y));

    if (prog_bar_) prog_bar_->setScaleX(0.0f);    // 重置进度条长度为0

    if (time_label_) time_label_->setString(StringUtils::format("%ds", static_cast<int>(build_total_time_)));

    CCLOG("开始升级... 需耗时 %.1f 秒", build_total_time_);
}

void BaseBuilding::StartConstruction() {
    // 1. 初次建造获取建筑一级的配置
    BuildingStats stats = GetStatsConfig(this->type_, this->level_);

    this->build_total_time_ = static_cast<float>(stats.build_time);    // 设置总时间
    this->build_left_time_ = this->build_total_time_;    // 设置剩余时间

    this->is_upgrading_target_ = false;           // 标记一下不是升级，不需要level++

    this->ChangeState(BuildingState::kBuilding);    // 4. 切换状态到Building

    if (!progress_node_) InitBuildUI();    //  显示建造时间进度条
    progress_node_->setVisible(true);

    // 2. 校准进度条的位置
    float center_x = 0;
    float center_y = 0; 

    if (this->main_sprite_) {
        // 1. 计算 X 轴中心 
        center_x = this->main_sprite_->getPositionX();

        // 2. 计算 Y 轴中心
        center_y = this->main_sprite_->getPositionY();
    }
    // 根据不同建筑大小进行微调
    if (this->type_ == BuildingType::kGoldMine || this->type_ == BuildingType::kElixirStorage || this->type_ == BuildingType::kGoldStorage) {
        center_y += 50;
    }
    else if (this->type_ == BuildingType::kBarracks || this->type_ == BuildingType::kWall || this->type_ == BuildingType::kElixirPump) {
        center_y += 100;
    }
    else {
        center_y += 150;
    }
    progress_node_->setPosition(Vec2(center_x, center_y));


    if (prog_bar_) prog_bar_->setScaleX(0.0f);    // 重置进度条长度为0
    if (time_label_) time_label_->setString(StringUtils::format("%ds", static_cast<int>(build_total_time_)));

    CCLOG("开始建造 Lv.%d %s... 需耗时 %.1f 秒", this->level_, this->name_.c_str(), build_total_time_);
}

void BaseBuilding::TakeDamage(int damage) {}

void BaseBuilding::ConstructionFinished() {
    // 如果是初次建造，直接完工 (level 不变)
    this->ChangeState(BuildingState::kIdle);    // 状态变为正常
    PlayerData::GetInstance()->PlayEffect("Audio/plop.mp3");
    // 1. 隐藏剩余时间进度条
    if (progress_node_) {
        progress_node_->setVisible(false);
    }

    // 完成放大一下表示完成
    auto action = Sequence::create(
        ScaleTo::create(0.1f, this->getScaleX() * 1.2f, this->getScaleY() * 1.2f), // 进行放大1.2
        ScaleTo::create(0.1f, this->getScaleX(), this->getScaleY()), // 回到正常
        nullptr
    );
    // 2. 如果是存储类建筑，建造完成后需要刷新容量
    if (type_ == BuildingType::kGoldStorage || type_ == BuildingType::kElixirStorage || type_ == BuildingType::kBarracks) {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("REFRESH_MAX_CAPACITY");
    }

    CCLOG("新建筑建造完成！");
}