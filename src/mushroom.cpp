#include "mushroom.hpp"
#include "fighter.hpp"
#include <raylib.h>
#include <cmath>

static AnimDef MUSHROOM_WALK  = {0, 0, 8, 10}; 
static AnimDef MUSHROOM_IDLE  = {1, 0, 6, 8};  
static AnimDef MUSHROOM_DIE   = {2, 0, 6, 10};
static AnimDef MUSHROOM_HURT  = {3, 0, 3, 10}; 

Mushroom::Mushroom()
{
    atlas = LoadTexture("resources/enemies/mushroom/mushroom_spritesheet.png");

    textureWidth  = 16;
    textureHeight = 16;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = atlas.width / textureWidth;

    idleAnim = LoadAnim(MUSHROOM_IDLE, atlas, atlasInfo, offsetX, offsetY, true);
    walkAnim = LoadAnim(MUSHROOM_WALK, atlas, atlasInfo, offsetX, offsetY, true);
    hurtAnim = LoadAnim(MUSHROOM_HURT, atlas, atlasInfo, offsetX, offsetY, false);
    dieAnim  = LoadAnim(MUSHROOM_DIE,  atlas, atlasInfo, offsetX, offsetY, false);

    // Place on ground 
    position = { 600.0f, (float)GetScreenHeight() - 300.0f };

    scale  = 5.0f;
    width  = (int)(textureWidth  * scale);
    height = (int)(textureHeight * scale);

    speed     = 3;   
    speedY    = 0.0f;
    isOnGround = false;
    facingRight = true;

    // Health initialization
    maxHealth = 100.0f;
    health = maxHealth;
    isDying = false;
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
    UnloadTexture(atlas);
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
    DrawRectangleLinesEx(rect, 2, BLUE); // Debug: Draw mushroom rectangle
    return rect;
}

bool Mushroom::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle rect = GetRect();
    bool grounded = false;

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
                break;
            }
        }
    }

    return grounded;
}

void Mushroom::Update(const std::vector<Platform>& platforms, const Fighter& player)
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
            //removing enemy
            
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
            if (playerCenterX > myCenterX) {
                position.x += speed;
                facingRight = true;
            } else {
                position.x -= speed;
                facingRight = false;
            }
            SetState(State::Walk);
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
