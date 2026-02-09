#pragma once

#include "Deck.h"
#include "GameState.h"
#include "cocos2d.h"

class GameScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);

private:
    void setupUi();
    void loadDeck();
    void runDemo();
    void updateStatusLabel();

    Deck deck_;
    GameState gameState_;
    cocos2d::Label* rulesLabel_ = nullptr;
    cocos2d::Label* statusLabel_ = nullptr;
};
