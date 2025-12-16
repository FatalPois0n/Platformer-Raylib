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