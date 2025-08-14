#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <memory>
#include "Vector2.h"

class Player;
class Game;

enum class ItemType {
    HEALING_BOX,
    MASS_BOMB
};

class Item {
public:
    Item(ItemType type, int power, int price);
    virtual ~Item() = default;
    
    virtual bool use(Player& player, Game& game) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    
    ItemType getType() const { return type; }
    int getPower() const { return power; }
    int getPrice() const { return price; }
    
protected:
    ItemType type;
    int power;    // healing amount or bomb damage
    int price;    // shop cost
    SDL_Texture* itemTexture;
};

class HealingBox : public Item {
public:
    HealingBox(int healAmount, int cost);
    bool use(Player& player, Game& game) override;
    std::string getName() const override { return "HEALING BOX"; }
    std::string getDescription() const override;
};

class MassBomb : public Item {
public:
    MassBomb(int damage, float timer, float radius, int cost);
    bool use(Player& player, Game& game) override;
    std::string getName() const override { return "MASS BOMB"; }
    std::string getDescription() const override;
    
private:
    float timer;
    float radius;
};