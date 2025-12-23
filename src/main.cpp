#include <raylib.h>
#include <math.h>
#include <vector>
#include <string>
#include "fighter.hpp"
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"
#include "mushroom.hpp"
#include "slime.hpp"

enum class GameState {
    Start,
    Level1,
    Level2,
    Pause,
    GameOver
};

// Forward declarations for level management functions
void ClearEnemies(std::vector<Enemy*>& enemies);
void SpawnLevel1Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void SpawnLevel2Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void CreateLevel1Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight);
void CreateLevel2Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight);


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
    InitAudioDevice();

    float HUDfontSize = 64.0f;
    Font fnt_chewy = LoadFontEx("resources/fonts/Chewy-Regular.ttf", HUDfontSize, 0, 0);
    
    Texture2D background = LoadTexture("resources/background/background_layer_1.png");
    Texture2D midground = LoadTexture("resources/background/background_layer_2.png");
    Texture2D foreground = LoadTexture("resources/background/background_layer_3.png");
    Texture2D gameLogo = LoadTexture("resources/logo1.png");
    Texture2D tileset = LoadTexture("resources/oak_woods_tileset.png");
    
    float masterVolume = 1.0f;

    Music menuMusic = LoadMusicStream("resources/music/Xasthur - Exit HD.mp3");
    Music level1Music = LoadMusicStream("resources/music/Fallen Down.mp3");
    Music level2Music = LoadMusicStream("resources/music/Guy thinks I can't play Rush E and calls me a liar.mp3");
    Music gameOverMusic = LoadMusicStream("resources/music/SUPER MARIO - game over - sound effect.mp3");
    SetMusicVolume(menuMusic, 0.6f);
    SetMusicVolume(level1Music, 0.6f);
    SetMusicVolume(level2Music, 0.6f);
    SetMusicVolume(gameOverMusic, 0.6f);
    SetMasterVolume(masterVolume);

    Fighter fighter;
    
    // Create enemies using polymorphic container
    std::vector<Enemy*> enemies;
    
    // Load shared mushroom texture
    Mushroom::LoadSharedTexture();
    
    // Enemies will be spawned when entering each level

    GameState gameState = GameState::Start;
    GameState prevState = GameState::Level1;
    float gameOverTimer = 0.0f;
    float gameOverTextY = -200.0f;

    bool volumeSliderActive = false;

    Color orange = { 255, 117, 0, (unsigned char)(0.67*255) };
    Color yellow = { 255, 214, 0, (unsigned char)(0.78*255) };
    Color green = { 111, 214, 0, (unsigned char)(0.78*255) };
    
    // Define tile properties
    int tileWidth = 24;
    int tileHeight = 24;
    int tileRow = 8;
    int tileCol = 12;
    int groundHeight = 80;

    // Create platforms
    std::vector<Platform> platforms;
    
    // Platforms will be created when entering each level

    // Game loop
    while(!WindowShouldClose()){
        // Input handling for state transitions
        if (gameState == GameState::Start) {
            if (IsKeyPressed(KEY_V)) {
                gameState = GameState::Level1;
                // Initialize Level 1
                ClearEnemies(enemies);
                platforms.clear();
                CreateLevel1Platforms(platforms, screenWidth, screenHeight, groundHeight);
                SpawnLevel1Enemies(enemies, screenWidth, screenHeight);
                fighter.Reset(); // Reset fighter state without reloading textures
            }
        } 
        else if (gameState == GameState::Pause) {
            if (IsKeyPressed(KEY_P))
                gameState = prevState;
        }
        else if (gameState == GameState::Level1) {
            if (Enemy::allEnemiesDefeated(enemies)) {
                // All enemies defeated, proceed to next level
                gameState = GameState::Level2;
                // Initialize Level 2
                ClearEnemies(enemies);
                platforms.clear();
                CreateLevel2Platforms(platforms, screenWidth, screenHeight, groundHeight);
                SpawnLevel2Enemies(enemies, screenWidth, screenHeight);
                StopMusicStream(level1Music);
            }
            if (IsKeyPressed(KEY_P)) {
                prevState = GameState::Level1;
                gameState = GameState::Pause;
            }
        }
        else if (gameState == GameState::Level2) {
            if (IsKeyPressed(KEY_P)) {
                prevState = GameState::Level2;
                gameState = GameState::Pause;
            }
        }

        // Toggle volume slider with tilde/grave key
        if (IsKeyPressed(KEY_GRAVE)) {
            volumeSliderActive = !volumeSliderActive;
        }

        // Adjust master volume when slider is active
        if (volumeSliderActive) {
            if (IsKeyDown(KEY_LEFT) && masterVolume > 0.0f) {
                masterVolume -= 0.005f;
                SetMasterVolume(masterVolume);
            }
            if (IsKeyDown(KEY_RIGHT) && masterVolume < 1.0f) {
                masterVolume += 0.005f;
                SetMasterVolume(masterVolume);
            }
        }

        // Update music streams
        UpdateMusicStream(menuMusic);
        UpdateMusicStream(level1Music);
        UpdateMusicStream(gameOverMusic);

        // Music control per game state
        if (gameState == GameState::Start) {
            // Play menu music; ensure others are stopped
            StopMusicStream(level1Music);
            StopMusicStream(gameOverMusic);
            ResumeMusicStream(menuMusic);
            if (!IsMusicStreamPlaying(menuMusic)) {
                PlayMusicStream(menuMusic);
            }
        } else if (gameState == GameState::Level1) {
            // Play level 1 music; ensure menu/gameover are stopped
            StopMusicStream(menuMusic);
            StopMusicStream(gameOverMusic);
            ResumeMusicStream(level1Music);
            if (!IsMusicStreamPlaying(level1Music)) {
                PlayMusicStream(level1Music);
            }
        } else if (gameState == GameState::Level2) {
            // Play level 2 music;
            ResumeMusicStream(level2Music);
            if (!IsMusicStreamPlaying(level2Music)) {
                PlayMusicStream(level2Music);
            }
        } else if (gameState == GameState::Pause) {
            // No music while paused
            PauseMusicStream(menuMusic);
            PauseMusicStream(level1Music);
            PauseMusicStream(gameOverMusic);
        } else if (gameState == GameState::GameOver) {
            // Play game over music; stop others
            StopMusicStream(menuMusic);
            StopMusicStream(level1Music);
            StopMusicStream(level2Music);
            ResumeMusicStream(gameOverMusic);
            if (!IsMusicStreamPlaying(gameOverMusic)) {
                PlayMusicStream(gameOverMusic);
            }
        }

        // Update (during gameplay and not when slider is active)
        if ((gameState != GameState::Start && gameState != GameState::GameOver) && !volumeSliderActive) {
            fighter.Update(platforms);
            
            // Update all enemies
            for (auto* enemy : enemies) {
                if (!enemy->IsDead()) {
                    enemy->Update(platforms, fighter);
                }
            }
            
            // Check if fighter is attacking and deal damage to all enemies
            if (fighter.IsAttacking()) {
                for (auto* enemy : enemies) {
                    if (!enemy->IsDead()) {
                        if (fighter.comboAttack) {
                            fighter.PerformComboSlash(*enemy);
                        } else {
                            fighter.PerformSlash(*enemy);
                        }
                    }
                }
            }
            
            fighter.characterDeath(enemies);
            
            // Check for game over
            if (fighter.lives < 0) {
                gameState = GameState::GameOver;
                gameOverTimer = 0.0f;
                gameOverTextY = -200.0f;
            }
        }
        
        // Update GameOver state
        if (gameState == GameState::GameOver) {
            gameOverTimer += GetFrameTime();
            
            // Animate text falling from top to center
            float targetY = (screenHeight - 120) / 2.0f;
            if (gameOverTextY < targetY) {
                gameOverTextY += 800.0f * GetFrameTime(); // Fall speed
                if (gameOverTextY > targetY) {
                    gameOverTextY = targetY;
                }
            }
            
            // After 10 seconds, return to start
            if (gameOverTimer >= 10.0f) {
                gameState = GameState::Start;
                // Clean up for restart
                ClearEnemies(enemies);
                platforms.clear();
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == GameState::Start) {
            // Draw only background image, game logo and start text
            DrawTexturePro(
                background,
                {0, 0, (float)background.width, (float)background.height},
                {0, 0, (float)screenWidth, (float)screenHeight},
                {0, 0},
                0.0f,
                WHITE
            );

            DrawTexturePro(
                gameLogo,
                {0, 0, (float)gameLogo.width, (float)gameLogo.height},
                {((float)screenWidth - (float)gameLogo.width * (float)1.5) / 2, (float)screenHeight / 3 - (float)gameLogo.height * (float)1.5 / 2, gameLogo.width * (float)1.5, gameLogo.height * (float)1.5},
                {0, 0},
                0.0f,
                WHITE
            );

            const char* prompt = "Press start button";
            int promptFontSize = 96;
            int promptWidth = MeasureText(prompt, promptFontSize);
            int promptX = (screenWidth - promptWidth) / 2;
            int promptY = screenHeight - 200;
            
            // Flashy prompt text with alpha oscillation
            float time = GetTime();
            float alpha = (sin(time * 3.0f) + 1.0f) / 2.0f; // Oscillates between 0 and 1
            alpha = alpha * 0.7f + 0.3f; // Clamp between 0.3 and 1.0 for better visibility
            Color flashyYellow = { 255, 214, 0, (unsigned char)(alpha * 255) };
            DrawText(prompt, promptX, promptY, promptFontSize, flashyYellow);
            
            // Volume slider UI (bottom of screen)
            if (volumeSliderActive) {
                int sliderWidth = screenWidth / 3;
                int sliderHeight = 12;
                int sliderX = (screenWidth - sliderWidth) / 2;
                int sliderY = screenHeight - 40;
                DrawRectangle(sliderX, sliderY, sliderWidth, sliderHeight, GRAY);
                DrawRectangle(sliderX, sliderY, (int)(sliderWidth * masterVolume), sliderHeight, RAYWHITE);
                DrawRectangleLines(sliderX, sliderY, sliderWidth, sliderHeight, LIGHTGRAY);
            }
        }
        else {
            // Draw background layers for gameplay and pause
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

            // HUD texts (only shown during gameplay & pause)
            if (gameState != GameState::Start && gameState != GameState::GameOver) {
            std::string livesCount = "Lives: " + std::to_string(fighter.lives);
            std::string invin = "Invincibility timer: " + std::to_string(fighter.invincibilityTimer);

            float fontSpacing = 3.0f;

            Vector2 textSize1 = MeasureTextEx(fnt_chewy, livesCount.c_str(), HUDfontSize, fontSpacing);
            Vector2 textPosition1 = { 50.0f , 20.0f };
            DrawTextEx(fnt_chewy, livesCount.c_str(), textPosition1, HUDfontSize, fontSpacing, WHITE);

            Vector2 textSize2 = MeasureTextEx(fnt_chewy, invin.c_str(), HUDfontSize, fontSpacing);
            Vector2 textPosition2 = {(float)screenWidth - textSize2.x, 20.0f };
            DrawTextEx(fnt_chewy, invin.c_str(), textPosition2, HUDfontSize, fontSpacing, WHITE);
            }

            // Draw Character and enemies
            fighter.Draw();
            for (auto* enemy : enemies) {
                if (!enemy->IsDead()) {
                    enemy->Draw();
                }
            }

            if (gameState == GameState::Pause) {
                // Apply a slight brownish hue overlay and pause text
                Color brownOverlay = {165, 105, 60, 80};
                DrawRectangle(0, 0, screenWidth, screenHeight, brownOverlay);
                
                // Draw centered "PAUSED" text
                const char* pausedText = "PAUSED";
                int pausedFontSize = 120;
                int pausedWidth = MeasureText(pausedText, pausedFontSize);
                int pausedX = (screenWidth - pausedWidth) / 2;
                int pausedY = (screenHeight - pausedFontSize) / 2;
                DrawText(pausedText, pausedX, pausedY, pausedFontSize, RAYWHITE);
            }
            
            if (gameState == GameState::GameOver) {
                // Apply grayscale overlay
                Color grayOverlay = {50, 50, 50, 180};
                DrawRectangle(0, 0, screenWidth, screenHeight, grayOverlay);
                
                // Draw falling "GAME OVER" text in red
                const char* gameOverText = "GAME OVER";
                int gameOverFontSize = 120;
                int gameOverWidth = MeasureText(gameOverText, gameOverFontSize);
                int gameOverX = (screenWidth - gameOverWidth) / 2;
                DrawText(gameOverText, gameOverX, (int)gameOverTextY, gameOverFontSize, RED);
            }

            // Volume slider UI (bottom of screen)
            if (volumeSliderActive) {
                int sliderWidth = screenWidth / 3;
                int sliderHeight = 12;
                int sliderX = (screenWidth - sliderWidth) / 2;
                int sliderY = screenHeight - 40;
                DrawRectangle(sliderX, sliderY, sliderWidth, sliderHeight, GRAY);
                DrawRectangle(sliderX, sliderY, (int)(sliderWidth * masterVolume), sliderHeight, RAYWHITE);
                DrawRectangleLines(sliderX, sliderY, sliderWidth, sliderHeight, LIGHTGRAY);
            }
        }

        EndDrawing();
    }

    // Clean up enemies
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    Mushroom::UnloadSharedTexture();

    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(gameLogo);
    UnloadTexture(tileset);
    UnloadMusicStream(menuMusic);
    UnloadMusicStream(level1Music);
    UnloadMusicStream(level2Music);
    UnloadMusicStream(gameOverMusic);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// Level management function implementations
void ClearEnemies(std::vector<Enemy*>& enemies) {
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
}

void SpawnLevel1Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight) {
    // Level 1: 2 mushrooms and 1 slime
    enemies.push_back(new Mushroom({300.0f, (float)screenHeight - 300.0f}));
    enemies.push_back(new Mushroom({1600.0f, (float)screenHeight - 300.0f}));
    enemies.push_back(new Slime({960.0f, (float)screenHeight - 400.0f}));
}

void SpawnLevel2Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight) {
    // Level 2: 3 mushrooms and 2 slimes
    enemies.push_back(new Mushroom({400.0f, (float)screenHeight - 500.0f}));
    enemies.push_back(new Mushroom({960.0f, (float)screenHeight - 350.0f}));
    enemies.push_back(new Mushroom({1500.0f, (float)screenHeight - 650.0f}));
    enemies.push_back(new Slime({700.0f, (float)screenHeight - 300.0f}));
    enemies.push_back(new Slime({1300.0f, (float)screenHeight - 450.0f}));
}

void CreateLevel1Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight) {
    // Ground platform (same for all levels)
    platforms.push_back(Platform(0, screenHeight - groundHeight, screenWidth, groundHeight, true));
    
    // Level 1 specific platforms - vertical towers on sides
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
}

void CreateLevel2Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight) {
    // Ground platform (same for all levels)
    platforms.push_back(Platform(0, screenHeight - groundHeight, screenWidth, groundHeight, true));
    
    // Level 2 specific platforms - scattered pattern with more vertical variety
    // Left side staircase
    platforms.push_back(Platform(100, screenHeight - 200, 250, 40, false));
    platforms.push_back(Platform(200, screenHeight - 350, 250, 40, false));
    platforms.push_back(Platform(300, screenHeight - 500, 250, 40, false));
    platforms.push_back(Platform(400, screenHeight - 650, 250, 40, false));
    
    // Center platforms
    platforms.push_back(Platform(screenWidth/2 - 200, screenHeight - 300, 400, 40, false));
    platforms.push_back(Platform(screenWidth/2 - 150, screenHeight - 600, 300, 40, false));
    
    // Right side platforms
    platforms.push_back(Platform(screenWidth - 450, screenHeight - 400, 300, 40, false));
    platforms.push_back(Platform(screenWidth - 350, screenHeight - 550, 250, 40, false));
    platforms.push_back(Platform(screenWidth - 250, screenHeight - 700, 200, 40, false));
    
    // Additional scattered platforms
    platforms.push_back(Platform(600, screenHeight - 800, 200, 40, false));
    platforms.push_back(Platform(1200, screenHeight - 800, 200, 40, false));
}