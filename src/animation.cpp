#include "animation.h"
#include <cstdlib>
#include <vector>

//animation metadata
AnimDef idle   = {0, 0, 6, 10};
AnimDef attack   = {1, 0, 6, 10};
AnimDef combo   = {1, 0, 8, 10};
AnimDef run    = {2, 0, 8, 10};
AnimDef jump   = {3, 0, 14, 10};
AnimDef land   = {4, 0, 5, 10};
AnimDef death  = {6, 0, 12, 10};
AnimDef crouch  = {9, 0, 3, 10};


spriteAnimation CreateSpriteAnimation(Texture2D atlas, int framesPerSecond, Rectangle rectangles[], int rectanglesCount, bool loop)
{
    spriteAnimation spriteAnimation = {
        .atlas = atlas,
        .framesPerSecond = framesPerSecond,
        .rectanglesCount = rectanglesCount,
        .loop = loop
    };

    Rectangle* mem = (Rectangle*)malloc(sizeof(Rectangle) * rectanglesCount);
    if (mem == NULL){
        TraceLog(LOG_FATAL, "CreateSpriteAnimation: Failed to allocate memory for rectangles");
        spriteAnimation.Rectangles = NULL;
        return spriteAnimation;
    }

    spriteAnimation.Rectangles = mem;
    for (int i = 0; i < rectanglesCount; i++){
        spriteAnimation.Rectangles[i] = rectangles[i];
    }

    return spriteAnimation;
}

void DrawSpriteAnimationPro(spriteAnimation animation, Rectangle dest, Vector2 origin, float rotation, Color tint, bool facingRight, float elapsedTime)
{
    int index;
    if (animation.loop) {
        index = (int)(elapsedTime * animation.framesPerSecond) % animation.rectanglesCount;
    } else {
        // Clamp to last frame if animation shouldn't loop
        int maxIndex = (int)(elapsedTime * animation.framesPerSecond);
        index = (maxIndex >= animation.rectanglesCount) ? animation.rectanglesCount - 1 : maxIndex;
    }
    Rectangle sourceRec = animation.Rectangles[index];
    if (!facingRight) {
        sourceRec.width = -sourceRec.width;
    }
    DrawTexturePro(animation.atlas, sourceRec, dest, origin, rotation, tint);
}

void DisposeSpriteAnimation(spriteAnimation animation)
{
    if (animation.Rectangles != NULL){
        free(animation.Rectangles);
    }
}

std::vector<Rectangle> BuildFrames(int startRow,int startCol,int frameCount,int columns,int frameW,int frameH,int trimX,int trimY
) {
    std::vector<Rectangle> frames;
    frames.reserve(frameCount);

    for (int i = 0; i < frameCount; i++) {
        int col = (startCol + i) % columns;
        int row = startRow + (startCol + i) / columns;

        frames.push_back(Rectangle{
            (float)col * frameW + trimX,
            (float)row * frameH + trimY,
            (float)frameW - 2 * trimX,
            (float)frameH - trimY
        });
    }
    return frames;
}

spriteAnimation LoadAnim(
    const AnimDef& def,
    Texture2D texture,
    const AtlasInfo& atlas,
    float trimX,
    float trimY,
    bool loop
) {
    std::vector<Rectangle> frames;
    frames.reserve(def.frames);

    for (int i = 0; i < def.frames; i++) {
        int index = def.col + i;
        int col = index % atlas.columns;
        int row = def.row + index / atlas.columns;

        frames.push_back(Rectangle{
            col * atlas.frameWidth  + trimX,
            row * atlas.frameHeight + trimY,
            atlas.frameWidth  - 2 * trimX,
            atlas.frameHeight - trimY
        });
    }

    return CreateSpriteAnimation(texture, def.fps, frames.data(), def.frames, loop);
}
