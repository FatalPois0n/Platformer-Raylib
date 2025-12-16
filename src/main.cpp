#include <raylib.h>
#include <vector>
#include "soldier.hpp"
#include "platform.hpp"

int main() 
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_WINDOW_TOPMOST);
    InitWindow(1920, 1080, "Game");

    // Get Monitor Dimensions
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    
    SetWindowSize(screenWidth, screenHeight);
    
    SetTargetFPS(60);
    
    Texture2D background = LoadTexture("resources/background/background_layer_1.png");
    Texture2D midground = LoadTexture("resources/background/background_layer_2.png");
    Texture2D foreground = LoadTexture("resources/background/background_layer_3.png");
    Texture2D tileset = LoadTexture("resources/oak_woods_tileset.png");

    Soldier soldier;
    
    // Define tile properties
    int tileWidth = 24;
    int tileHeight = 24;
    int tileRow = 8;
    int tileCol = 12;

    // Create platforms
    std::vector<Platform> platforms;
    
    // Ground platform (bottom of screen) - marked as ground so you can't fall through it
    platforms.push_back(Platform(0, screenHeight - 80, screenWidth, 80, true));
    
    // Additional platforms at different heights (not ground, so you can fall through them)
    platforms.push_back(Platform(200, screenHeight - 200, 300, 40, false));
    platforms.push_back(Platform(600, screenHeight - 400, 350, 40, false));
    platforms.push_back(Platform(1100, screenHeight - 300, 400, 40, false));
    platforms.push_back(Platform(300, screenHeight - 550, 250, 40, false));

    // Game loop
    while(WindowShouldClose() == false){
        // Update
        soldier.Update(platforms);

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
        
        // Draw soldier
        soldier.Draw();
        
        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(tileset);
    CloseWindow();
    return 0;
}