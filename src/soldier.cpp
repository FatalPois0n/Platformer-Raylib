#include "soldier.hpp"
#include <raylib.h>

Soldier::Soldier()
{
    image = LoadTexture("resources/soldier2.png");
    position = { 50.0f, 1080.0f - (float)image.height - 80 }; // Start on ground
    speed = 5;
    speedY = 0.0f;
    isJumping = false;
    isOnGround = false;
    isFallingThrough = false;
    fallingThroughTimer = 0.0f;
    facingRight = true;
}

Soldier::~Soldier()
{
    UnloadTexture(image);
}

void Soldier::Draw()
{
    Rectangle sourceRec = {
        0, 
        0, 
        (float)image.width * (facingRight ? 1 : -1), 
        (float)image.height
    };
    
    Rectangle destRec = {
        position.x,
        position.y,
        (float)image.width,
        (float)image.height
    };
    
    DrawTexturePro(image, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
}

Rectangle Soldier::GetRect()
{
    return Rectangle{position.x, position.y, (float)image.width, (float)image.height};
}

bool Soldier::CheckPlatformCollision(const std::vector<Platform>& platforms)
{
    Rectangle soldierRect = GetRect();
    
    for (const auto& platform : platforms) {
        Rectangle platformRect = platform.GetRect();
        
        // Skip collision if we're falling through platforms (but not ground)
        if (isFallingThrough && !platform.IsGround()) {
            continue;
        }
        
        // Check if soldier is falling and overlapping with platform
        if (speedY >= 0 && 
            soldierRect.x + soldierRect.width > platformRect.x &&
            soldierRect.x < platformRect.x + platformRect.width) {
            
            // Check if soldier's feet are at or below platform top
            float soldierBottom = soldierRect.y + soldierRect.height;
            float platformTop = platformRect.y;
            
            if (soldierBottom >= platformTop && soldierBottom <= platformTop + platformRect.height) {
                // Land on platform
                position.y = platformTop - image.height;
                speedY = 0.0f;
                return true;
            }
        }
    }
    
    return false;
}

void Soldier::Update(const std::vector<Platform>& platforms)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const float GRAVITY = 400.0f;
    const int JUMP_VELOCITY = -350;

    // Horizontal movement
    if(IsKeyDown(KEY_RIGHT) && position.x + image.width < screenWidth){
        position.x += speed;
        facingRight = true;
    }
    else if(IsKeyDown(KEY_LEFT) && position.x > 0){
        position.x -= speed;
        facingRight = false;
    }

    // Apply gravity
    float deltaTime = GetFrameTime();
    speedY += GRAVITY * deltaTime;

    // Update falling through timer
    if (isFallingThrough) {
        fallingThroughTimer -= deltaTime;
        if (fallingThroughTimer <= 0.0f) {
            isFallingThrough = false;
        }
    }

    // Fall through platforms when pressing Down + X (only if on ground and not already falling through)
    if (IsKeyPressed(KEY_X) && IsKeyDown(KEY_DOWN) && isOnGround && !isFallingThrough) {
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
    if (position.y + image.height >= screenHeight) {
        position.y = screenHeight - image.height;
        speedY = 0.0f;
        isOnGround = true;
        isFallingThrough = false; // Stop falling through when hitting screen bottom
    }
}
