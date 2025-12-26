#include "platform.hpp"

Platform::Platform(float x, float y, float width, float height, bool isGround)
{
    rect = {x, y, width, height};
    ground = isGround;
}

void Platform::Draw(Texture2D tileset, int tileWidth, int tileHeight, int tileRow, int tileCol)
{
    Rectangle sourceRec = {
        (float)(tileCol * tileWidth),
        (float)(tileRow * tileHeight),
        (float)tileWidth,
        (float)tileHeight
    };
    
    // Draw tiles to fill the platform width
    for (float x = rect.x; x < rect.x + rect.width; x += tileWidth) {
        Rectangle destRec = {x, rect.y, (float)tileWidth, rect.height};
        DrawTexturePro(tileset, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    }
}

Rectangle Platform::GetRect() const
{
    return rect;
}

bool Platform::IsGround() const
{
    return ground;
}

Wall::Wall(float x, float y, float width, float height, bool standable)
{
    rect = {x, y, width, height};
    standableTop = standable;
}

void Wall::Draw(Texture2D tileset, int tileWidth, int tileHeight, int tileRow, int tileCol)
{
    Rectangle sourceRec = {
        216.0f,
        144.0f,
        (float)tileWidth,
        (float)tileHeight
    };

    // Tile the texture along the height of the wall
    for (float y = rect.y; y < rect.y + rect.height; y += tileHeight) {
        Rectangle destRec = {rect.x, y, rect.width, (float)tileHeight};
        DrawTexturePro(tileset, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    }
}

Rectangle Wall::GetRect() const
{
    return rect;
}

bool Wall::CanStandOnTop() const
{
    return standableTop;
}

bool Wall::BlocksMovement() const
{
    return true;
}