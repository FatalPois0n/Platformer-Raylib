#pragma once
#include <raylib.h>

class Platform {
public:
    Platform(float x, float y, float width, float height, bool isGround = false);
    void Draw(Texture2D tileset, int tileWidth, int tileHeight, int tileRow, int tileCol);
    Rectangle GetRect() const;
    bool IsGround() const;
    
private:
    Rectangle rect;
    bool ground;
};

// Wall is a vertical platform: blocks horizontal motion but can optionally be stood upon
class Wall {
public:
    Wall(float x, float y, float width, float height, bool standable = true);
    void Draw(Texture2D tileset, int tileWidth, int tileHeight, int tileRow, int tileCol);
    Rectangle GetRect() const;
    bool CanStandOnTop() const;   // if true, treat top like a small platform
    bool BlocksMovement() const;  // always true for a wall

private:
    Rectangle rect;
    bool standableTop;
};