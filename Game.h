#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>
#include <random>
#include <string>

class Game
{
public:
    Game();
    void run();

private:
    // ======================================================
    // CONSTANTS
    // ======================================================
    static constexpr int TILE = 64;

    static constexpr int cellSize = 40;

    static constexpr int gridWidth = 36;
    static constexpr int gridHeight = 27;

    static constexpr int windowWidth = gridWidth * cellSize;   // 1440
    static constexpr int windowHeight = gridHeight * cellSize; // 1080

    static constexpr float moveDelay = 0.15f;

    // ======================================================
    // GAME STATE
    // ======================================================
    enum class GameState
    {
        Menu,
        GameOver,
        Playing,
        Paused
    };

    // ======================================================
    // CORE
    // ======================================================
    sf::RenderWindow window;
    bool ok = true;

    // ======================================================
    // FONT / UI
    // ======================================================
    sf::Font font;

    sf::Text gameOverTitle;
    sf::Text gameOverHint;
    sf::Text pauseTitle;
    sf::Text pauseHint;
    sf::Text menuTitle;
    sf::Text menuHint;
    sf::Text scoreText;
    sf::Text pointsRecordText;
    sf::Text muteHintText;

    sf::RectangleShape overlay;

    // ======================================================
    // STATE
    // ======================================================
    GameState state = GameState::Menu;

    // ======================================================
    // SPRITES / TEXTURES
    // ======================================================
    sf::Texture snakeTexture;
    sf::Sprite snakeSprite;

    sf::Texture bgMenuTexture;
    sf::Sprite bgMenuSprite;

    sf::Texture bgGameTexture;
    sf::Sprite bgGameSprite;

    // ======================================================
    // AUDIO
    // ======================================================
    sf::SoundBuffer gameOverBuffer;
    sf::SoundBuffer eatingFoodBuffer;
    sf::SoundBuffer pauseBuffer;

    sf::Sound eatingFoodSound;
    sf::Sound gameOverSound;
    sf::Sound pauseSound;

    sf::Music menuMusic;
    sf::Music playMusic;

    // ======================================================
    // RANDOM
    // ======================================================
    std::random_device rd;
    std::mt19937 gen{ rd() };
    std::uniform_int_distribution<int> distX;
    std::uniform_int_distribution<int> distY;

    // ======================================================
    // GAME DATA
    // ======================================================
    sf::Vector2i dir{ 1, 0 };
    sf::Vector2i nextDir{ 1, 0 };
    bool directionChangedThisStep = false;
    sf::Vector2i food{ 0, 0 };
    std::deque<sf::Vector2i> snake;

    int score = 0;
    int pointsRecordValue = 0;
    bool musicMuted = false;

    sf::Clock clock;
    float timer = 0.f;

private:
    // ======================================================
    // INIT
    // ======================================================
    void loadAssets();
    void setupUi();
    void setupOverlay();

    // ======================================================
    // UTILS
    // ======================================================
    static sf::IntRect tileRect(int col, int row);
    static void centerText(sf::Text& t, float cx, float cy);

    bool isOnSnake(const sf::Vector2i& p) const;
    sf::Vector2i foodSpawn();
    void resetGame();
    void updateMuteText();
    void updateRecordText();

    // ======================================================
    // LOOP PARTS
    // ======================================================
    void events();
    void update(float dt);
    void render();

    // ======================================================
    // DRAW HELPERS
    // ======================================================
    void drawTile(int col, int row, int gx, int gy);
    void drawHead(int gx, int gy, sf::Vector2i d);
    void drawTail(int gx, int gy, sf::Vector2i d);
    void drawBody(int gx, int gy, sf::Vector2i prev, sf::Vector2i cur, sf::Vector2i next);
};