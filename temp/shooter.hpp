#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"
#include "enemy.hpp"

// Lightweight projectile used by shooters
struct Projectile {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float damage;
    float lifeSeconds;
    Color color;
    bool alive;

    Rectangle GetRect() const {
        return Rectangle{ position.x - radius, position.y - radius, radius * 2.0f, radius * 2.0f };
    }
};

// Mixin-style base providing shooting capabilities
class Shooter {
public:
    Shooter();
    virtual ~Shooter() = default;

    // Spawn a projectile from origin, heading based on facingRight
    void Shoot(Vector2 origin, bool facingRight);

    // Update and draw projectiles; call each frame
    void UpdateProjectiles(const std::vector<Platform>& platforms, const std::vector<Wall>& walls);
    void DrawProjectiles() const;

    // Resolve projectile hits against enemies (removes projectile and applies damage)
    void ResolveHits(std::vector<Enemy*>& enemies);

protected:
    std::vector<Projectile> projectiles;
    float projectileSpeed;
    float fireCooldownSeconds;
    double lastShotTime;
};
