#include "Bomb.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>

Bomb::Bomb(Vector2 position, float timer, float radius, int damage)
    : position(position), timer(timer), maxTimer(timer), radius(radius),
      damage(damage), exploded(false), alive(true), flashTimer(0.0f),
      flashInterval(0.2f), showFlash(false), bombTexture(nullptr) {
}

Bomb::~Bomb() {
    if (bombTexture) {
        SDL_DestroyTexture(bombTexture);
        bombTexture = nullptr;
    }
}

void Bomb::update(float deltaTime) {
    if (!alive) return;
    
    if (!exploded) {
        timer -= deltaTime;
        
        // Flash effect speeds up as timer runs down
        flashInterval = std::max(0.1f, (timer / maxTimer) * 0.5f);
        flashTimer += deltaTime;
        if (flashTimer >= flashInterval) {
            showFlash = !showFlash;
            flashTimer = 0.0f;
        }
        
        if (timer <= 0.0f) {
            explode();
        }
    } else {
        // Explosion lasts for a short duration
        timer += deltaTime;
        if (timer >= 0.2f) { // Explosion duration
            alive = false;
        }
    }
}

void Bomb::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    int centerX = static_cast<int>(position.x);
    int centerY = static_cast<int>(position.y);
    
    // Render explosion radius indicator
    if (!exploded) {
        // Calculate alpha based on timer - more visible as timer runs down
        float timerRatio = timer / maxTimer;
        int alpha = static_cast<int>((1.0f - timerRatio) * 100 + 50); // 50-150 alpha range
        
        // Draw damage radius circle (outer ring)
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, alpha);
        renderCircle(renderer, centerX, centerY, static_cast<int>(radius), false);
        
        // Draw a second inner ring for better visibility
        if (radius > 20) {
            SDL_SetRenderDrawColor(renderer, 255, 150, 150, alpha / 2);
            renderCircle(renderer, centerX, centerY, static_cast<int>(radius - 5), false);
        }
        
        // Flash the radius more intensely as timer gets low
        if (timer < 1.0f && showFlash) {
            SDL_SetRenderDrawColor(renderer, 255, 200, 200, 200);
            renderCircle(renderer, centerX, centerY, static_cast<int>(radius), false);
        }
    } else {
        // During explosion - show full bright radius
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 200);
        renderCircle(renderer, centerX, centerY, static_cast<int>(radius), true);
        
        // Inner explosion effect
        SDL_SetRenderDrawColor(renderer, 255, 150, 50, 150);
        renderCircle(renderer, centerX, centerY, static_cast<int>(radius * 0.7f), true);
    }
    
    // Render the bomb itself
    if (!bombTexture) {
        // Fallback rendering - make bomb more visible
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rect = {
            centerX - 10,
            centerY - 10,
            20, 20
        };
        SDL_RenderFillRect(renderer, &rect);
        
        // Add a black border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    } else {
        // Calculate size based on explosion state
        int size = exploded ? static_cast<int>(radius * 2) : 20;
        SDL_Rect destRect = {
            centerX - size/2,
            centerY - size/2,
            size, size
        };
        
        // Flash effect
        if (!exploded && showFlash) {
            SDL_SetTextureColorMod(bombTexture, 255, 255, 255);
        } else {
            SDL_SetTextureColorMod(bombTexture, 200, 50, 50);
        }
        
        SDL_RenderCopy(renderer, bombTexture, nullptr, &destRect);
    }
}

void Bomb::explode() {
    exploded = true;
    timer = 0.0f;
    showFlash = true;
}

void Bomb::loadTexture(SDL_Renderer* renderer) {
    // Load bomb texture - for now using a fallback circle/square shape
    // TODO: Add proper bomb sprite
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    if (surface) {
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
        bombTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
}

void Bomb::renderCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, bool filled) {
    if (filled) {
        // Render filled circle
        for (int x = -radius; x <= radius; x++) {
            for (int y = -radius; y <= radius; y++) {
                if (x*x + y*y <= radius*radius) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
    } else {
        // Render circle outline using Bresenham-like algorithm
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;
        
        while (y >= x) {
            // Draw 8 points for each octant
            SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
            
            x++;
            
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
        }
    }
}