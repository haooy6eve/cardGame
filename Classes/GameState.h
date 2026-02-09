#pragma once

#include "Card.h"
#include "Deck.h"
#include <map>
#include <string>
#include <vector>

enum class BiomeType {
    Forest,
    Desert,
    Snow,
    Nether,
    End
};

enum class WeatherType {
    Clear,
    Rain,
    Thunderstorm
};

struct StructureState {
    Card card;
    int durability = 0;
};

struct MobState {
    Card card;
    int health = 0;
};

struct PlayerState {
    int life = 20;
    int armor = 0;
    int energy = 3;
    int baseEnergy = 3;
    int hunger = 6;
    int experience = 0;
    int level = 1;
    int hungerZeroStreak = 0;
    Card weapon;
    Card armorCard;
    std::vector<Card> hand;
    std::vector<Card> discard;
    std::vector<StructureState> structures;
    std::vector<MobState> mobs;
    std::map<std::string, int> materials;
};

struct CraftingRecipe {
    std::string id;
    std::string outputCardId;
    int materialCost = 0;
    float baseSuccessRate = 0.7f;
};

class GameState {
public:
    GameState();
    void startGame(Deck deck);
    void startTurn();
    void endTurn();
    void playCard(const Card& card);
    void playCardFromHand(size_t index);
    void playAllAffordableCards();
    bool craft(const CraftingRecipe& recipe);
    const std::vector<std::string>& getLogs() const;
    std::string getSummary() const;
    bool hasVictory() const;
    bool hasDefeat() const;
    void setBiome(BiomeType biome);
    void setWeather(WeatherType weather);
    const PlayerState& getPlayer() const;
    std::vector<CraftingRecipe> getDefaultRecipes() const;
    size_t getHandSize() const;

private:
    void log(const std::string& message);
    void drawCards(int count);
    void applyBiomeAndWeather();
    void resolveMobs();
    void resolveStructures();
    void gainExperience(int amount);
    float getCraftingSuccessRate(const CraftingRecipe& recipe) const;
    int getWeaponAttackBonus() const;

    Deck deck_;
    PlayerState player_;
    BiomeType biome_ = BiomeType::Forest;
    WeatherType weather_ = WeatherType::Clear;
    int turn_ = 0;
    bool victory_ = false;
    bool defeat_ = false;
    int turnWeaponAttackBonus_ = 0;
    int turnLifeOnKill_ = 0;
    bool freeToolAvailable_ = false;
    std::vector<std::string> logs_;
};
