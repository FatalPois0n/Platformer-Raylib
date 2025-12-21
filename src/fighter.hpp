#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"

// Forward declaration to avoid circular dependency
class Mushroom;

class Fighter {
    public:
    Fighter();
    ~Fighter();
    void Update(const std::vector<Platform>& platforms);
    void Draw(const Mushroom& enemy);
    Rectangle GetRect() const;
    Rectangle GetCollisionRect();
    Rectangle GetAttackHitbox() const;
    void characterDeath(const Mushroom& enemy);
    void PerformSlash(Mushroom& enemy);
    void PerformComboSlash(Mushroom& enemy);
    bool IsAttacking() const { return isAttacking || comboAttack; }
    int lives;
    float invincibilityTimer;
    bool comboAttack;
    
    private:
    Texture2D fighterSet1;
    Texture2D fighterSet2;
    AtlasInfo atlas;
    spriteAnimation idleAnimation;
    spriteAnimation runAnimation;
    spriteAnimation attackAnimation;
    spriteAnimation comboAnimation;
    spriteAnimation jumpAnimation;
    spriteAnimation landAnimation;
    spriteAnimation deathAnimation;
    spriteAnimation crouchAnimation;
    int frameCount;
    Vector2 startingPosition;
    Vector2 position;
    float scale;
    int width;
    int height;
    float offsetX;
    float offsetY;
    float textureWidth;
    float textureHeight;
    int speed;
    float speedY;
    float attackDuration;
    float comboDuration;
    float attackCooldown;
    float nextAttackReadyTime;
    float animationStartTime;
    bool isOnGround;
    bool isFallingThrough;
    float fallingThroughTimer;
    bool standingOnGroundPlatform;
    bool facingRight;
    bool isAttacking;
    bool isRunning;
    bool isJumping;
    bool isLanding;
    bool isCrouching;
    bool isDying;
    float deathTimer;
    
    // Damage system
    float baseDamage;
    float comboDamage;
    bool hasDealtDamage; // Track if damage was dealt this attack to prevent multiple hits
    
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
};