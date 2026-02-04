#include "Card.h"

Card::Card(std::string id,
           std::string name,
           CardType type,
           CardStats stats,
           std::string description,
           std::string art)
    : id_(std::move(id)),
      name_(std::move(name)),
      type_(type),
      stats_(stats),
      description_(std::move(description)),
      art_(std::move(art)) {}

const std::string& Card::getId() const {
    return id_;
}

const std::string& Card::getName() const {
    return name_;
}

CardType Card::getType() const {
    return type_;
}

const CardStats& Card::getStats() const {
    return stats_;
}

const std::string& Card::getDescription() const {
    return description_;
}

const std::string& Card::getArt() const {
    return art_;
}
