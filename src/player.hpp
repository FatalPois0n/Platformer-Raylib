#pragma once
#include <raylib.h>
#include <vector>
#include "platform.hpp"

// Abstract base for playable characters
class Player {
public:
    virtual ~Player() = default;

    // Common interface both Soldier and Fighter already implement
    virtual void Update(const std::vector<Platform>& platforms) = 0;
    virtual void Draw() = 0;
    virtual Rectangle GetRect() const = 0;
    virtual Rectangle GetHitbox() const = 0;
};
