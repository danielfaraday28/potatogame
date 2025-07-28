#include "ExperienceOrb.h"
#include <cmath>

ExperienceOrb::ExperienceOrb(Vector2 pos, int expValue) 
    : position(pos), radius(8), experienceValue(expValue), alive(true),
      lifetime(0), maxLifetime(30.0f), bobOffset(0), bobSpeed(3.0f) {
}

void ExperienceOrb::update(float deltaTime) {
    lifetime += deltaTime;
    bobOffset += bobSpeed * deltaTime;
    
    // Remove orb after max lifetime
    if (lifetime > maxLifetime) {
        alive = false;
    }
}

void ExperienceOrb::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Calculate bobbing effect
    float bob = sin(bobOffset) * 2.0f;
    Vector2 renderPos = Vector2(position.x, position.y + bob);
    
    // Calculate fade effect based on lifetime
    float fadeRatio = 1.0f;
    if (lifetime > maxLifetime * 0.8f) {
        fadeRatio = 1.0f - ((lifetime - maxLifetime * 0.8f) / (maxLifetime * 0.2f));
    }
    
    // Render experience orb as a glowing circle
    int alpha = (int)(255 * fadeRatio);
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, alpha); // Green glow
    
    int centerX = (int)renderPos.x;
    int centerY = (int)renderPos.y;
    int r = (int)radius;
    
    // Draw outer glow
    for (int x = -r-2; x <= r+2; x++) {
        for (int y = -r-2; y <= r+2; y++) {
            float distance = sqrt(x*x + y*y);
            if (distance <= r + 2 && distance > r) {
                int glowAlpha = (int)(alpha * 0.3f);
                SDL_SetRenderDrawColor(renderer, 100, 255, 100, glowAlpha);
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
    
    // Draw main orb
    SDL_SetRenderDrawColor(renderer, 150, 255, 150, alpha);
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x*x + y*y <= r*r) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
    
    // Draw bright center
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    for (int x = -r/2; x <= r/2; x++) {
        for (int y = -r/2; y <= r/2; y++) {
            if (x*x + y*y <= (r/2)*(r/2)) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
} 