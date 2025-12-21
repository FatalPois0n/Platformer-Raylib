#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "raylib.h"

typedef struct spriteAnimation {
    Texture2D atlas;
    Rectangle* Rectangles;
    int framesPerSecond;
    int rectanglesCount;
    bool loop;
} spriteAnimation;

typedef struct AnimDef {
    int row;
    int col;
    int frames;
    int fps;
} AnimDef;

struct AtlasInfo {
    int frameWidth;
    int frameHeight;
    int columns;
};

spriteAnimation CreateSpriteAnimation(Texture2D atlas, int framesPerSecond, Rectangle rectangles[], int rectanglesCount, bool loop);

void DrawSpriteAnimationPro(spriteAnimation animation, Rectangle dest, Vector2 origin, float rotation, Color tint, bool facingRight, float elapsedTime);

void DisposeSpriteAnimation(spriteAnimation animation);

spriteAnimation LoadAnim(
    const AnimDef& def,
    Texture2D texture,
    const AtlasInfo& atlas,
    float trimX,
    float trimY,
    bool loop
);


extern AnimDef jump;
extern AnimDef idle;
extern AnimDef run;
extern AnimDef land;
extern AnimDef death;
extern AnimDef crouch;
extern AnimDef attack;
extern AnimDef combo;

#endif