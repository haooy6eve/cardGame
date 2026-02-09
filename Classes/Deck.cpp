#include "Deck.h"
#include "json/document.h"

using cocos2d::FileUtils;

namespace {
CardType toCardType(const std::string& type) {
    if (type == "Block") return CardType::Block;
    if (type == "Resource") return CardType::Resource;
    if (type == "Tool") return CardType::Tool;
    if (type == "Weapon") return CardType::Weapon;
    if (type == "Armor") return CardType::Armor;
    if (type == "Mob") return CardType::Mob;
    if (type == "Event") return CardType::Event;
    if (type == "Structure") return CardType::Structure;
    if (type == "Enchant") return CardType::Enchant;
    return CardType::Resource;
}
}

bool Deck::loadFromJson(const std::string& path) {
    const auto data = FileUtils::getInstance()->getStringFromFile(path);
    if (data.empty()) {
        return false;
    }

    rapidjson::Document document;
    document.Parse(data.c_str());
    if (!document.IsObject() || !document.HasMember("cards")) {
        return false;
    }

    const auto& cards = document["cards"];
    if (!cards.IsArray()) {
        return false;
    }

    cards_.clear();
    for (const auto& cardValue : cards.GetArray()) {
        if (!cardValue.IsObject()) {
            continue;
        }

        CardStats stats;
        if (cardValue.HasMember("stats") && cardValue["stats"].IsObject()) {
            const auto& statsValue = cardValue["stats"];
            if (statsValue.HasMember("cost")) stats.cost = statsValue["cost"].GetInt();
            if (statsValue.HasMember("attack")) stats.attack = statsValue["attack"].GetInt();
            if (statsValue.HasMember("defense")) stats.defense = statsValue["defense"].GetInt();
            if (statsValue.HasMember("durability")) stats.durability = statsValue["durability"].GetInt();
            if (statsValue.HasMember("hunger")) stats.hunger = statsValue["hunger"].GetInt();
            if (statsValue.HasMember("experience")) stats.experience = statsValue["experience"].GetInt();
        }

        cards_.emplace_back(
            cardValue["id"].GetString(),
            cardValue["name"].GetString(),
            toCardType(cardValue["type"].GetString()),
            stats,
            cardValue["description"].GetString(),
            cardValue["art"].GetString());
    }

    return !cards_.empty();
}

void Deck::shuffle() {
    cocos2d::RandomHelper::shuffle(cards_.begin(), cards_.end());
}

Card Deck::draw() {
    if (cards_.empty()) {
        return Card{};
    }
    auto card = cards_.back();
    cards_.pop_back();
    return card;
}

bool Deck::empty() const {
    return cards_.empty();
}

const std::vector<Card>& Deck::getCards() const {
    return cards_;
}

const Card* Deck::findById(const std::string& id) const {
    for (const auto& card : cards_) {
        if (card.getId() == id) {
            return &card;
        }
    }
    return nullptr;
}
