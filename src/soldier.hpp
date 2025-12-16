#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"

class Soldier {
public:
    Soldier();
    ~Soldier();
    void Update(const std::vector<Platform>& platforms);
    void Draw();
    Rectangle GetRect();
    
private:
    Texture2D image;
    Vector2 position;
    int speed;
    float speedY; 
    bool isJumping;
    bool isOnGround;
    bool isFallingThrough;
    float fallingThroughTimer;
    bool facingRight;
    
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
};