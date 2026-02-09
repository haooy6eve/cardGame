#include "GameState.h"
#include "cocos2d.h"
#include <algorithm>
#include <sstream>

using cocos2d::RandomHelper;

GameState::GameState() {
    player_.materials["generic"] = 0;
}

void GameState::startGame(Deck deck) {
    deck_ = std::move(deck);
    deck_.shuffle();
    player_ = PlayerState{};
    player_.materials["generic"] = 0;
    victory_ = false;
    defeat_ = false;
    logs_.clear();
    turn_ = 0;
    log("游戏开始：抽取 5 张起手牌。");
    drawCards(5);
}

void GameState::startTurn() {
    if (victory_ || defeat_) {
        return;
    }
    turn_ += 1;
    std::ostringstream header;
    header << "=== 回合 " << turn_ << " 开始 ===";
    log(header.str());
    turnWeaponAttackBonus_ = 0;
    turnLifeOnKill_ = 0;
    freeToolAvailable_ = (biome_ == BiomeType::Snow);
    applyBiomeAndWeather();
    player_.energy = player_.baseEnergy;
    resolveStructures();
    drawCards(biome_ == BiomeType::End ? 2 : 1);
}

void GameState::endTurn() {
    if (victory_ || defeat_) {
        return;
    }
    resolveMobs();
    player_.hunger = std::max(0, player_.hunger - 1);
    if (player_.hunger == 0) {
        player_.hungerZeroStreak += 1;
        player_.life -= 2;
        log("饥饿归零：受到 2 点真实伤害。");
    } else {
        player_.hungerZeroStreak = 0;
    }

    if (player_.life <= 0 || player_.hungerZeroStreak >= 3) {
        defeat_ = true;
        log("失败条件触发：生命值或饥饿判定归零。");
    }
    log("=== 回合结束 ===");
}

void GameState::playCard(const Card& card) {
    if (victory_ || defeat_) {
        return;
    }
    std::ostringstream message;
    message << "打出卡牌：" << card.getName();
    log(message.str());
    const auto& stats = card.getStats();

    switch (card.getType()) {
        case CardType::Resource:
        case CardType::Block: {
            const int materialGain = std::max(1, stats.cost);
            player_.materials["generic"] += materialGain;
            std::ostringstream logLine;
            logLine << "获得材料点 " << materialGain << "，当前材料点："
                    << player_.materials["generic"];
            log(logLine.str());
            break;
        }
        case CardType::Tool: {
            if (card.getId() == "tool_iron_pickaxe") {
                log("铁镐效果：额外抽 1 张资源牌。");
                drawCards(1);
            }
            break;
        }
        case CardType::Weapon: {
            player_.weapon = card;
            log("装备武器，攻击力提升。");
            break;
        }
        case CardType::Armor: {
            player_.armorCard = card;
            player_.armor = stats.defense;
            log("装备护甲，获得护甲值。");
            break;
        }
        case CardType::Mob: {
            MobState mob{card, std::max(1, stats.defense + 2)};
            player_.mobs.push_back(mob);
            log("怪物加入战场，需要在回合结束前解决。");
            break;
        }
        case CardType::Event: {
            if (card.getId() == "event_creeper_explosion") {
                if (!player_.structures.empty()) {
                    for (auto& structure : player_.structures) {
                        structure.durability = std::max(0, structure.durability - 1);
                    }
                    log("苦力怕爆炸：所有建筑耐久 -1。");
                } else {
                    player_.life -= 2;
                    log("苦力怕爆炸：无建筑保护，生命 -2。");
                }
            }
            if (card.getId() == "event_dragon_arrives") {
                player_.mobs.push_back({card, stats.defense + 5});
                log("Boss 战触发：末影龙登场。");
            }
            break;
        }
        case CardType::Structure: {
            StructureState structure{card, std::max(1, stats.durability)};
            player_.structures.push_back(structure);
            log("建筑落地：提供持续增益。");
            break;
        }
        case CardType::Enchant: {
            if (card.getId() == "enchant_sharpness") {
                turnWeaponAttackBonus_ += 2;
                turnLifeOnKill_ += 1;
                log("附魔生效：本回合武器攻击 +2，击杀回血 +1。");
            }
            break;
        }
    }
}

void GameState::playCardFromHand(size_t index) {
    if (index >= player_.hand.size()) {
        return;
    }
    const Card card = player_.hand[index];
    int cost = card.getStats().cost;
    if (biome_ == BiomeType::Forest &&
        (card.getType() == CardType::Block || card.getType() == CardType::Resource)) {
        cost = std::max(0, cost - 1);
    }
    if (freeToolAvailable_ && card.getType() == CardType::Tool) {
        cost = 0;
        freeToolAvailable_ = false;
        log("雪地效果触发：第一张工具卡免费。");
    }
    if (player_.energy < cost) {
        log("能量不足，无法打出卡牌：" + card.getName());
        return;
    }
    player_.energy -= cost;
    player_.hand.erase(player_.hand.begin() + static_cast<long>(index));
    player_.discard.push_back(card);
    playCard(card);
}

void GameState::playAllAffordableCards() {
    size_t index = 0;
    while (index < player_.hand.size()) {
        const auto before = player_.hand.size();
        playCardFromHand(index);
        if (player_.hand.size() == before) {
            index += 1;
        }
    }
}

bool GameState::craft(const CraftingRecipe& recipe) {
    if (victory_ || defeat_) {
        return false;
    }
    if (player_.materials["generic"] < recipe.materialCost) {
        log("材料不足，合成失败。");
        return false;
    }

    const float successRate = getCraftingSuccessRate(recipe);
    const float roll = RandomHelper::random_real(0.0f, 1.0f);
    std::ostringstream message;
    message << "合成尝试：" << recipe.id << "，成功率 " << successRate;
    log(message.str());
    if (roll > successRate) {
        player_.materials["generic"] -= recipe.materialCost / 2;
        log("合成失败，损失部分材料。");
        return false;
    }

    player_.materials["generic"] -= recipe.materialCost;
    const Card* crafted = deck_.findById(recipe.outputCardId);
    if (crafted) {
        player_.hand.push_back(*crafted);
        log("合成成功，获得卡牌并加入手牌。");
        return true;
    }
    log("合成成功，但未找到对应卡牌。");
    return false;
}

const std::vector<std::string>& GameState::getLogs() const {
    return logs_;
}

std::string GameState::getSummary() const {
    std::ostringstream summary;
    summary << "生命: " << player_.life << " 护甲: " << player_.armor
            << " 能量: " << player_.energy << " 饥饿: " << player_.hunger
            << " 经验: " << player_.experience << " 等级: " << player_.level
            << " 材料: " << player_.materials.at("generic");
    return summary.str();
}

bool GameState::hasVictory() const {
    return victory_;
}

bool GameState::hasDefeat() const {
    return defeat_;
}

void GameState::setBiome(BiomeType biome) {
    biome_ = biome;
}

void GameState::setWeather(WeatherType weather) {
    weather_ = weather;
}

const PlayerState& GameState::getPlayer() const {
    return player_;
}

std::vector<CraftingRecipe> GameState::getDefaultRecipes() const {
    return {
        {"木板打造熔炉", "structure_furnace", 2, 0.8f},
        {"强化铁甲", "armor_iron_chestplate", 3, 0.75f},
        {"钻石锻造", "weapon_diamond_sword", 4, 0.7f}
    };
}

size_t GameState::getHandSize() const {
    return player_.hand.size();
}

void GameState::log(const std::string& message) {
    logs_.push_back(message);
    cocos2d::CCLOG("%s", message.c_str());
}

void GameState::drawCards(int count) {
    for (int i = 0; i < count; ++i) {
        if (deck_.empty()) {
            log("牌库已空。");
            break;
        }
        player_.hand.push_back(deck_.draw());
    }
}

void GameState::applyBiomeAndWeather() {
    switch (biome_) {
        case BiomeType::Forest:
            log("生物群系：森林，木材类卡牌费用 -1。");
            break;
        case BiomeType::Desert:
            log("生物群系：沙漠，怪物攻击 +1，食物效果 -1。");
            break;
        case BiomeType::Snow:
            log("生物群系：雪地，行动阶段第一张工具卡免费。");
            break;
        case BiomeType::Nether:
            log("生物群系：下界，火焰相关卡牌效果 +1。");
            break;
        case BiomeType::End:
            log("生物群系：末地，抽牌阶段额外抽 1 张。");
            break;
    }

    switch (weather_) {
        case WeatherType::Clear:
            log("天气：晴天，无特殊效果。");
            break;
        case WeatherType::Rain:
            log("天气：雨天，水相关卡牌效果 +1。");
            break;
        case WeatherType::Thunderstorm: {
            log("天气：雷暴，回合结束可能触发闪电事件。");
            const bool lightning = RandomHelper::random_int(0, 1) == 1;
            if (lightning) {
                player_.life -= 1;
                log("雷暴闪电击中，生命 -1。");
            }
            break;
        }
    }
}

void GameState::resolveMobs() {
    if (player_.mobs.empty()) {
        return;
    }
    std::vector<MobState> survivors;
    for (auto& mob : player_.mobs) {
        const int playerAttack = getWeaponAttackBonus();
        if (playerAttack >= mob.health) {
            log("击败怪物：" + mob.card.getName());
            gainExperience(1);
            if (turnLifeOnKill_ > 0) {
                player_.life += turnLifeOnKill_;
                log("击杀回血 +" + std::to_string(turnLifeOnKill_));
            }
            if (mob.card.getId() == "event_dragon_arrives") {
                victory_ = true;
                log("击败末影龙，获得胜利！");
                return;
            }
        } else {
            int damage = std::max(0, mob.card.getStats().attack - player_.armor);
            if (biome_ == BiomeType::Desert) {
                damage += 1;
            }
            player_.life -= damage;
            std::ostringstream attackLog;
            attackLog << mob.card.getName() << " 造成伤害 " << damage;
            log(attackLog.str());
            survivors.push_back(mob);
        }
    }
    player_.mobs = std::move(survivors);
}

void GameState::resolveStructures() {
    if (player_.structures.empty()) {
        return;
    }
    for (auto& structure : player_.structures) {
        if (structure.card.getId() == "structure_furnace") {
            if (player_.materials["generic"] > 0) {
                player_.materials["generic"] += 1;
                log("熔炉运转：材料点 +1。");
            }
        }
    }
    player_.structures.erase(
        std::remove_if(player_.structures.begin(), player_.structures.end(),
                       [](const StructureState& structure) { return structure.durability <= 0; }),
        player_.structures.end());
}

void GameState::gainExperience(int amount) {
    player_.experience += amount;
    if (player_.experience >= player_.level * 3) {
        player_.level += 1;
        player_.baseEnergy += 1;
        log("升级！基础能量提升。");
    }
}

float GameState::getCraftingSuccessRate(const CraftingRecipe& recipe) const {
    float rate = recipe.baseSuccessRate;
    rate += static_cast<float>(player_.level) * 0.02f;
    if (biome_ == BiomeType::Forest) {
        rate += 0.05f;
    }
    if (weather_ == WeatherType::Rain) {
        rate += 0.03f;
    }
    return std::min(rate, 0.95f);
}

int GameState::getWeaponAttackBonus() const {
    const int baseAttack = player_.weapon.getStats().attack;
    return baseAttack + turnWeaponAttackBonus_;
}
