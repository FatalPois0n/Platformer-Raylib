#include "bringerofdeath.hpp"
#include "fighter.hpp"
// #include <algorithm>
#include <cmath>

static AnimDef IDLE  = {0, 0, 8, 10}; 
static AnimDef WALK  = {1, 0, 8, 10};
static AnimDef ATTACK1  = {2, 0, 10, 10}; 
static AnimDef HURT  = {3, 2, 3, 10}; 
static AnimDef DEATH  = {3, 5, 10, 10}; 
static AnimDef CAST  = {4, 7, 9, 10};  
static AnimDef SPELL   = {6, 0, 16, 10};

// Initialize static texture
Texture2D Boss::sharedAtlas = {0};

void Boss::LoadSharedTexture()
{
    if (sharedAtlas.id == 0) {
        sharedAtlas = LoadTexture("resources/enemies/Bringer-Of-Death/SpriteSheet/Bringer-of-Death-SpritSheet.png");
    }
}

void Boss::UnloadSharedTexture()
{
    if (sharedAtlas.id != 0) {
        UnloadTexture(sharedAtlas);
        sharedAtlas = {0};
    }
}

Boss::Boss()
{
    LoadSharedTexture();

    textureWidth  = 140;
    textureHeight = 93;
    offsetX = 0.0f;
    offsetY = 0.0f;

    atlasInfo.frameWidth  = textureWidth;
    atlasInfo.frameHeight = textureHeight;
    atlasInfo.columns     = sharedAtlas.width / textureWidth;

    attack1Anim  = LoadAnim(ATTACK1,  sharedAtlas, atlasInfo, false);
    castAnim  = LoadAnim(CAST,  sharedAtlas, atlasInfo, false);
    spellAnim  = LoadAnim(SPELL,  sharedAtlas, atlasInfo, false);
    hurtAnim = LoadAnim(HURT, sharedAtlas, atlasInfo, false);
    dieAnim  = LoadAnim(DEATH,  sharedAtlas, atlasInfo, false);
    idleAnim = LoadAnim(IDLE, sharedAtlas, atlasInfo, true);
    walkAnim = LoadAnim(WALK, sharedAtlas, atlasInfo, true);

    position = { 1200.0f, 400.0f };
    scale = 5.5f;
    width = (int)(textureWidth * scale);
    height = (int)(textureHeight * scale);
    speed = 2;
    speedY = 0.0f;
    isOnGround = false;
    facingRight = false;

    maxHealth = 500.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();

}

Boss::Boss(Vector2 startPos) : Boss()
{
    position = startPos;
}

Boss::~Boss()
{
    DisposeSpriteAnimation(attack1Anim);
    DisposeSpriteAnimation(castAnim);
    DisposeSpriteAnimation(spellAnim);
    DisposeSpriteAnimation(hurtAnim);
    DisposeSpriteAnimation(dieAnim);
    DisposeSpriteAnimation(idleAnim);
    DisposeSpriteAnimation(walkAnim);
    // Do not unload shared texture - managed by LoadSharedTexture/UnloadSharedTexture
}

Rectangle Boss::GetRect() const
{
    return Rectangle{ position.x, position.y, (float)width, (float)height };
}
Rectangle Boss::GetHitbox() const
{
   // Use full standing height for collision to avoid flicker/jitter when crouching
    float offsetX = 55.0f;
    float offsetY = 75.0f;
    float x = position.x + offsetX;
    float y = position.y + offsetY;
    float w = (float)width - offsetX*2;
    float h = (float)height - offsetY*2;
   
    return Rectangle{x, y, w, h};
}

bool Boss::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle rect = GetHitbox();
    bool grounded = false;

    for (const auto& platform : platforms) {
        Rectangle pr = platform.GetRect();
        if (speedY >= 0 && rect.x + rect.width > pr.x && rect.x < pr.x + pr.width) {
            float bottom = rect.y + rect.height;
            float top = pr.y;
            if (bottom >= top && bottom <= top + pr.height) {
                position.y = top - height;
                speedY = 0.0f;
                grounded = true;
                break;
            }
        }
    }
    return grounded;
}

void Boss::TakeDamage(float damageAmount)
{
    if (isDying || isDeadFinal) return;
    health -= damageAmount;
    if (health <= 0.0f) {
        health = 0.0f;
        isDying = true;
    }
}

void Boss::Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    const float GRAVITY = 800.0f;
    float dt = GetFrameTime();
    playerRectCache = player.GetHitbox();

    if (isDeadFinal) return;

    if (isDying) {
        state = State::Die;
        float elapsed = GetTime() - animationStartTime;
        if (elapsed >= (float)dieAnim.rectanglesCount / dieAnim.framesPerSecond) {
            isDeadFinal = true;
        }
        return;
    }

    // Face player
    float playerCenterX = player.GetHitbox().x + player.GetHitbox().width * 0.5f;
    float myCenterX = position.x + width * 0.5f;
    facingRight = (playerCenterX <= myCenterX);

    // Simple AI: move toward player if not in attack range
    float distanceToPlayer = fabsf(playerCenterX - myCenterX);
    isOnGround = CheckPlatformCollision(platforms);
    if (distanceToPlayer > 200.0f) {
        // Move toward player
        if (!facingRight) {
            position.x += speed;
        } else {
            position.x -= speed;
        }
        SetState(State::Walk);
    } else {
        // In range - attack
        SetState(State::Attack1);
    }
}

void Boss::Draw()
{
    if (isDeadFinal) return;
    float elapsed = GetTime() - animationStartTime;
    Vector2 origin{0,0};
    Rectangle dest = GetHitbox();
    Rectangle dest2 = GetHitbox();
    dest2.x = playerRectCache.x;
    dest2.y = playerRectCache.y - 20.0f; // slightly above player
    

    DrawRectangleLinesEx(dest, 1.0f, RED); // Debug: draw hitbox

    switch (state) {
        case State::Attack1:
            DrawSpriteAnimationPro(attack1Anim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Cast:
            DrawSpriteAnimationPro(castAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            DrawSpriteAnimationPro(spellAnim, dest2, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Hurt:
            DrawSpriteAnimationPro(hurtAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Die:
            DrawSpriteAnimationPro(dieAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Idle:
            DrawSpriteAnimationPro(idleAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Walk:
            DrawSpriteAnimationPro(walkAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
    }

}

void Boss::SetState(State newState)
{
    if (state != newState) {
        state = newState;
        animationStartTime = GetTime();
        lastState = newState;
    }
}
