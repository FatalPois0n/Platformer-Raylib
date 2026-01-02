#include <raylib.h>
#include <math.h>
#include <vector>
#include <string>
#include <cctype>
#include "fighter.hpp"
#include "platform.hpp"
#include "animation.h"
#include "enemy.hpp"
#include "mushroom.hpp"
#include "slime.hpp"
#include "huntress.hpp"
#include "bringerofdeath.hpp"

enum class GameState {
    Start,
    Level1,
    Level2,
    Level3,
    BossLevel,
    Pause,
    GameWon,
    GameOver
};

// function declarations for level management
void ClearEnemies(std::vector<Enemy*>& enemies);
void SpawnLevel1Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void SpawnLevel2Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void SpawnLevel3Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void SpawnBossLevelEnemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight);
void CreateLevel1Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight);
void CreateLevel2Platforms(std::vector<Platform>& platforms, int screenWidth, int screenHeight, int groundHeight);
void CreateLevel3Platforms(std::vector<Platform>& platforms, std::vector<Wall>& walls, int screenWidth, int screenHeight, int groundHeight);
void CreateBossLevelPlatforms(std::vector<Platform>& platforms, std::vector<Wall>& walls, int screenWidth, int screenHeight, int groundHeight);


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
    Texture2D bossBG = LoadTexture("resources/background/awesomeCavePixelArt.png");
    Texture2D gameLogo = LoadTexture("resources/logo1a.png");
    Texture2D tileset = LoadTexture("resources/oak_woods_tileset.png");
    
    float masterVolume = 1.0f;

    Music menuMusic = LoadMusicStream("resources/music/Xasthur - Exit HD.mp3");
    Music level1Music = LoadMusicStream("resources/music/Fallen Down.mp3");
    Music level2Music = LoadMusicStream("resources/music/Rush E.mp3");
    Music level3Music = LoadMusicStream("resources/music/hkmori - anybody can find love (except you.).mp3");
    Music bossLevelMusic = LoadMusicStream("resources/music/MEGALOVANIA - Toby Fox.mp3");
    Music gameOverMusic = LoadMusicStream("resources/music/SUPER MARIO - game over - sound effect.mp3");
    Music gameWonMusic = LoadMusicStream("resources/music/Drum Roll (Ending Celebration) - Sound Effect  ProSounds.mp3");
    SetMusicVolume(menuMusic, 0.6f);
    SetMusicVolume(level1Music, 0.6f);
    SetMusicVolume(level2Music, 0.6f);
    SetMusicVolume(level3Music, 0.6f);
    SetMusicVolume(bossLevelMusic, 0.6f);
    SetMusicVolume(gameOverMusic, 0.6f);
    SetMusicVolume(gameWonMusic, 0.6f);
    SetMasterVolume(masterVolume);

    Fighter fighter;
    
    std::vector<Enemy*> enemies;

    Mushroom::LoadSharedTexture();

    GameState gameState = GameState::Start;
    GameState prevState = GameState::Level1;
    float gameOverTimer = 0.0f;
    float gameOverTextY = -200.0f;
    float gameWonTimer = 0.0f;
    float gameWonTextY = -200.0f;

    bool volumeSliderActive = false;

    // Cheat code: type "bigbang" to kill all enemies
    const char* CHEAT_PHRASE = "bigbang";
    const int CHEAT_LEN = 7;
    int cheatProgress = 0;

    // Konami code: up, up, down, down, left, right, left, right, B, A
    const int KONAMI_CODE[] = { KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_X, KEY_Z };
    const int KONAMI_LEN = 10;
    int konamiProgress = 0;
    bool konamiActivated = false;

    Color orange = { 255, 117, 0, (unsigned char)(0.67*255) };
    Color yellow = { 255, 214, 0, (unsigned char)(0.78*255) };
    Color green = { 111, 214, 0, (unsigned char)(0.78*255) };
    
    int tileWidth = 24;
    int tileHeight = 24;
    int tileRow = 8;
    int tileCol = 12;
    int wallWidth = 48;
    int wallHeight = 120;
    int wallRow = 9;
    int wallCol = 6;
    int groundHeight = 80;

    std::vector<Platform> platforms;
    std::vector<Wall> walls;
    std::vector<Spear> spears;

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
                fighter.Reset();
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
                fighter.resetPos();
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
            if (Enemy::allEnemiesDefeated(enemies)) {
                // All enemies defeated, proceed to next level
                gameState = GameState::Level3;
                // Initialize Level 3
                fighter.resetPos();
                ClearEnemies(enemies);
                platforms.clear();
                walls.clear();
                CreateLevel3Platforms(platforms, walls, screenWidth, screenHeight, groundHeight);
                SpawnLevel3Enemies(enemies, screenWidth, screenHeight);
                StopMusicStream(level2Music);
            }
            if (IsKeyPressed(KEY_P)) {
                prevState = GameState::Level2;
                gameState = GameState::Pause;
            }
        }
        else if (gameState == GameState::Level3) {
            if (Enemy::allEnemiesDefeated(enemies)) {
                // All enemies defeated, proceed to next level
                gameState = GameState::BossLevel;
                // Initialize Boss level
                fighter.resetPos();
                fighter.speed = 7; //increased speed for boss level
                ClearEnemies(enemies);
                platforms.clear();
                walls.clear();
                CreateBossLevelPlatforms(platforms, walls, screenWidth, screenHeight, groundHeight);
                SpawnBossLevelEnemies(enemies, screenWidth, screenHeight);
                StopMusicStream(level3Music);
            }
            if (IsKeyPressed(KEY_P)) {
                prevState = GameState::Level3;
                gameState = GameState::Pause;
            }
        }
        else if (gameState == GameState::BossLevel) {
            if (Enemy::allEnemiesDefeated(enemies)) {
                // All enemies defeated, proceed to game over
                gameState = GameState::GameWon;
                gameWonTimer = 0.0f;
                gameWonTextY = -200.0f;
                StopMusicStream(bossLevelMusic);
            }
            if (IsKeyPressed(KEY_P)) {
                prevState = GameState::BossLevel;
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

        // Konami code detection
        if (!volumeSliderActive && !konamiActivated) {
            if (IsKeyPressed(KONAMI_CODE[konamiProgress])) {
                konamiProgress++;
                if (konamiProgress >= KONAMI_LEN) {
                    // Konami code activated!
                    konamiActivated = true;
                    fighter.lives = 999;
                    fighter.baseDamage = 1000.0f;
                    fighter.comboDamage = 1000.0f;
                }
            } else if (IsKeyPressed(KEY_SPACE)) {
                // Reset on wrong key
                konamiProgress = 0;
            }
        }

        // Update music streams
        UpdateMusicStream(menuMusic);
        UpdateMusicStream(level1Music);
        UpdateMusicStream(level2Music);
        UpdateMusicStream(level3Music);
        UpdateMusicStream(bossLevelMusic);
        UpdateMusicStream(gameOverMusic);
        UpdateMusicStream(gameWonMusic);

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
        } 
        else if (gameState == GameState::Level2) {
            // Play level 2 music;
            StopMusicStream(menuMusic);
            StopMusicStream(gameOverMusic);
            StopMusicStream(level1Music);
            ResumeMusicStream(level2Music);
            if (!IsMusicStreamPlaying(level2Music)) {
                PlayMusicStream(level2Music);
            }
        } 
        else if (gameState == GameState::Level3) {
            // Play level 3 music;
            StopMusicStream(menuMusic);
            StopMusicStream(level2Music);
            StopMusicStream(level1Music);
            ResumeMusicStream(level3Music);
            if (!IsMusicStreamPlaying(level3Music)) {
                PlayMusicStream(level3Music);
            }
        } 
        else if (gameState == GameState::BossLevel) {
            // Play boss level music;
            StopMusicStream(menuMusic);
            StopMusicStream(level2Music);
            StopMusicStream(level1Music);
            StopMusicStream(level3Music);
            ResumeMusicStream(bossLevelMusic);
            if (!IsMusicStreamPlaying(bossLevelMusic)) {
                PlayMusicStream(bossLevelMusic);
            }
        }
        else if (gameState == GameState::Pause) {
            // No music while paused
            PauseMusicStream(menuMusic);
            PauseMusicStream(level1Music);
            PauseMusicStream(level2Music);
            PauseMusicStream(level3Music);
            PauseMusicStream(bossLevelMusic);
            PauseMusicStream(gameOverMusic);
        } else if (gameState == GameState::GameOver) {
            // Play game over music; stop others
            StopMusicStream(menuMusic);
            StopMusicStream(level1Music);
            StopMusicStream(level2Music);
            StopMusicStream(level3Music);
            StopMusicStream(bossLevelMusic);

            ResumeMusicStream(gameOverMusic);
            if (!IsMusicStreamPlaying(gameOverMusic)) {
                PlayMusicStream(gameOverMusic);
            }
        }
        else if (gameState == GameState::GameWon) {
            // Play game won music; stop others
            StopMusicStream(menuMusic);
            StopMusicStream(level1Music);
            StopMusicStream(level2Music);
            StopMusicStream(level3Music);
            StopMusicStream(bossLevelMusic);

            ResumeMusicStream(gameWonMusic);
            if (!IsMusicStreamPlaying(gameWonMusic)) {
                PlayMusicStream(gameWonMusic);
            }
        }

        // Update (during gameplay and not when slider is active)
        if ((gameState != GameState::Start && gameState != GameState::GameOver && gameState != GameState::Pause) && !volumeSliderActive)
        {
            // Handle cheat phrase typing
            int ch = GetCharPressed();
            while (ch > 0) {
                char c = (char)std::tolower(ch);
                if (c == CHEAT_PHRASE[cheatProgress]) {
                    cheatProgress++;
                } else {
                    cheatProgress = (c == CHEAT_PHRASE[0]) ? 1 : 0;
                }
                if (cheatProgress >= CHEAT_LEN) {
                    // Kill all current level enemies
                    for (auto* enemy : enemies) {
                        if (!enemy->IsDead()) {
                            enemy->TakeDamage(1000000.0f);
                        }
                    }
                    cheatProgress = 0;
                }
                ch = GetCharPressed();
            }

            fighter.Update(platforms, walls);
            // Update all enemies
            for (auto* enemy : enemies) {
                if (!enemy->IsDead()) {
                    enemy->Update(platforms, walls, fighter);
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
            DrawRectangleGradientEx(
                {0, 0, (float)screenWidth, (float)screenHeight},
                RED,
                ORANGE,
                DARKGRAY,
                MAROON
            );
            gameOverTimer += GetFrameTime();      
            // text animation
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
        // Update GameWon state
        if (gameState == GameState::GameWon) {
            gameWonTimer += GetFrameTime();      
            // text animation
            float targetY = (screenHeight - 120) / 2.0f;
            if (gameWonTextY < targetY) {
                gameWonTextY += 800.0f * GetFrameTime(); // Fall speed
                if (gameWonTextY > targetY) {
                    gameWonTextY = targetY;
                }
            }
            
            // After 10 seconds, return to start
            if (gameWonTimer >= 10.0f) {
                gameState = GameState::Start;
                // Clean up for restart
                ClearEnemies(enemies);
                platforms.clear();
                StopMusicStream(gameWonMusic);
                gameWonTimer = 0.0f;
                gameWonTextY = -200.0f;
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
                {((float)screenWidth - (float)gameLogo.width * 0.75f) / 2, ((float)screenHeight - (float)gameLogo.height* 0.75f) / 3, (float)gameLogo.width* 0.75f , (float)gameLogo.height* 0.75f },
                {0, 0},
                0.0f,
                WHITE
            );

            const char* prompt = "Press v to start";
            int promptFontSize = 96;
            int promptWidth = MeasureText(prompt, promptFontSize);
            int promptX = (screenWidth - promptWidth) / 2;
            int promptY = screenHeight - 200;
            const char* cred = "Made by Faizan Ali with <3";
            int credFontSize = 72;
            int credWidth = MeasureText(cred, credFontSize);
            int credX = 200.0f;
            int credY = screenHeight / 3;
            Vector2 credPos = { (float)credX, (float)credY };
            
            // Flashy prompt text with alpha oscillation
            float time = GetTime();
            float alpha = (sin(time * 3.0f) + 1.0f) / 2.0f; // Oscillates between 0 and 1
            alpha = alpha * 0.7f + 0.3f; // Clamp between 0.3 and 1.0 for better visibility
            Color flashyYellow = { 255, 214, 0, (unsigned char)(alpha * 255) };
            DrawText(prompt, promptX, promptY, promptFontSize, flashyYellow);
            
            float alpha2 = (cos(time * 3.0f) + 1.0f) / 2.0f; // Oscillates between 0 and 1
            Color flashyGreen = { 0, 255, 41, (unsigned char)(alpha2 * 255) };
            DrawTextPro(fnt_chewy, cred,  credPos, {0,0}, -30.0f,  credFontSize, 1.0f,  flashyGreen);
            
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

                if(gameState == GameState::BossLevel || gameState == GameState::GameWon) {
                DrawTexturePro(
                    bossBG,
                    {0, 0, (float)bossBG.width, (float)bossBG.height},
                    {0, 0, (float)screenWidth, (float)screenHeight},
                    {0, 0},
                    0.0f,
                    WHITE
                );
            }

            // Draw all platforms
            for (auto& platform : platforms) {
                platform.Draw(tileset, tileWidth, tileHeight, tileRow, tileCol);
            }
            // Draw all walls
            for (auto& wall : walls) {
                wall.Draw(tileset, wallWidth, wallHeight, wallRow, wallCol);
            }

            // HUD texts (only shown during gameplay & pause)
            if (gameState != GameState::Start && gameState != GameState::GameOver) {
            std::string livesCount = "Lives: " + std::to_string(fighter.lives);
            std::string invin;
            if (gameState == GameState::Level1){
                invin = "Stage 1";
            }
            else if (gameState == GameState::Level2){
                invin = "Stage 2";
            }
            else if (gameState == GameState::Level3){
                invin = "Stage 3";
            }
            else if (gameState == GameState::BossLevel){
                invin = "Boss Level";
            }
        
            float fontSpacing = 3.0f;

            Vector2 textSize1 = MeasureTextEx(fnt_chewy, livesCount.c_str(), HUDfontSize, fontSpacing);
            Vector2 textPosition1 = { 50.0f , 20.0f };
            DrawTextEx(fnt_chewy, livesCount.c_str(), textPosition1, HUDfontSize, fontSpacing, WHITE);

            Vector2 textSize2 = MeasureTextEx(fnt_chewy, invin.c_str(), HUDfontSize, fontSpacing);
            Vector2 textPosition2 = {(float)screenWidth - textSize2.x - 50.0f, 20.0f };
            DrawTextEx(fnt_chewy, invin.c_str(), textPosition2, HUDfontSize, fontSpacing, WHITE);
            
            // Boss health bar for Boss Level
            if (gameState == GameState::BossLevel) {
                for (auto* enemy : enemies) {
                    Boss* boss = dynamic_cast<Boss*>(enemy);
                    if (boss && !boss->IsDead()) {
                        float bossHealth = boss->GetHealth();
                        float maxHealth = 500.0f; // Assuming boss max health is 500
                        float healthPercent = bossHealth / maxHealth;
                        
                        // Boss health bar
                        float barWidth = 900.0f;
                        float barHeight = 8.0f;
                        float barX = (screenWidth - barWidth) / 2.0f;
                        float barY = 120.0f;
    
                        // Background bar (red)
                        DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
                        // Health bar (green to red gradient based on health)
                        Color healthColor = healthPercent > 0.5f ? GREEN : (healthPercent > 0.25f ? YELLOW : RED);
                        DrawRectangle(barX, barY, barWidth * healthPercent, barHeight, healthColor);
                        DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);
                        
                        // Boss name
                        const char* bossName = "BRINGER OF DEATH";
                        Vector2 bossNameSize = MeasureTextEx(fnt_chewy, bossName, HUDfontSize * 1.75f, fontSpacing);
                        Vector2 bossNamePos = { barX + (barWidth - bossNameSize.x) / 2.0f, barY - bossNameSize.y - 5.0f };
                        DrawTextEx(fnt_chewy, bossName, bossNamePos, HUDfontSize * 1.75f, fontSpacing, RED);
                        
                        break; // Only draw for first boss found
                    }
                }
            }
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
            if (gameState == GameState::GameWon) {
                // Apply golden overlay
                Color goldOverlay = {255, 215, 0, 100};
                DrawRectangle(0, 0, screenWidth, screenHeight, goldOverlay);
                // Draw falling "GAME WON" text in gold
                const char* gameWonText = "GAME WON";
                int gameWonFontSize = 120;
                int gameWonWidth = MeasureText(gameWonText, gameWonFontSize);
                int gameWonX = (screenWidth - gameWonWidth) / 2;
                DrawText(gameWonText, gameWonX, (int)gameWonTextY, gameWonFontSize, GOLD);
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
    UnloadTexture(bossBG);
    UnloadTexture(gameLogo);
    UnloadTexture(tileset);
    UnloadMusicStream(menuMusic);
    UnloadMusicStream(level1Music);
    UnloadMusicStream(level2Music);
    UnloadMusicStream(level3Music);
    UnloadMusicStream(bossLevelMusic);
    UnloadMusicStream(gameOverMusic);
    UnloadMusicStream(gameWonMusic);
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
    enemies.push_back(new Mushroom({400.0f, (float)screenHeight - (160*6)}));
    enemies.push_back(new Mushroom({1600.0f, (float)screenHeight - 300.0f}));
    enemies.push_back(new Slime({960.0f, (float)screenHeight - 400.0f}));
}

void SpawnLevel2Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight) {
    // Level 2: 3 mushrooms and 2 slimes
    enemies.push_back(new Mushroom({1500.0f, (float)screenHeight - 100.0f}));
    enemies.push_back(new Mushroom({960.0f, (float)screenHeight - 350.0f}));
    enemies.push_back(new Mushroom({1500.0f, (float)screenHeight - 650.0f}));
    enemies.push_back(new Slime({700.0f, (float)screenHeight - 300.0f}));
    enemies.push_back(new Slime({1300.0f, (float)screenHeight - 450.0f}));
}
void SpawnLevel3Enemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight) {
    // Level 3: 2 huntresses
    enemies.push_back(new Huntress({250.0f, (float)screenHeight - 950.0f}));
    enemies.push_back(new Huntress({screenWidth - 250.0f, (float)screenHeight - 950.0f}));
    
}
void SpawnBossLevelEnemies(std::vector<Enemy*>& enemies, int screenWidth, int screenHeight) {
    enemies.push_back(new Boss({1500.0f, (float)screenHeight - 538.0f}));
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
void CreateLevel3Platforms(std::vector<Platform>& platforms, std::vector<Wall>& walls, int screenWidth, int screenHeight, int groundHeight) {
    // Ground platform (same for all levels)
    platforms.push_back(Platform(0, screenHeight - groundHeight, screenWidth, groundHeight, true));
    
    // Level 3 specific platforms - Climbing sections on either side of screen


    // Left side platforms
    platforms.push_back(Platform(screenWidth/2 - 50 - 375, screenHeight - 250, 325, 40, false));
    platforms.push_back(Platform(0, screenHeight - 400, 350, 40, false));
    platforms.push_back(Platform(screenWidth/2 - 50 - 375, screenHeight - 550, 325, 40, false));
    platforms.push_back(Platform(0, screenHeight - 700, 350, 40, false));
    
    // Center platform
    platforms.push_back(Platform(200, screenHeight - 850, screenWidth - 400, 40, false));
    
    // // Right side platforms
    platforms.push_back(Platform(screenWidth/2 + 75, screenHeight - 250, 325, 40, false));
    platforms.push_back(Platform(screenWidth - 350, screenHeight - 400, 350, 40, false));
    platforms.push_back(Platform(screenWidth/2 + 75, screenHeight - 550, 325, 40, false));
    platforms.push_back(Platform(screenWidth - 350, screenHeight - 700, 350, 40, false));
    
    // // Additional scattered platforms
    // platforms.push_back(Platform(600, screenHeight - 800, 200, 40, false));
    // platforms.push_back(Platform(1200, screenHeight - 800, 200, 40, false));
}
void CreateBossLevelPlatforms(std::vector<Platform>& platforms, std::vector<Wall>& walls, int screenWidth, int screenHeight, int groundHeight) {
    // Ground platform (same for all levels)
    platforms.push_back(Platform(0, screenHeight - groundHeight, screenWidth, groundHeight, true));
    
    // Boss level specific platforms - tall walls on sides and central elevated platform

    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*1), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*2), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*3), 200, 40, false));
    platforms.push_back(Platform(0, screenHeight - groundHeight - (160*4), 200, 40, false));

    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*1), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*2), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*3), 200, 40, false));
    platforms.push_back(Platform(screenWidth - 200, screenHeight - groundHeight - (160*4), 200, 40, false));

    platforms.push_back(Platform(300, screenHeight - (160*5), screenWidth - 600, 40, false));

}