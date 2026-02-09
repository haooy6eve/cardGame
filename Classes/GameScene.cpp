#include "GameScene.h"
#include "GameRules.h"
#include <sstream>

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
    runDemo();
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
    rulesLabel_->setDimensions(visibleSize.width - 40.0f, visibleSize.height * 0.55f);
    addChild(rulesLabel_);

    statusLabel_ = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    statusLabel_->setAnchorPoint(Vec2(0.0f, 1.0f));
    statusLabel_->setPosition(origin.x + 20.0f, origin.y + visibleSize.height * 0.45f);
    statusLabel_->setDimensions(visibleSize.width - 40.0f, visibleSize.height * 0.4f);
    addChild(statusLabel_);
}

void GameScene::loadDeck() {
    const bool loaded = deck_.loadFromJson("cards.json");
    if (!loaded) {
        CCLOG("Failed to load cards.json");
    }
}

void GameScene::runDemo() {
    gameState_.startGame(deck_);
    gameState_.setBiome(BiomeType::Forest);
    gameState_.setWeather(WeatherType::Rain);

    const auto recipes = gameState_.getDefaultRecipes();
    for (int turn = 0; turn < 3 && !gameState_.hasVictory() && !gameState_.hasDefeat(); ++turn) {
        gameState_.startTurn();
        gameState_.playAllAffordableCards();

        for (const auto& recipe : recipes) {
            gameState_.craft(recipe);
        }
        gameState_.endTurn();
    }
    updateStatusLabel();
}

void GameScene::updateStatusLabel() {
    std::ostringstream output;
    for (const auto& line : gameState_.getLogs()) {
        output << line << "\n";
    }
    output << "\n状态: " << gameState_.getSummary();
    statusLabel_->setString(output.str());
}
