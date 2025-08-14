#include "Item.h"
#include "Player.h"
#include "Game.h"
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Item::Item(ItemType type, int power, int price)
    : type(type), power(power), price(price), itemTexture(nullptr) {
}

// HealingBox Implementation
HealingBox::HealingBox(int healAmount, int cost)
    : Item(ItemType::HEALING_BOX, healAmount, cost) {
}

bool HealingBox::use(Player& player, Game& game) {
    player.heal(power);
    return true; // Item consumed
}

std::string HealingBox::getDescription() const {
    std::stringstream ss;
    ss << "Restores " << power << " health";
    return ss.str();
}

// MassBomb Implementation
MassBomb::MassBomb(int damage, float timer, float radius, int cost)
    : Item(ItemType::MASS_BOMB, damage, cost), timer(timer), radius(radius) {
}

bool MassBomb::use(Player& player, Game& game) {
    Vector2 position = player.getPosition();
    game.addBomb(position, timer, radius, power);
    return true; // Item consumed
}

std::string MassBomb::getDescription() const {
    std::stringstream ss;
    ss << "Deals " << power << " damage\nRadius: " << radius;
    return ss.str();
}
