#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"

// Forward declaration to avoid circular dependency
class Fighter;

class Slime : public Enemy {
public:
    Slime();
    Slime(Vector2 startPos);
    virtual ~Slime() override;

    // Override Enemy interface
    void Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player) override;
    void Draw() override;
    Rectangle GetRect() const override;
    Rectangle GetHitbox() const override;
    void TakeDamage(float damageAmount) override;
    bool IsDead() const override { return isDeadFinal; }
    float GetHealth() const override { return health; }
    
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
    bool facingLeft;
    int moveDir;           // -1 left, +1 right
    float directionTimer;  // time until we reconsider direction
    float jumpCooldown;    // delay between jumps
    bool isFallingThrough; // falling through platforms
    float fallingThroughTimer; // duration of fall-through
    bool standingOnGroundPlatform; // track if on ground platform

    // Health
    float health;
    float maxHealth;
    bool isDying;
    bool isDeadFinal;
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
