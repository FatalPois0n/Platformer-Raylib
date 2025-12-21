#include <raylib.h>
#include <vector>
#include <string>
#include "fighter.hpp"
#include "platform.hpp"
#include "animation.h"
#include "mushroom.hpp"


int main() 
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_WINDOW_TOPMOST);
    InitWindow(1920, 1080, "Game");

    HideCursor();

    // Get Monitor Dimensions
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);

    const Vector2 screenSize = { (float)screenWidth, (float)screenHeight };
    
    SetWindowSize(screenWidth, screenHeight);
    
    SetTargetFPS(60);

    
    Texture2D background = LoadTexture("resources/background/background_layer_1.png");
    Texture2D midground = LoadTexture("resources/background/background_layer_2.png");
    Texture2D foreground = LoadTexture("resources/background/background_layer_3.png");
    Texture2D tileset = LoadTexture("resources/oak_woods_tileset.png");

    Fighter fighter;
    Mushroom mushroom;
    
    // Define tile properties
    int tileWidth = 24;
    int tileHeight = 24;
    int tileRow = 8;
    int tileCol = 12;
    int groundHeight = 80;

    // Create platforms
    std::vector<Platform> platforms;
    
    // Ground platform (bottom of screen) - marked as ground
    platforms.push_back(Platform(0, screenHeight - groundHeight, screenWidth, groundHeight, true));
    
    // Additional platforms at different heights (not ground)
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*1), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*2), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*3), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*4), 200, 40, false));

    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*1), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*2), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*3), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*4), 200, 40, false));

    platforms.push_back(Platform(300, screenHeight - (160*5), screenWidth - 600, 40, false));
    platforms.push_back(Platform(1100, screenHeight - 300, 400, 40, false));
    platforms.push_back(Platform(300, screenHeight - 550, 250, 40, false));

    // Game loop
    while(!WindowShouldClose()){
        // Update
        fighter.Update(platforms);
        fighter.characterDeath(mushroom);
        if(!mushroom.IsDead())
            mushroom.Update(platforms, fighter);
        // Check if fighter is attacking and deal damage
        if (fighter.IsAttacking()) {
            if (fighter.comboAttack) {
                fighter.PerformComboSlash(mushroom);
            } else {
                fighter.PerformSlash(mushroom);
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Draw background layers
        DrawTexturePro(
            background,
            {0, 0, (float)background.width, (float)background.height},
            {0, 0, (float)screenWidth, (float)screenHeight},
            {0, 0},
            0.0f,
            WHITE
        );
        
        DrawTexturePro(
            midground,
            {0, 0, (float)midground.width, (float)midground.height},
            {0, 0, (float)screenWidth, (float)screenHeight},
            {0, 0},
            0.0f,
            WHITE
        );
        
        DrawTexturePro(
            foreground,
            {0, 0, (float)foreground.width, (float)foreground.height},
            {0, 0, (float)screenWidth, (float)screenHeight},
            {0, 0},
            0.0f,
            WHITE
        );

        // Draw all platforms
        for (auto& platform : platforms) {
            platform.Draw(tileset, tileWidth, tileHeight, tileRow, tileCol);
        }

        //drawing all the text
        std::string livesCount = "Lives: " + std::to_string(fighter.lives);
        std::string invin = "Invincibility timer: " + std::to_string(fighter.invincibilityTimer);

        float fontSize = 32.0f;
        float fontSpacing = 3.0f;

        Font fnt_chewy = LoadFontEx("resources/fonts/Chewy.ttf", fontSize, 0, 0);

        Vector2 textSize1 = MeasureTextEx(fnt_chewy, livesCount.c_str(), fontSize, fontSpacing);
        Vector2 textSize2 = MeasureTextEx(fnt_chewy, invin.c_str(), fontSize, fontSpacing);
        Vector2 textPosition1 = { textSize1.x + 20.0f, 20.0f };
        Vector2 textPosition2 = { screenWidth - 400.0f, 20.0f };
    
        DrawTextEx(fnt_chewy, livesCount.c_str(), textPosition1, fontSize, fontSpacing, WHITE);
        DrawTextEx(fnt_chewy, invin.c_str(), textPosition2, fontSize, fontSpacing, WHITE);
        
        // Draw Character and enemies
        fighter.Draw(mushroom);
        if(!mushroom.IsDead())
            mushroom.Draw();
        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(tileset);
    CloseWindow();
    return 0;
}