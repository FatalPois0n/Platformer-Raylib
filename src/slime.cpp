#include "slime.hpp"
#include "fighter.hpp"
#include <raylib.h>
#include <cmath>

static AnimDef SLIME_WALK  = {1, 0, 15, 10}; 
static AnimDef SLIME_DIE   = {2, 0, 6, 10};
static AnimDef SLIME_IDLE  = {3, 0, 5, 10};  
static AnimDef SLIME_HURT  = {4, 0, 3, 10}; 

Slime::Slime()
{
    atlas = LoadTexture("resources/enemies/slime/slime_spritesheet.png");

    textureWidth  = 16;
    textureHeight = 16;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = atlas.width / textureWidth;

    walkAnim = LoadAnim(SLIME_WALK, atlas, atlasInfo, true);
    dieAnim  = LoadAnim(SLIME_DIE,  atlas, atlasInfo, false);
    idleAnim = LoadAnim(SLIME_IDLE, atlas, atlasInfo, true);
    hurtAnim = LoadAnim(SLIME_HURT, atlas, atlasInfo, false);

    // Place on ground 
    position = { 600.0f, (float)GetScreenHeight() - 300.0f };

    scale  = 6.5f;
    width  = (int)(textureWidth  * scale);
    height = (int)(textureHeight * scale);

    speed     = 3;   
    speedY    = 0.0f;
    isOnGround = false;
    facingLeft = true;
    moveDir = -1;
    directionTimer = 1.5f;
    jumpCooldown = 0.0f;
    isFallingThrough = false;
    fallingThroughTimer = 0.0f;
    standingOnGroundPlatform = false;

    // Health initialization
    maxHealth = 100.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;
    hurtTimer = 0.0f;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();
}

Slime::Slime(Vector2 startPos)
{
    atlas = LoadTexture("resources/enemies/slime/slime_spritesheet.png");

    textureWidth  = 16;
    textureHeight = 16;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = atlas.width / textureWidth;

    walkAnim = LoadAnim(SLIME_WALK, atlas, atlasInfo, true);
    dieAnim  = LoadAnim(SLIME_DIE,  atlas, atlasInfo, false);
    idleAnim = LoadAnim(SLIME_IDLE, atlas, atlasInfo, true);
    hurtAnim = LoadAnim(SLIME_HURT, atlas, atlasInfo, false);

    // Use provided starting position
    position = startPos;

    scale  = 6.5f;
    width  = (int)(textureWidth  * scale);
    height = (int)(textureHeight * scale);

    speed     = 3;   
    speedY    = 0.0f;
    isOnGround = false;
    facingLeft = true;
    moveDir = -1;
    directionTimer = 1.5f;
    jumpCooldown = 0.0f;
    isFallingThrough = false;
    fallingThroughTimer = 0.0f;
    standingOnGroundPlatform = false;

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

Slime::~Slime()
{
    DisposeSpriteAnimation(idleAnim);
    DisposeSpriteAnimation(walkAnim);
    DisposeSpriteAnimation(hurtAnim);
    DisposeSpriteAnimation(dieAnim);
    UnloadTexture(atlas);
}

void Slime::SetState(State newState)
{
    if (state != newState) {
        state = newState;
        animationStartTime = GetTime();
        lastState = newState;
    }
}

void Slime::TakeDamage(float damageAmount)
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

Rectangle Slime::GetRect() const
{
    return Rectangle{ position.x, position.y, (float)width, (float)height };
}
Rectangle Slime::GetHitbox() const
{
    return Rectangle{ position.x, position.y, (float)width, (float)height };
}

bool Slime::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle rect = GetRect();
    bool grounded = false;
    standingOnGroundPlatform = false;

    for (const auto& platform : platforms) {
        Rectangle platformRect = platform.GetRect();
        
        // Skip collision if we're falling through platforms (but not ground)
        if (isFallingThrough && !platform.IsGround()) {
            continue;
        }

        if (speedY >= 0 &&
            rect.x + rect.width > platformRect.x &&
            rect.x < platformRect.x + platformRect.width) {

            float bottom = rect.y + rect.height;
            float top    = platformRect.y;
            if (bottom >= top && bottom <= top + platformRect.height) {
 
                position.y = top - height;
                speedY = 0.0f;
                grounded = true;
                if (platform.IsGround()) {
                    standingOnGroundPlatform = true;
                }
                break;
            }
        }
    }

    return grounded;
}

void Slime::Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    const float GRAVITY = 800.0f;
    const float JUMP_VELOCITY = -600.0f;
    float dt = GetFrameTime();

    // Timers
    directionTimer -= dt;
    if (directionTimer <= 0.0f) {
        directionTimer = 1.0f + (float)GetRandomValue(0, 200) / 100.0f; // 1.0 - 3.0 seconds
        if (GetRandomValue(0, 1) == 1) moveDir *= -1; // occasionally flip direction
    }
    if (jumpCooldown > 0.0f) jumpCooldown -= dt;
    
    // Update falling through timer
    if (isFallingThrough) {
        fallingThroughTimer -= dt;
        if (fallingThroughTimer <= 0.0f) {
            isFallingThrough = false;
        }
    }

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

    // Wander and occasional jumps (no player chase)
    if (isOnGround) {
        // horizontal wander
        position.x += moveDir * speed;
        facingLeft = (moveDir < 0);
        SetState(State::Walk);

        // Clamp to screen bounds and flip if we hit edges
        int screenW = GetScreenWidth();
        if (position.x < 0.0f) {
            position.x = 0.0f;
            moveDir = 1;
            facingLeft = false;
        } else if (position.x + width > screenW) {
            position.x = (float)screenW - width;
            moveDir = -1;
            facingLeft = true;
        }

        // Chance to jump to a platform right above
        if (jumpCooldown <= 0.0f) {
            Rectangle myRect = GetRect();
            float myCenterX = myRect.x + myRect.width * 0.5f;
            bool jumped = false;
            for (const auto& pf : platforms) {
                Rectangle pfRect = pf.GetRect();
                bool above = pfRect.y + pfRect.height < myRect.y;
                bool alignedX = myCenterX >= pfRect.x && myCenterX <= pfRect.x + pfRect.width;
                float verticalGap = myRect.y - (pfRect.y + pfRect.height);
                if (above && alignedX && verticalGap < 300.0f) { // reasonably above
                    if (GetRandomValue(0, 100) < 25) { // 25% chance when conditions met
                        speedY = JUMP_VELOCITY;
                        isOnGround = false;
                        jumpCooldown = 0.5f; // wait before next jump attempt
                        SetState(State::Idle);
                        jumped = true;
                    }
                    break;
                }
            }
            if (!jumped && jumpCooldown <= 0.0f) {
                jumpCooldown = 0.4f; // small delay before rechecking
            }
        }

        // Edge handling: if next step has no supporting platform, flip or fall
        bool hasSupportNext = false;
        Rectangle nextRect = GetRect();
        nextRect.x += moveDir * speed;
        for (const auto& pf : platforms) {
            Rectangle pfRect = pf.GetRect();
            bool sameHeight = std::fabs((nextRect.y + nextRect.height) - pfRect.y) < 2.5f;
            bool overlapX = nextRect.x + nextRect.width > pfRect.x && nextRect.x < pfRect.x + pfRect.width;
            if (sameHeight && overlapX) { hasSupportNext = true; break; }
        }
        if (!hasSupportNext) {
            // At edge: either turn or fall/drop through
            int choice = GetRandomValue(0, 2); // 0=turn, 1=fall naturally, 2=fall through platform
            if (choice == 0) {
                moveDir *= -1; // turn away
                facingLeft = (moveDir < 0);
            } else if (choice == 2 && !standingOnGroundPlatform && !isFallingThrough) {
                // Initiate fall through current platform
                isFallingThrough = true;
                fallingThroughTimer = 0.5f;
                isOnGround = false;
                speedY = 100.0f; // Push downward
            }
            // else: natural fall (do nothing, gravity handles it)
        }
        
        // Occasionally fall through platform when not at edge (if not on ground)
        if (!standingOnGroundPlatform && !isFallingThrough && GetRandomValue(0, 300) < 1) {
            isFallingThrough = true;
            fallingThroughTimer = 0.5f;
            isOnGround = false;
            speedY = 100.0f;
        }
    } else {
        SetState(State::Idle); // in air
    }

    // gravity
    speedY += GRAVITY * dt;
    position.y += speedY * dt;

    // Platform collisions (only if not falling through)
    if (!isFallingThrough) {
        isOnGround = CheckPlatformCollision(platforms);
    } else {
        isOnGround = false;
    }

    // stick to bottom of screen
    int screenH = GetScreenHeight();
    if (position.y + height >= screenH) {
        position.y = screenH - height;
        speedY = 0.0f;
        isOnGround = true;
        isFallingThrough = false; // Stop falling through when hitting screen bottom
    }
    
}

void Slime::Draw()
{
    float elapsed = GetTime() - animationStartTime;
    Vector2 origin{0,0};
    Rectangle dest = GetRect();

    switch (state) {
        case State::Idle:
            DrawSpriteAnimationPro(idleAnim, dest, origin, 0.0f, WHITE, facingLeft, elapsed);
            break;
        case State::Walk:
            DrawSpriteAnimationPro(walkAnim, dest, origin, 0.0f, WHITE, facingLeft, elapsed);
            break;
        case State::Hurt:
            DrawSpriteAnimationPro(hurtAnim, dest, origin, 0.0f, WHITE, facingLeft, elapsed);
            break;
        case State::Die:
            DrawSpriteAnimationPro(dieAnim, dest, origin, 0.0f, WHITE, facingLeft, elapsed);
            break;
    }
}
