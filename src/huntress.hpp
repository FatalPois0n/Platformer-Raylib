#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"

// class Fighter; // forward declaration

// Spear projectile struct
struct Spear {
    Vector2 position;
    float speedX;           // Horizontal velocity (positive = right, negative = left)
    float width;
    float height;
    bool alive;
    
    Rectangle GetRect() const {
        return Rectangle{ position.x, position.y, width*2.50f, height*2.50f };
    }
};

class Huntress : public Enemy{
public:
    Huntress();
    explicit Huntress(Vector2 startPos);
    virtual ~Huntress() override;

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
    const std::vector<Spear>& GetSpears() const { return spears; }

private:
    static Texture2D sharedAtlas;
    static Texture2D spearAtlas;
    static constexpr float HITBOX_OFFSET_X = 135.0f;
    static constexpr float HITBOX_OFFSET_Y = 133.0f;
    AtlasInfo atlasInfo;
    // spriteAnimation attack1Anim;
    // spriteAnimation attack2Anim;
    spriteAnimation attack3Anim;
    spriteAnimation spearAnim;
    spriteAnimation hurtAnim;
    spriteAnimation dieAnim;
    spriteAnimation jumpAnim;
    spriteAnimation fallAnim;
    spriteAnimation idleAnim;
    spriteAnimation runAnim;

    int textureWidth;
    int textureHeight;

    Vector2 position;
    float scale;
    int width;
    int height;
    int speed;
    float speedY;
    bool isOnGround;
    bool facingRight;


    float maxHealth;
    float health;
    bool isFallingThrough;
    float fallingThroughTimer;
    bool isJumping;
    bool isLanding;
    bool isDying;
    bool isDeadFinal;

    int moveDir;
    float walkingTimer;
    float idleTimer;
    Rectangle standingPlatformRect;
    bool hasStandingPlatform;
    float jumpCooldown;
    float edgeCooldown;
    float directionChangeCooldown;
    int directionChanges;

    enum class State { Idle, Walk, Hurt, Die, Jump, Fall, Attack1, Attack2, Attack3};
    State state;
    State lastState;
    float animationStartTime;
    float attack3StartTime;      // When attack3 state begins
    float attack3Cooldown;       // Cooldown before next attack3 allowed
    bool attack3ProjectileFired; // Track if projectile was fired this attack3
    
    std::vector<Spear> spears;   // Active spears in flight

    void SetState(State newState);
    bool CheckPlatformCollision(const std::vector<Platform>& platforms, Rectangle* platformHit);
    float GetFeetY() const;
    void SpawnSpear();
    void UpdateSpear(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player);
};
