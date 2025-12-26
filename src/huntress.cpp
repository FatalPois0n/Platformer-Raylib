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
    speed = 2;
    speedY = 0.0f;
    isOnGround = false;
    facingRight = true;

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
    // Use full standing height for collision to avoid flicker/jitter when crouching
    float offsetX = 135.0f;
    float offsetY = 133.0f;
    float x = position.x + offsetX;
    float y = position.y + offsetY;
    float w = (float)width - offsetX*2;
    float h = (float)height - offsetY*2;
   
    return Rectangle{x, y, w, h};
}

bool Huntress::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle rect = GetHitbox();
    bool grounded = false;

    for (const auto& platform : platforms) {
        Rectangle pr = platform.GetRect();
        if (speedY >= 0 && rect.x + rect.width > pr.x && rect.x < pr.x + pr.width) {
            float bottom = rect.y + rect.height;
            float top = pr.y;
            if (bottom >= top && bottom <= top + pr.height) {
                position.y = top - rect.height;
                speedY = 0.0f;
                grounded = true;
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
    float dt = GetFrameTime();
    Rectangle hitbox = GetHitbox();

    if (isDeadFinal) return;

    if (isDying) {
        // simple death transition
        isDeadFinal = true;
        return;
    }

    // Face player
    float playerCenterX = player.GetHitbox().x + player.GetHitbox().width * 0.5f;
    float myCenterX = position.x + hitbox.width * 0.5f;
    facingRight = (playerCenterX >= myCenterX);

    // Hover on platform without walking (huntress stays put)
    speedY += GRAVITY * dt;
    position.y += speedY * dt;
    isOnGround = CheckPlatformCollision(platforms);
    if (position.y + hitbox.height >= GetScreenHeight()) {
        position.y = (float)GetScreenHeight() - hitbox.height;
        speedY = 0.0f;
        isOnGround = true;
    }

    // Attack3 timing and state machine
    double now = GetTime();
    if (state == State::Attack3) {
        float attack3Elapsed = (float)(now - attack3StartTime);
        
        // Spawn spear 1 second after attack3 starts
        if (attack3Elapsed >= 1.0f && !attack3ProjectileFired) {
            SpawnSpear();
            attack3ProjectileFired = true;
        }
        
        // End attack3 after 1 second, return to idle
        if (attack3Elapsed >= 1.0f) {
            state = State::Idle;
            attack3Cooldown = 2.0f; // 2 second cooldown before next attack3
        }
    }
    
    // Handle attack3 cooldown
    if (attack3Cooldown > 0.0f) {
        attack3Cooldown -= dt;
    }
    
    // Initiate new attack3 when on ground and cooldown expired
    if (isOnGround && state == State::Idle && attack3Cooldown <= 0.0f) {
        state = State::Attack3;
        attack3StartTime = now;
        attack3ProjectileFired = false;
    }

    // Update spear (platform collisions, lifetime)
    UpdateSpear(platforms, walls);
}

void Huntress::Draw()
{
    if (isDeadFinal) return;
    float elapsed = GetTime() - animationStartTime;
    Vector2 origin{0,0};
    Rectangle dest = GetRect();
    Rectangle spearDest = GetRect();
    spearDest.x += facingRight ? dest.width : -20.0f;
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

void Huntress::ResolveHitsOnPlayer(Fighter& player)
{
    if (isDeadFinal) return;
    Rectangle prPlayer = player.GetHitbox();
    for (auto& p : spears) {
        if (!p.alive) continue;
        if (CheckCollisionRecs(p.GetRect(), prPlayer)) {
            // Basic damage: reduce a life and give brief invincibility
            if (player.invincibilityTimer <= 0.0f) {
                player.lives -= 1;
                player.invincibilityTimer = 2.0f;
            }
            p.alive = false;
        }
    }
}
void Huntress::SpawnSpear()
{
    // Spawn spear 20 pixels away from huntress in the facing direction
    Vector2 spearPos = position;
    spearPos.x += facingRight ? width + 20.0f : -20.0f;
    spearPos.y += height * 0.4f; // Vertical center-ish
    
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

void Huntress::UpdateSpear(const std::vector<Platform>& platforms, const std::vector<Wall>& walls)
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