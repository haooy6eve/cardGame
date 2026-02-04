#include "GameScene.h"
#include "GameRules.h"

using namespace cocos2d;

Scene* GameScene::createScene() {
    auto scene = Scene::create();
    auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}

bool GameScene::init() {
    if (!Layer::init()) {
        return false;
    }

    setupUi();
    loadDeck();
    return true;
}

void GameScene::setupUi() {
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto origin = Director::getInstance()->getVisibleOrigin();

    auto title = Label::createWithTTF("我的世界卡牌游戏", "fonts/Marker Felt.ttf", 36);
    title->setPosition(origin.x + visibleSize.width * 0.5f,
                       origin.y + visibleSize.height - 40.0f);
    addChild(title);

    rulesLabel_ = Label::createWithTTF(GameRules::summary(), "fonts/Marker Felt.ttf", 20);
    rulesLabel_->setAnchorPoint(Vec2(0.0f, 1.0f));
    rulesLabel_->setPosition(origin.x + 20.0f, origin.y + visibleSize.height - 90.0f);
    rulesLabel_->setDimensions(visibleSize.width - 40.0f, visibleSize.height - 120.0f);
    addChild(rulesLabel_);
}

void GameScene::loadDeck() {
    const bool loaded = deck_.loadFromJson("cards.json");
    if (!loaded) {
        CCLOG("Failed to load cards.json");
    }
}
