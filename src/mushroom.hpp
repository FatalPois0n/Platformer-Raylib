#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"

// Forward declaration to avoid circular dependency
class Fighter;

class Mushroom {
public:
    Mushroom();
    ~Mushroom();

    // Update AI + physics
    void Update(const std::vector<Platform>& platforms, const Fighter& player);
    // Draw current animation
    void Draw();
    // Render rectangle
    Rectangle GetRect() const;
    // Take damage from player
    void TakeDamage(float damageAmount);
    // Check if dead
    bool IsDead() const { return health <= 0; }
    // Get current health
    float GetHealth() const { return health; }
    
private:
    // Textures & animations
    Texture2D atlas;
    AtlasInfo atlasInfo;
    spriteAnimation idleAnim;
    spriteAnimation walkAnim;
    spriteAnimation hurtAnim;
    spriteAnimation dieAnim;

    // Geometry
    Vector2 position;
    float scale;
    int width;
    int height;
    int textureWidth;
    int textureHeight;
    float offsetX;
    float offsetY;

    // Movement/physics
    int speed;
    float speedY;
    bool isOnGround;
    bool facingRight;

    // Health
    float health;
    float maxHealth;
    bool isDying;
    float hurtTimer;

    // State
    enum class State { Idle, Walk, Hurt, Die };
    State state;
    State lastState;
    float animationStartTime;

    // Internals
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
    void SetState(State newState);
};
