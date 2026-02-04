#pragma once

#include "Card.h"
#include "cocos2d.h"
#include <vector>

class Deck {
public:
    bool loadFromJson(const std::string& path);
    void shuffle();
    Card draw();
    bool empty() const;
    const std::vector<Card>& getCards() const;

private:
    std::vector<Card> cards_;
};
