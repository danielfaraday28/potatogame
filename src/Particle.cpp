#include "Particle.h"
#include <random>
#include <algorithm>

Particle::Particle(Vector2 position, Vector2 velocity, float normalDuration, 
                   SDL_Color color, float scale)
    : position(position), velocity(velocity), color(color), scale(scale),
      alive(true), lifetime(0.0f), normalDuration(normalDuration),
      phase(Phase::NORMAL), alpha(255.0f) {
    
    // Add 10% randomization to normal duration
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.9f, 1.1f);
    this->normalDuration = normalDuration * dist(gen);
    
    // Disappear duration is 20% of normal phase
    disappearDuration = this->normalDuration * 0.2f;
    totalDuration = this->normalDuration + disappearDuration;
}

void Particle::update(float deltaTime) {
    if (!alive) return;
    
    // Update position
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    
    // Update lifetime
    lifetime += deltaTime;
    
    // Handle phase transitions and alpha calculation
    if (lifetime >= totalDuration) {
        alive = false;
        return;
    }
    
    if (lifetime >= normalDuration) {
        // Disappearing phase
        phase = Phase::DISAPPEARING;
        
        // Calculate fade-out alpha (linear fade from 255 to 0)
        float disappearProgress = (lifetime - normalDuration) / disappearDuration;
        alpha = 255.0f * (1.0f - disappearProgress);
        alpha = std::max(0.0f, alpha); // Clamp to minimum 0
    } else {
        // Normal phase
        phase = Phase::NORMAL;
        alpha = 255.0f;
    }
}

void Particle::render(SDL_Renderer* renderer, SDL_Texture* starTexture) {
    if (!alive || !starTexture) return;
    
    // Set up texture blending and color modulation
    SDL_SetTextureBlendMode(starTexture, SDL_BLENDMODE_ADD);
    SDL_SetTextureColorMod(starTexture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(starTexture, static_cast<Uint8>(alpha));
    
    // Calculate render position and size
    int texWidth, texHeight;
    SDL_QueryTexture(starTexture, nullptr, nullptr, &texWidth, &texHeight);
    
    int scaledWidth = static_cast<int>(texWidth * scale);
    int scaledHeight = static_cast<int>(texHeight * scale);
    
    SDL_Rect destRect = {
        static_cast<int>(position.x - scaledWidth / 2),
        static_cast<int>(position.y - scaledHeight / 2),
        scaledWidth,
        scaledHeight
    };
    
    // Render the particle
    SDL_RenderCopy(renderer, starTexture, nullptr, &destRect);
    
    // Reset texture modulation to prevent affecting other renders
    SDL_SetTextureColorMod(starTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(starTexture, 255);
    SDL_SetTextureBlendMode(starTexture, SDL_BLENDMODE_BLEND);
}
