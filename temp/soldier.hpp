#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "shooter.hpp"
#include "player.hpp"

class Soldier : public Player, public Shooter {
public:
    Soldier();
    ~Soldier();
    void Update(const std::vector<Platform>& platforms) override;
    void Draw() override;
    Rectangle GetRect() const override;
    // Optional: resolve projectile hits against enemies
    void ResolveShotsOnEnemies(std::vector<Enemy*>& enemies);
    
private:
    Texture2D image;
    Vector2 position;
    int speed;
    float speedY; 
    bool isJumping;
    bool isOnGround;
    bool isFallingThrough;
    float fallingThroughTimer;
    bool standingOnGroundPlatform;
    bool facingRight;
    
    bool CheckPlatformCollision(const std::vector<Platform>& platforms);
};