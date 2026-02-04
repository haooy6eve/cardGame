#pragma once

#include "Deck.h"
#include "cocos2d.h"

class GameScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);

private:
    void setupUi();
    void loadDeck();

    Deck deck_;
    cocos2d::Label* rulesLabel_ = nullptr;
};
