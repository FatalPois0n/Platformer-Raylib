#include "huntress.hpp"
#include "animation.h"
#include "fighter.hpp"
#include <algorithm>

// static AnimDef ATTACK1  = {0, 0, 5, 10}; 
// static AnimDef ATTACK2  = {1, 0, 5, 10};  
static AnimDef ATTACK3   = {2, 0, 7, 7};
static AnimDef HURT  = {3, 0, 3, 10}; 
static AnimDef DEATH  = {4, 0, 8, 10}; 
static AnimDef JUMP  = {5, 0, 2, 10}; 
static AnimDef FALL  = {6, 0, 2, 10}; 
static AnimDef IDLE  = {7, 0, 8, 10}; 
static AnimDef RUN  = {8, 0, 8, 10};
static AnimDef SPEAR= {0, 0, 4, 10};

// Initialize static textures
Texture2D Huntress::sharedAtlas = {0};
Texture2D Huntress::spearAtlas = {0};

void Huntress::LoadSharedTexture()
{
    if (sharedAtlas.id == 0) {
        sharedAtlas = LoadTexture("resources/enemies/Huntress/Sprites/huntress_spritesheet.png");
    }
}

void Huntress::UnloadSharedTexture()
{
    if (sharedAtlas.id != 0) {
        UnloadTexture(sharedAtlas);
        sharedAtlas = {0};
    }
}

Huntress::Huntress()
{
    LoadSharedTexture();

    textureWidth  = 150;
    textureHeight = 150;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = sharedAtlas.width / textureWidth;

    // attack1Anim  = LoadAnim(ATTACK1,  sharedAtlas, atlasInfo, offsetX, offsetY, false);
    // attack2Anim  = LoadAnim(ATTACK2,  sharedAtlas, atlasInfo, offsetX, offsetY, false);
    attack3Anim  = LoadAnim(ATTACK3,  sharedAtlas, atlasInfo, false);
    hurtAnim = LoadAnim(HURT, sharedAtlas, atlasInfo, false);
    dieAnim  = LoadAnim(DEATH,  sharedAtlas, atlasInfo, false);
    jumpAnim  = LoadAnim(JUMP,  sharedAtlas, atlasInfo, false);
    fallAnim  = LoadAnim(FALL,  sharedAtlas, atlasInfo, false);
    idleAnim = LoadAnim(IDLE, sharedAtlas, atlasInfo, true);
    runAnim = LoadAnim(RUN, sharedAtlas, atlasInfo, true);

    spearAtlas = LoadTexture("resources/enemies/Huntress/Sprites/Spear move.png");
    spearAnim = CreateSpriteAnimation(spearAtlas, 10, (Rectangle[]){
        (Rectangle){0, 0, 60, 20},
        (Rectangle){60, 0, 60, 20},
        (Rectangle){120, 0, 60, 20},
        (Rectangle){180, 0, 60, 20},
    }, 4, true);

    position = { 1200.0f, (float)GetScreenHeight() - 400.0f };
    scale = 2.5f;
    width = (int)(textureWidth * scale);
    height = (int)(textureHeight * scale);
    speed = 3;
    speedY = 0.0f;
    isOnGround = false;
    facingRight = true;
    moveDir = -1;
    walkingTimer = 8.0f;
    idleTimer = 0.0f;
    standingPlatformRect = {0.0f, 0.0f, 0.0f, 0.0f};
    hasStandingPlatform = false;
    jumpCooldown = 0.0f;
    edgeCooldown = 0.0f;
    directionChangeCooldown = 0.0f;
    directionChanges = 0;

    maxHealth = 80.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();
    attack3StartTime = 0.0f;
    attack3Cooldown = 0.0f;
    attack3ProjectileFired = false;

}

Huntress::Huntress(Vector2 startPos) : Huntress()
{
    position = startPos;
}

Huntress::~Huntress()
{
    // DisposeSpriteAnimation(attack1Anim);
    // DisposeSpriteAnimation(attack2Anim);
    DisposeSpriteAnimation(attack3Anim);
    DisposeSpriteAnimation(spearAnim);
    DisposeSpriteAnimation(hurtAnim);
    DisposeSpriteAnimation(dieAnim);
    DisposeSpriteAnimation(jumpAnim);
    DisposeSpriteAnimation(fallAnim);
    DisposeSpriteAnimation(idleAnim);
    DisposeSpriteAnimation(runAnim);
    UnloadTexture(spearAtlas);
    // Do not unload shared texture - managed by LoadSharedTexture/UnloadSharedTexture
}

Rectangle Huntress::GetRect() const
{
    return Rectangle{ position.x, position.y, (float)width, (float)height };
}

Rectangle Huntress::GetHitbox() const
{
    float x = position.x + HITBOX_OFFSET_X;
    float y = position.y + HITBOX_OFFSET_Y;
    float w = (float)width - HITBOX_OFFSET_X * 2.0f;
    float h = (float)height - HITBOX_OFFSET_Y * 2.0f;
    return Rectangle{x, y, w, h};
}

float Huntress::GetFeetY() const
{
    Rectangle hit = GetHitbox();
    return hit.y + hit.height;
}

bool Huntress::CheckPlatformCollision(const std::vector<Platform>& platforms, Rectangle* platformHit)
{
    Rectangle rect = GetHitbox();
    bool grounded = false;
    if (platformHit) {
        *platformHit = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    for (const auto& platform : platforms) {
        Rectangle pr = platform.GetRect();
        if (speedY >= 0 && rect.x + rect.width > pr.x && rect.x < pr.x + pr.width) {
            float feetY = rect.y + rect.height;
            float platformTop = pr.y;
            if (feetY >= platformTop && feetY <= platformTop + pr.height) {
                position.y = platformTop - (HITBOX_OFFSET_Y + rect.height);
                speedY = 0.0f;
                grounded = true;
                if (platformHit) {
                    *platformHit = pr;
                }
                break;
            }
        }
    }
    return grounded;
}

void Huntress::TakeDamage(float damageAmount)
{
    if (isDying || isDeadFinal) return;
    health -= damageAmount;
    if (health <= 0.0f) {
        health = 0.0f;
        isDying = true;
        state = State::Die;
        animationStartTime = GetTime();
    }
}

void Huntress::Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    const float GRAVITY = 800.0f;
    const float JUMP_VELOCITY = -700.0f;
    float dt = GetFrameTime();
    double now = GetTime();

    if (isDeadFinal) return;

    if (isDying) {
        // simple death transition
        isDeadFinal = true;
        return;
    }

    // Vertical movement and ground snap
    speedY += GRAVITY * dt;
    position.y += speedY * dt;

    Rectangle groundRect{};
    isOnGround = CheckPlatformCollision(platforms, &groundRect);
    if (isOnGround) {
        standingPlatformRect = groundRect;
        hasStandingPlatform = true;
        speedY = 0.0f;
    } else {
        hasStandingPlatform = false;
    }

    if (isOnGround && (state == State::Jump || state == State::Fall)) {
        SetState(State::Idle);
    }

    Rectangle hitbox = GetHitbox();
    float feetHeight = HITBOX_OFFSET_Y + hitbox.height;
    float feetY = GetFeetY();
    int screenH = GetScreenHeight();
    int screenW = GetScreenWidth();
    if (feetY >= (float)screenH) {
        position.y = (float)screenH - feetHeight;
        speedY = 0.0f;
        isOnGround = true;
        hasStandingPlatform = true;
        standingPlatformRect = {0.0f, (float)screenH - 2.0f, (float)screenW, 4.0f};
        hitbox = GetHitbox();
    }

    float playerCenterX = player.GetHitbox().x + player.GetHitbox().width * 0.5f;
    float myCenterX = hitbox.x + hitbox.width * 0.5f;

    // Attack3 timing and state machine
    if (state == State::Attack3) {
        float attack3Elapsed = (float)(now - attack3StartTime);
        
        // Spawn spear 1 second after attack3 starts
        if (attack3Elapsed >= 1.0f && !attack3ProjectileFired) {
            facingRight = (playerCenterX >= myCenterX);
            SpawnSpear();
            attack3ProjectileFired = true;
        }
        
        // End attack3 after 1 second, return to idle
        if (attack3Elapsed >= 1.0f) {
            state = State::Idle;
            attack3Cooldown = 5.0f; // 5 second cooldown before next attack3
        }
    }
    
    // Handle attack3 cooldown
    if (attack3Cooldown > 0.0f) {
        attack3Cooldown -= dt;
    }

    // Movement and patrol AI (skip while attacking or hurt)
    if (state != State::Attack3 && state != State::Hurt) {
        if (jumpCooldown > 0.0f) {
            jumpCooldown -= dt;
        }
        if (edgeCooldown > 0.0f) {
            edgeCooldown -= dt;
        }
        if (directionChangeCooldown > 0.0f) {
            directionChangeCooldown -= dt;
        }
        if (walkingTimer > 0.0f) {
            walkingTimer -= dt;
        }
        if (idleTimer > 0.0f) {
            idleTimer -= dt;
        }

        hitbox = GetHitbox();

        if (isOnGround) {
            bool jumped = false;
            if (jumpCooldown <= 0.0f) {
                float centerX = hitbox.x + hitbox.width * 0.5f;
                for (const auto& pf : platforms) {
                    Rectangle pfRect = pf.GetRect();
                    bool above = pfRect.y + pfRect.height < hitbox.y;
                    bool alignedX = centerX >= pfRect.x && centerX <= pfRect.x + pfRect.width;
                    float verticalGap = hitbox.y - (pfRect.y + pfRect.height);
                    if (above && alignedX && verticalGap < 320.0f) {
                        if (GetRandomValue(0, 0.5*60*100) < 25) { // 25% chance to jump per 0.5 second
                            speedY = JUMP_VELOCITY;
                            isOnGround = false;
                            hasStandingPlatform = false;
                            SetState(State::Jump);
                            jumpCooldown = 1.2f;
                            jumped = true;
                        }
                        break;
                    }
                }
                if (!jumped && jumpCooldown <= 0.0f) {
                    jumpCooldown = 0.3f;
                }
            }

            if (!jumped) {
                float step = moveDir * (float)speed;
                Rectangle nextHitbox = hitbox;
                nextHitbox.x += step;

                // Check screen edges
                int screenW = GetScreenWidth();
                bool atScreenEdge = (nextHitbox.x <= 0.0f) || (nextHitbox.x + nextHitbox.width >= (float)screenW);
                
                if (atScreenEdge) {
                    // Flip direction at screen edge
                    moveDir *= -1;
                    directionChangeCooldown = 5.0f;
                    directionChanges++;
                    step = moveDir * (float)speed;
                    position.x += step * 2.0f; // Move further away from edge
                    SetState(State::Walk);
                } else {
                    // Determine current state and action
                    if (idleTimer > 0.0f) {
                        // Currently idling - don't move
                        SetState(State::Idle);
                    } else if (walkingTimer > 0.0f) {
                        // Walking - move forward
                        position.x += step;
                        SetState(State::Walk);
                    } else if (walkingTimer <= 0.0f && idleTimer <= 0.0f && state == State::Walk) {
                        // Just transitioned from walking to idle
                        idleTimer = 3.0f;
                        SetState(State::Idle);
                    } else if (walkingTimer <= 0.0f && idleTimer <= 0.0f && state == State::Idle) {
                        // Just finished idling - resume walking
                        walkingTimer = 5.0f;
                        if (GetRandomValue(0, 1) == 0) { // 50% chance to change direction
                            moveDir *= -1;
                            directionChangeCooldown = 5.0f;
                            directionChanges++;
                        }
                        position.x += step;
                        SetState(State::Walk);
                    }
                }
            }
            
            facingRight = moveDir > 0;
        } else {
            if (speedY > 0.0f) {
                SetState(State::Fall);
            }
        }

        hitbox = GetHitbox();
        myCenterX = hitbox.x + hitbox.width * 0.5f;

        if (isOnGround && attack3Cooldown <= 0.0f) {
            facingRight = (playerCenterX >= myCenterX);
            SetState(State::Attack3);
            attack3StartTime = now;
            attack3ProjectileFired = false;
            if (directionChangeCooldown < 1.5f) {
                directionChangeCooldown = 1.5f; // lock direction changes for 1s after attack start
            }
        }


    }

    // Update spear (platform collisions, lifetime)
    UpdateSpear(platforms, walls, player);
}

void Huntress::Draw()
{
    if (isDeadFinal) return;
    float elapsed = GetTime() - animationStartTime;
    Vector2 origin{0,0};
    Rectangle dest = GetRect();
    Rectangle spearDest = GetRect();
    spearDest.x += facingRight ? GetHitbox().width : -20.0f;
    spearDest.y += dest.height * 0.4f;

    DrawRectangleLinesEx(GetHitbox(), 1.0f, RED); // Debug: draw hitbox

    switch (state) {
        // case State::Attack1:
        //     DrawSpriteAnimationPro(attack1Anim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
        //     break;
        // case State::Attack2:
        //     DrawSpriteAnimationPro(attack2Anim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
        //     break;
        case State::Attack3:
            DrawSpriteAnimationPro(attack3Anim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            // DrawSpriteAnimationPro(spearAnim, spearDest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Hurt:
            DrawSpriteAnimationPro(hurtAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Die:
            DrawSpriteAnimationPro(dieAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Jump:
            DrawSpriteAnimationPro(jumpAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Fall:
            DrawSpriteAnimationPro(fallAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Idle:
            DrawSpriteAnimationPro(idleAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Walk:
            DrawSpriteAnimationPro(runAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
    }

    // Draw active spears
    for (const auto& spear : spears) {
        if (spear.alive) {
            DrawSpriteAnimationPro(spearAnim, spear.GetRect(), origin, 0.0f, WHITE, spear.speedX > 0.0f, elapsed);
            DrawRectangleLinesEx(spear.GetRect(), 2.0f, YELLOW); // Debug: draw spear hitbox
        }
    }

}

void Huntress::SetState(State newState)
{
    if (state != newState) {
        state = newState;
        animationStartTime = GetTime();
        lastState = newState;
    }
}

void Huntress::SpawnSpear()
{
    // Spawn spear from huntress center
    Rectangle spriteRect = GetRect();
    float centerX = spriteRect.x + spriteRect.width * 0.5f;
    float centerY = spriteRect.y + spriteRect.height * 0.40f; // Upper-mid body
    
    Vector2 spearPos;
    spearPos.x = facingRight ? centerX + 40.0f : centerX - 40.0f;
    spearPos.y = centerY;
    
    float spearSpeed = 300.0f; // pixels per second
    float speedX = facingRight ? spearSpeed : -spearSpeed;
    
    Spear newSpear;
    newSpear.position = spearPos;
    newSpear.speedX = speedX;
    newSpear.width = 60.0f;   // Spear width
    newSpear.height = 20.0f;  // Spear height
    newSpear.alive = true;
    
    spears.push_back(newSpear);
}

void Huntress::UpdateSpear(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    float dt = GetFrameTime();
    
    // Update each active spear
    for (auto it = spears.begin(); it != spears.end(); ) {
        if (!it->alive) {
            it = spears.erase(it);
            continue;
        }
        
        // Move spear horizontally
        it->position.x += it->speedX * dt;
        
        // Check collision with platforms
        bool collided = false;
        Rectangle spearRect = it->GetRect();
        
        for (const auto& platform : platforms) {
            Rectangle platformRect = platform.GetRect();
            if (CheckCollisionRecs(spearRect, platformRect)) {
                collided = true;
                break;
            }
        }
        
        // Check collision with walls
        if (!collided) {
            Rectangle playerRect = player.GetRect();
            if (CheckCollisionRecs(spearRect, playerRect)) {
                collided = true;
                break;
            }
            for (const auto& wall : walls) {
                Rectangle wallRect = wall.GetRect();
                if (CheckCollisionRecs(spearRect, wallRect)) {
                    collided = true;
                    break;
                }
            }
        }
        
        // Remove spear if it hit something or left screen
        if (collided || it->position.x < 0 || it->position.x > GetScreenWidth()) {
            it->alive = false;
            it = spears.erase(it);
        } else {
            ++it;
        }
    }
}