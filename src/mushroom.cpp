#include "mushroom.hpp"
#include "fighter.hpp"
#include <raylib.h>
#include <cmath>
#include <algorithm>

// clamp function for C++14 compatibility
template<typename T>
T clamp(T value, T min_val, T max_val) {
    return std::max(min_val, std::min(value, max_val));
}

static AnimDef MUSHROOM_WALK  = {0, 0, 8, 10}; 
static AnimDef MUSHROOM_IDLE  = {1, 0, 6, 8};  
static AnimDef MUSHROOM_DIE   = {2, 0, 6, 10};
static AnimDef MUSHROOM_HURT  = {3, 0, 3, 10}; 

// Initialize static texture
Texture2D Mushroom::sharedAtlas = {0};

void Mushroom::LoadSharedTexture()
{
    if (sharedAtlas.id == 0) {
        sharedAtlas = LoadTexture("resources/enemies/mushroom/mushroom_spritesheet.png");
    }
}

void Mushroom::UnloadSharedTexture()
{
    if (sharedAtlas.id != 0) {
        UnloadTexture(sharedAtlas);
        sharedAtlas = {0};
    }
}

Mushroom::Mushroom()
{
    LoadSharedTexture();

    textureWidth  = 16;
    textureHeight = 16;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = sharedAtlas.width / textureWidth;

    idleAnim = LoadAnim(MUSHROOM_IDLE, sharedAtlas, atlasInfo, true);
    walkAnim = LoadAnim(MUSHROOM_WALK, sharedAtlas, atlasInfo, true);
    hurtAnim = LoadAnim(MUSHROOM_HURT, sharedAtlas, atlasInfo, false);
    dieAnim  = LoadAnim(MUSHROOM_DIE,  sharedAtlas, atlasInfo, false);

    // Place on ground 
    position = { 600.0f, (float)GetScreenHeight() - 300.0f };

    scale  = 5.0f;
    width  = (int)(textureWidth  * scale);
    height = (int)(textureHeight * scale);

    speed     = 2.5;   
    speedY    = 0.0f;
    isOnGround = false;
    hasPlatformSupport = false;
    currentPlatformRect = {0, 0, 0, 0};
    facingRight = true;

    // Health initialization
    maxHealth = 50.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;
    hurtTimer = 0.0f;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();
}

Mushroom::Mushroom(Vector2 startPos)
{
    LoadSharedTexture();

    textureWidth  = 16;
    textureHeight = 16;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = sharedAtlas.width / textureWidth;

    idleAnim = LoadAnim(MUSHROOM_IDLE, sharedAtlas, atlasInfo, true);
    walkAnim = LoadAnim(MUSHROOM_WALK, sharedAtlas, atlasInfo, true);
    hurtAnim = LoadAnim(MUSHROOM_HURT, sharedAtlas, atlasInfo, false);
    dieAnim  = LoadAnim(MUSHROOM_DIE,  sharedAtlas, atlasInfo, false);

    // Use provided starting position
    position = startPos;

    scale  = 5.0f;
    width  = (int)(textureWidth  * scale);
    height = (int)(textureHeight * scale);

    speed     = 2.5;   
    speedY    = 0.0f;
    isOnGround = false;
    hasPlatformSupport = false;
    currentPlatformRect = {0, 0, 0, 0};
    facingRight = true;

    // Health initialization
    maxHealth = 50.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;
    hurtTimer = 0.0f;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();
}

Mushroom::~Mushroom()
{
    DisposeSpriteAnimation(idleAnim);
    DisposeSpriteAnimation(walkAnim);
    DisposeSpriteAnimation(hurtAnim);
    DisposeSpriteAnimation(dieAnim);
    // Do not unload shared texture - managed by LoadSharedTexture/UnloadSharedTexture
}

void Mushroom::SetState(State newState)
{
    if (state != newState) {
        state = newState;
        animationStartTime = GetTime();
        lastState = newState;
    }
}

void Mushroom::TakeDamage(float damageAmount)
{
    if (isDying) return; // Already dying, ignore damage
    
    health -= damageAmount;
    
    if (health <= 0) {
        health = 0;
        isDying = true;
        SetState(State::Die);
    } else {
        SetState(State::Hurt);
        hurtTimer = 0.4f; // Hurt animation duration
    }
}

Rectangle Mushroom::GetRect() const
{
    Rectangle rect = { position.x, position.y, (float)width, (float)height };
    return rect;
}

Rectangle Mushroom::GetHitbox() const
{
    Rectangle rect = { position.x, position.y, (float)width, (float)height };
    return rect;
}

bool Mushroom::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle rect = GetRect();
    bool grounded = false;
    hasPlatformSupport = false;

    for (const auto& platform : platforms) {
        Rectangle platformRect = platform.GetRect();

        if (speedY >= 0 &&
            rect.x + rect.width > platformRect.x &&
            rect.x < platformRect.x + platformRect.width) {

            float bottom = rect.y + rect.height;
            float top    = platformRect.y;
            if (bottom >= top && bottom <= top + platformRect.height) {
 
                position.y = top - height;
                speedY = 0.0f;
                grounded = true;
                hasPlatformSupport = true;
                currentPlatformRect = platformRect;
                break;
            }
        }
    }

    return grounded;
}

void Mushroom::Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    const float GRAVITY = 800.0f;
    float dt = GetFrameTime();

    // Update hurt timer
    if (hurtTimer > 0.0f) {
        hurtTimer -= dt;
        if (hurtTimer <= 0.0f && !isDying) {
            SetState(State::Idle);
        }
    }

    // If dying, skip AI and only handle animation
    if (isDying) {
        float dieAnimDuration = (float)dieAnim.rectanglesCount / (float)dieAnim.framesPerSecond;
        if (GetTime() - animationStartTime >= dieAnimDuration) {
            // mark fully dead after death animation finishes
            isDeadFinal = true;
        }
        return;
    }

    // Skip movement during hurt animation
    if (state == State::Hurt) {
        // Apply gravity and platform collision even when hurt
        speedY += GRAVITY * dt;
        position.y += speedY * dt;
        isOnGround = CheckPlatformCollision(platforms);
        
        int screenH = GetScreenHeight();
        if (position.y + height >= screenH) {
            position.y = screenH - height;
            speedY = 0.0f;
            isOnGround = true;
        }
        return;
    }

    // Simple chase AI (no jumping). move toward fighter horizontally
    float playerCenterX = player.GetRect().x + player.GetRect().width * 0.5f;
    float myCenterX     = position.x + width * 0.5f;

    if (std::fabs(playerCenterX - myCenterX) > 4.0f) {
        // Chase only when on ground, otherwise fall
        if (isOnGround) {
            float dir = (playerCenterX > myCenterX) ? 1.0f : -1.0f;
            float nextX = position.x + dir * speed;

            if (hasPlatformSupport) {
                float minX = currentPlatformRect.x;
                float maxX = currentPlatformRect.x + currentPlatformRect.width - width;
                float clampedX = clamp(nextX, minX, maxX);
                if (std::fabs(clampedX - position.x) < 0.001f) {
                    SetState(State::Idle);
                } else {
                    position.x = clampedX;
                    facingRight = (dir > 0.0f);
                    SetState(State::Walk);
                }
            } else {
                // Fallback when no platform data is available yet
                position.x = nextX;
                facingRight = (dir > 0.0f);
                SetState(State::Walk);
            }
        } else {
            //keep motion minimal when in air
            SetState(State::Idle);
        }
    } else {
        SetState(State::Idle);
    }

    // gravity
    speedY += GRAVITY * dt;
    position.y += speedY * dt;

    // Platform collisions
    isOnGround = CheckPlatformCollision(platforms);

    // Keep mushroom on top of its supporting platform
    if (isOnGround && hasPlatformSupport) {
        float minX = currentPlatformRect.x;
        float maxX = currentPlatformRect.x + currentPlatformRect.width - width;
        position.x = clamp(position.x, minX, maxX);
    }

    // stick to bottom of screen
    int screenH = GetScreenHeight();
    if (position.y + height >= screenH) {
        position.y = screenH - height;
        speedY = 0.0f;
        isOnGround = true;
    }
}

void Mushroom::Draw()
{
    float elapsed = GetTime() - animationStartTime;
    Vector2 origin{0,0};
    Rectangle dest = GetRect();

    switch (state) {
        case State::Idle:
            DrawSpriteAnimationPro(idleAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Walk:
            DrawSpriteAnimationPro(walkAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Hurt:
            DrawSpriteAnimationPro(hurtAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Die:
            DrawSpriteAnimationPro(dieAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
    }
}
