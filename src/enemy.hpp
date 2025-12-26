#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"

// Forward declaration
class Fighter;

// Base class for all enemies
class Enemy {
public:
    virtual ~Enemy() = default;

    // Pure virtual functions that all enemies must implement
    virtual void Update(const std::vector<Platform>& platforms, const std::vector<Wall>& walls, const Fighter& player) = 0;
    virtual void Draw() = 0;
    virtual Rectangle GetRect() const = 0;
    virtual Rectangle GetHitbox() const = 0;
    virtual void TakeDamage(float damageAmount) = 0;
    virtual bool IsDead() const = 0;
    virtual float GetHealth() const = 0;
    static bool allEnemiesDefeated(const std::vector<Enemy*>& enemies) {
        for (const auto* enemy : enemies) {
            if (!enemy->IsDead()) {
                return false;
            }
        }
        return true;
    }

protected:
    Enemy() = default;
};
