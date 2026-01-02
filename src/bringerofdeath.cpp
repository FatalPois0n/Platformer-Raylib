#include "bringerofdeath.hpp"
#include "fighter.hpp"
// #include <algorithm>
#include <cmath>

static AnimDef IDLE  = {0, 0, 8, 10}; 
static AnimDef WALK  = {1, 0, 8, 10};
static AnimDef ATTACK1  = {2, 0, 10, 10}; 
static AnimDef HURT  = {3, 2, 3, 10}; 
static AnimDef DEATH  = {3, 5, 10, 1}; 
static AnimDef CAST  = {4, 7, 9, 4};  
static AnimDef SPELL   = {6, 0, 16, 4};

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
    scale = 5.0f;
    width = (int)(textureWidth * scale);
    height = (int)(textureHeight * scale);
    speed = 1.5f;
    speedY = 0.0f;
    isOnGround = false;
    facingRight = false;

    maxHealth = 500.0f;
    health = maxHealth;
    isDying = false;
    isDeadFinal = false;
    castCooldown = 15.0f;

    state = State::Idle;
    lastState = State::Idle;
    animationStartTime = GetTime();
    spellStartPos = {0.0f, 0.0f};
    spellStarted = false;
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
    float offsetXleft = 84.0f * scale;
    float offsetXright = 13.0f * scale;
    float offsetY = 37.0f * scale;
    float x = position.x + offsetXright;
    float y = position.y + offsetY;
    float w = (float)width - offsetXleft - offsetXright;
    float h = (float)height - offsetY;
   
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
    SetState(State::Hurt);
    animationStartTime = GetTime();
    health -= damageAmount;
    if (health <= 0.0f) {
        health = 0.0f;
        isDying = true;
    }
}

void Boss::Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player)
{
    playerRectCache = player.GetHitbox();

    if (isDeadFinal) return;

    if (isDying) {
        state = State::Die;
        float elapsed = GetTime() - animationStartTime;
        if (elapsed >= 10.0f) { // death animation lasts 10 seconds
            isDeadFinal = true;
        }
        return;
    }

    // Face player
    float playerCenterX = player.GetHitbox().x + player.GetHitbox().width * 0.5f;
    float myCenterX = position.x + GetHitbox().width/2;
    facingRight = (playerCenterX <= myCenterX);

    if(state == State::Hurt) {
        float elapsed = GetTime() - animationStartTime;
        if (elapsed >= 0.3f) { // hurt animation duration
            SetState(State::Idle);
        }
        return;
    }
    else if(state == State::Attack1 || state == State::Cast) {
        float animDuration = 0.0f;
        switch(state) {
            case State::Attack1:
                animDuration = (float)attack1Anim.rectanglesCount / (float)attack1Anim.framesPerSecond;
                break;
            case State::Cast:
                animDuration = (float)castAnim.rectanglesCount / (float)castAnim.framesPerSecond;
                // Store spell starting position when cast starts
                if (!spellStarted) {
                    spellStartPos = {player.GetHitbox().x, player.GetHitbox().y};
                    spellStarted = true;
                    castTimer = 0.0f;  // Reset timer when cast begins
                }
                // Update cast timer for hitbox growth
                castTimer += GetFrameTime();
                break;
            default:
                break;
        }
        float elapsed = GetTime() - animationStartTime;
        if (elapsed >= animDuration) {
            SetState(State::Idle);
            spellStarted = false; // Reset for next cast
        }
        return;
    }
    //after 15 seconds the boss will perform a cast attack
    static float castTimer = 0.0f;
    castTimer += GetFrameTime();
    if (castTimer >= castCooldown) {
        SetState(State::Cast);
        castTimer = 0.0f;
        return;
    }

    // Simple AI: move toward player if not in attack range
    float distanceToPlayer = fabsf(playerCenterX - myCenterX);
    isOnGround = CheckPlatformCollision(platforms);
    if (distanceToPlayer > 150.0f) {
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
    
    // Use full sprite rectangle for drawing to prevent visual jumping
    Rectangle dest = GetRect();
    
    // Apply horizontal offset to compensate for asymmetric sprite when flipping
    // Adjust this value based on how off-center the sprite is in the texture
    float flipOffset = 380.0f; // Compensates for character position in texture
    if (facingRight) {
        dest.x -= flipOffset;
    }
    
    // Draw spell at stored position (not following player)
    Rectangle spellDest;
    spellDest.x = spellStartPos.x - 100.0f;  // Match hitbox centering
    spellDest.y = spellStartPos.y - 200.0f;  // Match hitbox offset
    spellDest.width = 200.0f;   // Match hitbox width
    spellDest.height = 300.0f;  // Match hitbox height

    // DrawRectangleLinesEx(GetHitbox(), 1.0f, RED); // Debug: draw hitbox
    // DrawRectangleLinesEx(dest, 1.0f, YELLOW); // Debug: draw texture box
    
    // Draw attack hitboxes during Attack1 and Cast states
    // if (state == State::Attack1) {
    //     Rectangle attack1Hitbox = GetAttack1Hitbox();
    //     DrawRectangleLinesEx(attack1Hitbox, 2.0f, ORANGE); // Debug: attack hitbox
    // } else if (state == State::Cast) {
    //     DrawRectangleLinesEx(spellDest, 2.0f, PURPLE); // Debug: spell hitbox
    // }
    // float myCenterX =position.x + GetHitbox().width/2; // adjust for sprite center
    // DrawCircle(myCenterX, 250, 50.0f, RED); // Debug: draw center point

    switch (state) {
        case State::Attack1:
            DrawSpriteAnimationPro(attack1Anim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            break;
        case State::Cast:
            DrawSpriteAnimationPro(castAnim, dest, origin, 0.0f, WHITE, facingRight, elapsed);
            DrawSpriteAnimationPro(spellAnim, spellDest, origin, 0.0f, WHITE, facingRight, elapsed);
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

Rectangle Boss::GetAttack1Hitbox() const
{
    // Only active during Attack1 state
    if (state != State::Attack1) {
        return Rectangle{0, 0, 0, 0};
    }

    // Add 1 second buffer before hitbox spawns (use elapsed time from animation start)
    float elapsed = GetTime() - animationStartTime;
    if (elapsed < 0.5f) {
        return Rectangle{0, 0, 0, 0};
    }

    Rectangle attack1Hitbox = GetHitbox();

    // Facing logic: when facingRight is true, boss looks left (toward smaller X)
    // Place the hitbox forward in the facing direction with a modest vertical offset
    const float forwardWidth = 400.0f;
    const float forwardOffset = 40.0f;
    const float verticalOffset = -100.0f; // slightly upward so it covers chest/head

    if (facingRight) {
        // Attack to the left side
        attack1Hitbox.x -= (forwardWidth + forwardOffset);
    } else {
        // Attack to the right side
        attack1Hitbox.x += attack1Hitbox.width + forwardOffset;
    }

    attack1Hitbox.y += verticalOffset;
    attack1Hitbox.width = forwardWidth;
    attack1Hitbox.height = attack1Hitbox.height + 200.0f; // extend downward a bit

    return attack1Hitbox;
}

Rectangle Boss::GetCastHitbox() const
{
    // Only return hitbox when in Cast state to prevent lingering
    if (state != State::Cast) {
        return Rectangle{0, 0, 0, 0};
    }
    
    // Start with upper half (150px), grow to full height (300px) over 3 seconds
    float upperHalfHeight = 150.0f;
    float lowerHalfHeight = 150.0f;
    float growthProgress = castTimer / 4.0f;  // 0.0 to 1.0 over 4 seconds
    if (growthProgress > 1.0f) growthProgress = 1.0f;
    
    float currentLowerHeight = lowerHalfHeight * growthProgress;
    float currentTotalHeight = upperHalfHeight + currentLowerHeight;
    
    Rectangle castHitbox;
    castHitbox.x = spellStartPos.x - 100.0f;  // Center larger hitbox
    castHitbox.y = spellStartPos.y - 200.0f;  // Offset upward for taller hitbox
    castHitbox.width = 200.0f;   // Wider to overlap fighter
    castHitbox.height = currentTotalHeight;  // Grows from 150px to 300px
    return castHitbox;
}
