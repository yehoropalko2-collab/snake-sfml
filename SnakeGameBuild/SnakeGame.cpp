#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>
#include <random>
#include <string>

// ======================================================
// SPRITE SHEET SETTINGS
// ======================================================
// snake-graphics.png
// size: 320x256
// grid: 5x4
// tile size: 64x64
//
// Head:
// (3,0)=up   (4,0)=right   (4,1)=down   (3,1)=left
//
// Tail:
// (3,2)=up   (4,2)=right   (4,3)=down   (3,3)=left
//
// Body:
// (1,0)=horizontal   (2,1)=vertical
//
// Corners:
// (0,0)=top-left
// (2,0)=top-right
// (0,1)=bottom-left
// (2,2)=bottom-right
//
// Food:
// (0,3)=apple
// ======================================================

static constexpr int TILE = 64;

sf::IntRect tileRect(int col, int row)
{
    return sf::IntRect(col * TILE, row * TILE, TILE, TILE);
}

class Game
{
public:
    Game()
        : window(sf::VideoMode(800, 600), "Snake"),
        distX(0, gridWidth - 1),
        distY(0, gridHeight - 1)
    {
        // ======================================================
        // FONT
        // ======================================================
        if (!font.loadFromFile("Pixel_Font-7.ttf"))
            ok = false;

        // ======================================================
        // TEXTURES / SPRITES
        // ======================================================
        if (!snakeTexture.loadFromFile("snake-graphics.png"))
            ok = false;

        snakeTexture.setSmooth(false);
        snakeSprite.setTexture(snakeTexture);

        float scale = (float)cellSize / (float)TILE;
        snakeSprite.setScale(scale, scale);

        // ======================================================
        // BACKGROUNDS
        // ======================================================
        if (!bgMenuTexture.loadFromFile("menuMap.png"))
            ok = false;
        bgMenuSprite.setTexture(bgMenuTexture);

        if (!bgGameTexture.loadFromFile("map.png"))
            ok = false;
        bgGameSprite.setTexture(bgGameTexture);

        // ======================================================
        // SOUNDS
        // ======================================================
        if (!gameOverBuffer.loadFromFile("hit.wav"))
            ok = false;
        gameOverSound.setBuffer(gameOverBuffer);

        if (!eatingFoodBuffer.loadFromFile("eat.wav"))
            ok = false;
        eatingFoodSound.setBuffer(eatingFoodBuffer);

        if (!pauseBuffer.loadFromFile("pause.wav"))
            ok = false;
        pauseSound.setBuffer(pauseBuffer);

        // ======================================================
        // MUSIC
        // ======================================================
        if (!menuMusic.openFromFile("menu.ogg"))
            ok = false;
        menuMusic.setLoop(true);
        menuMusic.setVolume(40.f);
        menuMusic.play();

        if (!playMusic.openFromFile("play.ogg"))
            ok = false;
        playMusic.setLoop(true);
        playMusic.setVolume(40.f);

        // ======================================================
        // UI TEXT
        // ======================================================
        menuTitle = sf::Text("SNAKE", font, 64);
        menuTitle.setFillColor(sf::Color::White);

        menuHint = sf::Text("Press ENTER to start", font, 48);
        menuHint.setFillColor(sf::Color::White);

        gameOverTitle = sf::Text("Game Over", font, 48);
        gameOverTitle.setFillColor(sf::Color::White);

        gameOverHint = sf::Text("Press R to restart", font, 24);
        gameOverHint.setFillColor(sf::Color::White);

        pauseTitle = sf::Text("PAUSE", font, 48);
        pauseTitle.setFillColor(sf::Color::White);

        pauseHint = sf::Text("Press P to continue", font, 24);
        pauseHint.setFillColor(sf::Color::White);

        scoreText = sf::Text("Score: 0", font, 30);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(sf::Vector2f(10.f, 10.f));

        PointsRecord = sf::Text("Points Record: ", font, 30);
        PointsRecord.setFillColor(sf::Color::White);
        PointsRecord.setPosition(sf::Vector2f(550.f, 10.f));

        // ======================================================
        // UI OVERLAY
        // ======================================================
        overlay.setSize(sf::Vector2f(800.f, 600.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 160));

        // ======================================================
        // GAME STARTUP
        // ======================================================
        resetGame();
        state = GameState::Menu;
    }

    void run()
    {
        if (!ok) return;

        while (window.isOpen())
        {
            float dt = clock.restart().asSeconds();
            events();
            update(dt);
            render();
        }
    }

private:
    // ======================================================
    // CONSTANTS
    // ======================================================
    static constexpr int cellSize = 32;
    static constexpr float moveDelay = 0.15f;
    static constexpr int gridWidth = 800 / cellSize;
    static constexpr int gridHeight = 600 / cellSize;

    // ======================================================
    // CORE
    // ======================================================
    sf::RenderWindow window;
    bool ok = true;

    // ======================================================
    // FONT / UI
    // ======================================================
    sf::Font font;
    sf::Text gameOverTitle, gameOverHint;
    sf::Text pauseTitle, pauseHint;
    sf::Text menuTitle, menuHint;
    sf::Text scoreText;
    sf::Text PointsRecord;
    sf::RectangleShape overlay;

    // ======================================================
    // GAME STATE
    // ======================================================
    enum GameState { Menu, GameOver, Playing, Paused };
    GameState state = Menu;

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
    sf::Vector2i food{ 0, 0 };
    std::deque<sf::Vector2i> snake;
    int score = 0;
    int pointsRecordValue = 0;
    bool musicMuted = false;

    sf::Clock clock;
    float timer = 0.f;

private:
    // ======================================================
    // UTILS
    // ======================================================
    static void centerText(sf::Text& t, float cx, float cy)
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(cx, cy);
    }

    bool isOnSnake(const sf::Vector2i& p) const
    {
        for (const auto& s : snake)
            if (p == s) return true;
        return false;
    }

    sf::Vector2i foodSpawn()
    {
        sf::Vector2i f;
        do
        {
            f = { distX(gen), distY(gen) };
        } while (isOnSnake(f));
        return f;
    }

    void resetGame()
    {
        dir = { 1, 0 };
        snake.clear();
        snake.push_back({ 10, 10 });
        snake.push_back({ 9, 10 });
        snake.push_back({ 8, 10 });
        snake.push_back({ 7, 10 });

        score = 0;
        food = foodSpawn();
        scoreText.setString("Score: 0");
        timer = 0.f;
    }

    // ======================================================
    // EVENTS / INPUT
    // ======================================================
    void events()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (state == GameState::Menu && event.key.code == sf::Keyboard::Enter)
                {
                    resetGame();
                    menuMusic.stop();
                    playMusic.play();
                    state = GameState::Playing;
                }
                else if (state == GameState::GameOver && event.key.code == sf::Keyboard::R)
                {
                    resetGame();
                    state = GameState::Playing;
                    playMusic.play();
                }
                else if (event.key.code == sf::Keyboard::Escape)
                {
                    state = GameState::Menu;
                    menuMusic.play();
                    playMusic.stop();
                }
                else if (state == GameState::Playing && event.key.code == sf::Keyboard::P)
                {
                    state = GameState::Paused;
                    pauseSound.play();
                    playMusic.pause();
                }
                else if (state == GameState::Paused && event.key.code == sf::Keyboard::P)
                {
                    state = GameState::Playing;
                    pauseSound.play();
                    playMusic.play();
                }
                else if (event.key.code == sf::Keyboard::M)
                {
                    musicMuted = !musicMuted;
                    if (musicMuted)
                        playMusic.setVolume(0.f);
                    else
                        playMusic.setVolume(40.f);
                }
            }
        }

        if (state == GameState::Playing)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && dir.x != 1) dir = { -1,  0 };
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && dir.x != -1) dir = { 1,  0 };
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && dir.y != 1) dir = { 0, -1 };
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && dir.y != -1) dir = { 0,  1 };
        }
    }

    // ======================================================
    // UPDATE
    // ======================================================
    void update(float dt)
    {
        if (state != GameState::Playing) return;

        timer += dt;

        while (timer >= moveDelay)
        {
            timer -= moveDelay;

            sf::Vector2i head = snake.front();
            sf::Vector2i newHead = head + dir;

            if (newHead.x < 0 || newHead.y < 0 ||
                newHead.x >= gridWidth || newHead.y >= gridHeight)
            {
                gameOverSound.play();
                state = GameState::GameOver;
                break;
            }

            bool willGrow = (newHead == food);
            bool hitSelf = isOnSnake(newHead) && !(newHead == snake.back() && !willGrow);

            if (hitSelf)
            {
                gameOverSound.play();
                playMusic.stop();
                state = GameState::GameOver;
                break;
            }

            snake.push_front(newHead);

            if (willGrow)
            {
                eatingFoodSound.play();
                food = foodSpawn();
                score++;
                scoreText.setString("Score: " + std::to_string(score));
            }
            else
            {
                snake.pop_back();
            }
        }
    }

    // ======================================================
    // DRAW HELPERS
    // ======================================================
    void drawTile(int col, int row, int gx, int gy)
    {
        snakeSprite.setTextureRect(tileRect(col, row));
        snakeSprite.setOrigin(0.f, 0.f);
        snakeSprite.setRotation(0.f);
        snakeSprite.setPosition((float)gx * cellSize, (float)gy * cellSize);
        window.draw(snakeSprite);
    }

    void drawHead(int gx, int gy, sf::Vector2i d)
    {
        if (d.y == -1) drawTile(3, 0, gx, gy);
        else if (d.x == 1) drawTile(4, 0, gx, gy);
        else if (d.y == 1) drawTile(4, 1, gx, gy);
        else drawTile(3, 1, gx, gy);
    }

    void drawTail(int gx, int gy, sf::Vector2i d)
    {
        if (d.y == -1) drawTile(4, 3, gx, gy);
        else if (d.x == 1) drawTile(3, 3, gx, gy);
        else if (d.y == 1) drawTile(3, 2, gx, gy);
        else drawTile(4, 2, gx, gy);
    }

    void drawBody(int gx, int gy, sf::Vector2i prev, sf::Vector2i cur, sf::Vector2i next)
    {
        bool hasLeft = (prev.x == cur.x - 1 || next.x == cur.x - 1);
        bool hasRight = (prev.x == cur.x + 1 || next.x == cur.x + 1);
        bool hasUp = (prev.y == cur.y - 1 || next.y == cur.y - 1);
        bool hasDown = (prev.y == cur.y + 1 || next.y == cur.y + 1);

        if (hasLeft && hasRight) { drawTile(1, 0, gx, gy); return; }
        if (hasUp && hasDown) { drawTile(2, 1, gx, gy); return; }

        if (hasRight && hasDown) { drawTile(0, 0, gx, gy); return; }
        if (hasLeft && hasDown) { drawTile(2, 0, gx, gy); return; }
        if (hasRight && hasUp) { drawTile(0, 1, gx, gy); return; }
        if (hasLeft && hasUp) { drawTile(2, 2, gx, gy); return; }

        drawTile(1, 0, gx, gy);
    }

    // ======================================================
    // RENDER
    // ======================================================
    void render()
    {
        window.clear();

        if (state == GameState::Menu)
            window.draw(bgMenuSprite);
        else
            window.draw(bgGameSprite);

        if (state == GameState::Playing || state == GameState::GameOver)
        {
            drawTile(0, 3, food.x, food.y);

            int n = (int)snake.size();

            for (int i = 0; i < n; i++)
            {
                int gx = snake[i].x;
                int gy = snake[i].y;

                if (i == 0)
                {
                    sf::Vector2i headDir = snake[0] - snake[1];
                    drawHead(gx, gy, headDir);
                }
                else if (i == n - 1)
                {
                    sf::Vector2i tailDir = snake[i] - snake[i - 1];
                    drawTail(gx, gy, tailDir);
                }
                else
                {
                    drawBody(gx, gy, snake[i - 1], snake[i], snake[i + 1]);
                }
            }

            window.draw(scoreText);
            window.draw(PointsRecord);

            sf::Text muteHint;
            muteHint = sf::Text(musicMuted ? "M: Sound OFF" : "M: Sound ON", font, 18);
            muteHint.setFillColor(sf::Color::White);
            muteHint.setPosition(10.f, 40.f);
            window.draw(muteHint);
        }

        if (state == GameState::Menu)
        {
            window.draw(overlay);
            centerText(menuTitle, 400.f, 230.f);
            centerText(menuHint, 400.f, 330.f);
            window.draw(menuTitle);
            window.draw(menuHint);
        }

        if (state == GameState::GameOver)
        {
            playMusic.stop();
            window.draw(overlay);
            centerText(gameOverTitle, 400.f, 230.f);
            centerText(gameOverHint, 400.f, 330.f);
            window.draw(gameOverTitle);
            window.draw(gameOverHint);

            if (score > pointsRecordValue)
            {
                pointsRecordValue = score;
                PointsRecord.setString("Points Record: " + std::to_string(pointsRecordValue));
            }
        }

        if (state == GameState::Paused)
        {
            window.draw(overlay);
            centerText(pauseTitle, 400.f, 280.f);
            centerText(pauseHint, 400.f, 350.f);
            window.draw(pauseTitle);
            window.draw(pauseHint);
        }

        window.display();
    }
};

int main()
{
    Game game;
    game.run();
    return 0;
}