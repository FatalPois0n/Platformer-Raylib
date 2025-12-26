#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"
#include "huntress.hpp"

class Fighter {
    public:
    Fighter();
    ~Fighter();
    void Reset();
    void resetPos();
    void Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls);
    void Draw();
    Rectangle GetRect() const;
    Rectangle GetHitbox() const;
    Rectangle GetAttackHitbox() const;
    void characterDeath(const std::vector<Enemy*>& enemies, const std::vector<Spear>& spears);
    void PerformSlash(Enemy& enemy);
    void PerformComboSlash(Enemy& enemy);
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
    bool standingOnWallTop;
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
    float attackHitboxDelay; // Delay before attack hitbox becomes active (1 second)
    
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
    bool CheckWallCollision(const std::vector<Wall>& walls);
};