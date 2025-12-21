#include "fighter.hpp"
#include "animation.h"
#include "mushroom.hpp" 
#include <raylib.h>

Fighter::Fighter()
{

    fighterSet1 = LoadTexture("resources/char_red_1.png");
    fighterSet2 = LoadTexture("resources/char_red_2.png");
    frameCount = 6;
    offsetX = 15;
    offsetY = 20;
    textureWidth = 56;
    textureHeight = 56;
    atlas.frameWidth  = textureWidth;
    atlas.frameHeight = textureHeight;
    atlas.columns     = fighterSet1.width / textureWidth;
    idleAnimation  = LoadAnim(idle, fighterSet1, atlas, offsetX, offsetY, true);
    runAnimation  = LoadAnim(run, fighterSet1, atlas, offsetX, offsetY, true);
    jumpAnimation = LoadAnim(jump, fighterSet1, atlas, offsetX, offsetY, false);
    landAnimation = LoadAnim(land, fighterSet1, atlas, offsetX, offsetY, false);
    deathAnimation = LoadAnim(death, fighterSet1, atlas, offsetX, offsetY, false);
    crouchAnimation = LoadAnim(crouch, fighterSet1, atlas, offsetX, offsetY, false);
    attackAnimation = LoadAnim(attack, fighterSet1, atlas, offsetX, offsetY, false);
    comboAnimation = LoadAnim(combo, fighterSet1, atlas, offsetX, offsetY, false);

    attackDuration = (float)attackAnimation.rectanglesCount / (float)attackAnimation.framesPerSecond;
    comboDuration  = (float)comboAnimation.rectanglesCount  / (float)comboAnimation.framesPerSecond;
    attackCooldown = 0.45f;
    nextAttackReadyTime = 0.0f;

    startingPosition = { 50.0f, 1080.0f - 80 - 200.0f }; // Start on ground
    position = startingPosition; // manipulatable position vector
    scale = 2.5;
    width = (textureWidth * scale - 2 * offsetX);
    height = (textureHeight * scale - offsetY);
    speed = 5;
    speedY = 0.0f;
    animationStartTime = 0.0f;
    isOnGround = true;
    isFallingThrough = false;
    fallingThroughTimer = 0.0f;
    facingRight = true;
    isAttacking = false;
    comboAttack = false;
    isRunning = false;
    isJumping = false;
    isLanding = false;
    isCrouching = false;
    isDying = false;
    deathTimer = 0.0f;
    lives = 4;
    invincibilityTimer = 0.0f;
    
    // Damage system
    baseDamage = 25.0f; // Normal attack damage
    comboDamage = 40.0f; // Combo attack damage (more powerful)
    hasDealtDamage = false;
}

Fighter::~Fighter()
{
    DisposeSpriteAnimation(idleAnimation);
    DisposeSpriteAnimation(runAnimation);
    DisposeSpriteAnimation(jumpAnimation);
    DisposeSpriteAnimation(crouchAnimation);
    DisposeSpriteAnimation(landAnimation);
    DisposeSpriteAnimation(deathAnimation);
    DisposeSpriteAnimation(attackAnimation);
    DisposeSpriteAnimation(comboAnimation);
}

Rectangle Fighter::GetAttackHitbox() const
{
    // Attack hitbox extends in front of the player
    float hitboxWidth = width * 1.2f; // Slightly larger than player
    float hitboxHeight = height * 0.8f; // Most of player height
    float hitboxX = facingRight ? position.x + width * 0.5f : position.x - hitboxWidth + width * 0.5f;
    float hitboxY = position.y + height * 0.1f;
    
    return Rectangle{hitboxX, hitboxY, hitboxWidth, hitboxHeight};
}

void Fighter::PerformSlash(Mushroom& enemy)
{
    if (hasDealtDamage || !isAttacking) return; // Already dealt damage this attack or not attacking
    
    Rectangle attackBox = GetAttackHitbox();
    Rectangle enemyBox = enemy.GetRect();
    
    if (CheckCollisionRecs(attackBox, enemyBox) && !enemy.IsDead()) {
        enemy.TakeDamage(baseDamage);
        hasDealtDamage = true; // Mark that we've dealt damage this attack
    }
}

void Fighter::PerformComboSlash(Mushroom& enemy)
{
    if (hasDealtDamage || !comboAttack) return; // Already dealt damage this attack or not combo attacking
    
    Rectangle attackBox = GetAttackHitbox();
    Rectangle enemyBox = enemy.GetRect();
    
    if (CheckCollisionRecs(attackBox, enemyBox) && !enemy.IsDead()) {
        enemy.TakeDamage(comboDamage);
        hasDealtDamage = true; // Mark that we've dealt damage this attack
    }
}

void Fighter::Draw(const Mushroom& enemy)
{
    Rectangle a = GetRect();
    Rectangle b = enemy.GetRect();
    (CheckCollisionRecs(a,b)) ? DrawRectangleLinesEx(GetRect(), 2, RED) : DrawRectangleLinesEx(GetRect(), 2, GREEN); // Debug: Draw fighter rectangle
    
    // Debug: Draw attack hitbox when attacking
    if (isAttacking || comboAttack) {
        DrawRectangleLinesEx(GetAttackHitbox(), 2, YELLOW);
    }
    
    float elapsedTime = GetTime() - animationStartTime;
    
    if(comboAttack){
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(comboAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isAttacking){
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(attackAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isDying)
    {
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(deathAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isRunning){
        // Rectangle dest = {position.x, position.y, (float)width, (float)height};
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(runAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isJumping)
    {
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(jumpAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isLanding)
    {
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(landAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else if(isCrouching)
    {
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(crouchAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }
    else{
        Vector2 origin = {0,0};
        DrawSpriteAnimationPro(idleAnimation, GetRect(), origin, 0, WHITE, facingRight, elapsedTime);
    }

}

Rectangle Fighter::GetRect() const
{
    if (isCrouching) {
        // height is slightly reduced when crouching (90% of normal height)
        float crouchHeight = height * 0.9f;
        float crouchOffsetY = height - crouchHeight; // keep feet planted
        return Rectangle{position.x, position.y + crouchOffsetY, (float)width, crouchHeight};
    }
    // if (isJumping) {
    //     // height is increased when jumping (120% of normal height)
    //     float jumpHeight = height * 1.2f;
    //     float jumpWidth = width * 1.2f;
    //     float jumpOffsetY = height - jumpHeight; // keep feet planted
    //     return Rectangle{position.x, position.y + jumpOffsetY, (float)jumpWidth, jumpHeight};
    // }
    return Rectangle{position.x, position.y, (float)width, (float)height};
}

Rectangle Fighter::GetCollisionRect()
{
    // Use full standing height for collision to avoid flicker/jitter when crouching
    return Rectangle{position.x, position.y, (float)width, (float)height};
}

bool Fighter::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle fighterRect = GetCollisionRect();
    standingOnGroundPlatform = false;
    
    for (const auto& platform : platforms) {
        Rectangle platformRect = platform.GetRect();
        
        // Skip collision if we're falling through platforms (but not ground)
        if (isFallingThrough && !platform.IsGround()) {
            continue;
        }
        
        // Check if fighter is falling and overlapping with platform
        if (speedY >= 0 && 
            fighterRect.x + fighterRect.width > platformRect.x &&
            fighterRect.x < platformRect.x + platformRect.width) {
            
            // Check if fighter's feet are at or below platform top
            float fighterBottom = fighterRect.y + fighterRect.height;
            float platformTop = platformRect.y;
            
            if (fighterBottom >= platformTop && fighterBottom <= platformTop + platformRect.height) {
                // Land on platform
                position.y = platformTop - height;
                speedY = 0.0f;
                if (platform.IsGround()) {
                    standingOnGroundPlatform = true;
                }
                return true;
            }
        }
    }
    
    return false;
}

void Fighter::characterDeath(const Mushroom& enemy)
{
    float deltaTime = GetFrameTime();
    
    // Handle ongoing death animation
    if (isDying) {
        deathTimer += deltaTime;
        
        // After 2 seconds, respawn the player
        if (deathTimer >= 2.0f) {
            // Respawn at starting position
            width = (textureWidth * scale - 2 * offsetX);
            height = (textureHeight * scale - offsetY);
            position = startingPosition;
            speedY = 0.0f;
            isDying = false;
            deathTimer = 0.0f;
            invincibilityTimer = 2.0f; // 2 seconds of invincibility after respawn
            isOnGround = true;
            isFallingThrough = false;
        }
        return; // Skip collision check while dying
    }
    
    // Only check collision if not dying and not invincible
    if (invincibilityTimer <= 0.0f) {
        Rectangle a = GetRect();
        Rectangle b = enemy.GetRect();
        
        if (CheckCollisionRecs(a, b)) {
            // Collision detected - trigger death

            isDying = true;
            width = textureWidth * scale;
            height = textureHeight * scale;
            deathTimer = 0.0f;
            lives -= 1;
        }
    }
}

void Fighter::Update(const std::vector<Platform>& platforms){
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const float GRAVITY = 800.0f;
    const int JUMP_VELOCITY = -500;
    float deltaTime = GetFrameTime();

    // Update invincibility timer
    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= deltaTime;
    }

    // Skip all input and physics if dying
    if (isDying) {
        return;
    }

    // Handle crouching
    if(!(IsKeyDown(KEY_DOWN) && isOnGround && !isJumping)) {
        isCrouching = false;

    } else {
        isCrouching = true;
    }

    // Horizontal movement (can't run while crouching or mid-attack)
    if(!isCrouching && !isAttacking && !comboAttack) {
        if(IsKeyDown(KEY_RIGHT) && position.x + width < screenWidth){
            position.x += speed;
            facingRight = true;
            isRunning = true;
        }
        else if(IsKeyDown(KEY_LEFT) && position.x > 0){
            position.x -= speed;
            facingRight = false;
            isRunning = true;
        }
    }

    // attack input with cooldown and combo extension
    if(IsKeyPressed(KEY_Z)){
        double now = GetTime();
        if(isAttacking){
            // upgrade to combo (extra frames)
            comboAttack = true;
            animationStartTime = now; // restart timing for full combo anim
            hasDealtDamage = false; // Reset damage flag for combo
        } else if(now >= nextAttackReadyTime){
            isAttacking = true;
            comboAttack = false;
            animationStartTime = now;
            hasDealtDamage = false; // Reset damage flag for new attack
        }
    }
    // Apply gravity
    float deltaTime2 = GetFrameTime();
    speedY += GRAVITY * deltaTime2;

    // Update falling through timer
    if (isFallingThrough) {
        fallingThroughTimer -= deltaTime;
        if (fallingThroughTimer <= 0.0f) {
            isFallingThrough = false;
        }
    }

    // Fall through platforms when pressing Down + X (only if on ground and not already falling through)
    if (IsKeyPressed(KEY_X) && IsKeyDown(KEY_DOWN) && isOnGround && !isFallingThrough) {
        if(standingOnGroundPlatform) return; // Can't fall through ground platforms
        isFallingThrough = true;
        fallingThroughTimer = 0.5f; // Fall through for 0.5 seconds (increased for reliability)
        isOnGround = false;
        speedY = 100.0f; // Stronger downward velocity to ensure we pass through quickly
    }
    // Normal jump input
    else if (IsKeyPressed(KEY_X) && isOnGround && !isFallingThrough) {
        speedY = JUMP_VELOCITY;
        isJumping = true;
        isOnGround = false;
        animationStartTime = GetTime();
    }

    // Update vertical position
    position.y += speedY * deltaTime;

    // Check platform collisions (only if not falling through)
    if (!isFallingThrough) {
        isOnGround = CheckPlatformCollision(platforms);
    } else {
        isOnGround = false;
    }
    
    // Prevent falling through bottom of screen
    if (position.y + height >= screenHeight) {
        position.y = screenHeight - height;
        speedY = 0.0f;
        isOnGround = true;
        isLanding = false;
        isFallingThrough = false; // Stop falling through when hitting screen bottom
    }

    if(isOnGround){
        if(isOnGround){
            if(isJumping) {
                animationStartTime = GetTime();
            }
            isJumping = false;
            speedY = 0.0f;
            if(isLanding) {
                animationStartTime = GetTime();
            } else {
                isLanding = false;
            }
        }
        if(!IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT)){
            isRunning = false;
        }
    
        // Reset crouch animation timer when crouch starts
        static bool wasCrouching = false;
        if(isCrouching && !wasCrouching) {
            animationStartTime = GetTime();
            wasCrouching = true;
        } else if(!isCrouching) {
            wasCrouching = false;
        }

        // finish attack/combo when animation ends and set cooldown
        if(isAttacking || comboAttack){
            double now = GetTime();
            float duration = comboAttack ? comboDuration : attackDuration;
            if((now - animationStartTime) >= duration){
                isAttacking = false;
                comboAttack = false;
                nextAttackReadyTime = now + attackCooldown;
                animationStartTime = now; // reset for idle/run
                hasDealtDamage = false; // Reset damage flag when attack ends
            }
        }
    }
}
