#pragma once

#include "cocos2d.h"
#include <string>

enum class CardType {
    Block,
    Resource,
    Tool,
    Weapon,
    Armor,
    Mob,
    Event,
    Structure,
    Enchant
};

struct CardStats {
    int cost = 0;
    int attack = 0;
    int defense = 0;
    int durability = 0;
    int hunger = 0;
    int experience = 0;
};

class Card {
public:
    Card() = default;
    Card(std::string id,
         std::string name,
         CardType type,
         CardStats stats,
         std::string description,
         std::string art);

    const std::string& getId() const;
    const std::string& getName() const;
    CardType getType() const;
    const CardStats& getStats() const;
    const std::string& getDescription() const;
    const std::string& getArt() const;

private:
    std::string id_;
    std::string name_;
    CardType type_ = CardType::Resource;
    CardStats stats_{};
    std::string description_;
    std::string art_;
};
