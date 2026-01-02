#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"

class Fighter; // forward declaration

class Boss:public Enemy{
public:
    Boss();
    explicit Boss(Vector2 startPos);
    virtual ~Boss() override;
    // Enemy interface
    void Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player) override;
    void Draw() override;
    Rectangle GetRect() const override;
    Rectangle GetHitbox() const override;
    void TakeDamage(float damageAmount) override;
    bool IsDead() const override { return isDeadFinal; }
    float GetHealth() const override { return health; }
    void LoadSharedTexture();
    void UnloadSharedTexture();
    Rectangle GetAttack1Hitbox() const;
    Rectangle GetCastHitbox() const;

private:
    static Texture2D sharedAtlas;
    AtlasInfo atlasInfo;
    spriteAnimation attack1Anim;
    spriteAnimation castAnim;
    spriteAnimation spellAnim;
    spriteAnimation hurtAnim;
    spriteAnimation dieAnim;
    spriteAnimation idleAnim;
    spriteAnimation walkAnim;

    int textureWidth;
    int textureHeight;
    float offsetX;
    float offsetY;

    Vector2 position;
    float scale;
    int width;
    int height;
    int speed;
    float speedY;
    bool isOnGround;
    bool facingRight;
    Rectangle playerRectCache;
    Vector2 spellStartPos;
    bool spellStarted;


    float maxHealth;
    float health;
    bool isFallingThrough;
    float fallingThroughTimer;
    bool isJumping;
    bool isLanding;
    bool isDying;
    bool isDeadFinal;
    float castCooldown;
    float castTimer;

    enum class State { Idle, Walk, Hurt, Die, Attack1, Cast};
    State state;
    State lastState;
    float animationStartTime;

    void SetState(State newState);
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
};
