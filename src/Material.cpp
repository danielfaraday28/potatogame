#include "Material.h"
#include <cmath>
#include <random>

Material::Material(Vector2 pos, int matValue, int expValue)
    : position(pos), radius(6), materialValue(matValue), experienceValue(expValue),
      alive(true), lifetime(0), maxLifetime(60.0f), bobOffset(0), bobSpeed(2.0f) {
    
    // Random bob offset so materials don't all bob in sync
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0, 2 * M_PI);
    bobOffset = dist(gen);
}

void Material::update(float deltaTime) {
    if (!alive) return;
    
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        alive = false;
    }
    
    // Bob up and down slightly
    bobOffset += bobSpeed * deltaTime;
}

void Material::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Calculate bobbing position
    float bobY = sin(bobOffset) * 3.0f;
    
    // Render as green blob (materials in Brotato are green)
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    
    int centerX = (int)position.x;
    int centerY = (int)position.y + (int)bobY;
    int r = (int)radius;
    
    // Draw filled circle for material
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x*x + y*y <= r*r) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
    
    // Add a lighter green center
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    r = r / 2;
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x*x + y*y <= r*r) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
    
    // Fade out near end of lifetime
    if (lifetime > maxLifetime * 0.8f) {
        // Visual indicator that material will disappear soon
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
        r = (int)radius + 2;
        for (int angle = 0; angle < 360; angle += 20) {
            float rad = angle * M_PI / 180.0f;
            int x = centerX + cos(rad) * r;
            int y = centerY + sin(rad) * r;
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}