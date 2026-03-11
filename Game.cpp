#include "Game.h"
#include <sstream>
#include <iomanip>

// ======================================================
// GAME / GRID SETTINGS
// ======================================================
// Window size: 1440x1080
// Cell size: 40x40
// Grid size: 36x27
//
// 1440 / 40 = 36 cells
// 1080 / 40 = 27 cells
// ======================================================
// SPRITE SHEET SETTINGS
// ======================================================
// File: snake-graphics.png
// Texture size: 320x256
// Grid: 5 columns x 4 rows
// Source tile size: 64x64
// Tile coordinates: (col, row)
// ======================================================
//
// HEAD
// (3,0) = up
// (4,0) = right
// (4,1) = down
// (3,1) = left
//
// TAIL
// (4,3) = up
// (3,3) = right
// (3,2) = down
// (4,2) = left
//
// BODY
// (1,0) = horizontal
// (2,1) = vertical
//
// CORNERS
// (0,0) = turn right + down
// (2,0) = turn left + down
// (0,1) = turn right + up
// (2,2) = turn left + up
//
// FOOD
// (0,3) = apple
// ======================================================

sf::IntRect Game::tileRect(int col, int row)
{
    return sf::IntRect(col * TILE, row * TILE, TILE, TILE);
}

Game::Game()
    : window(sf::VideoMode(windowWidth, windowHeight), "Snake"),
    distX(0, gridWidth - 1),
    distY(0, gridHeight - 1)
{
    loadAssets();
    setupUi();
    setupOverlay();

    resetGame();
    state = GameState::Menu;
}

void Game::run()
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

// ======================================================
// INIT
// ======================================================

void Game::loadAssets()
{
    // FONT
    if (!font.loadFromFile("Pixel_Font-7.ttf"))
        ok = false;

    // TEXTURES / SPRITES
    if (!snakeTexture.loadFromFile("snake-graphics.png"))
        ok = false;

    snakeTexture.setSmooth(false);
    snakeSprite.setTexture(snakeTexture);

    float snakeScale = static_cast<float>(cellSize) / static_cast<float>(TILE);
    snakeSprite.setScale(snakeScale, snakeScale);

    float lemonScale = static_cast<float>(cellSize) / 512.f * 1.7f;
    lemonSprite.setScale(lemonScale, lemonScale);

    // BACKGROUNDS
    if (!bgMenuTexture.loadFromFile("menuMap.png"))
        ok = false;
    bgMenuSprite.setTexture(bgMenuTexture);

    if (!bgGameTexture.loadFromFile("map.png"))
        ok = false;
    bgGameSprite.setTexture(bgGameTexture);

    if (!lemonTexture.loadFromFile("lemon.png"))
        ok = false;
    lemonSprite.setTexture(lemonTexture);

    // SOUNDS
    if (!gameOverBuffer.loadFromFile("hit.wav"))
        ok = false;
    gameOverSound.setBuffer(gameOverBuffer);

    if (!eatingFoodBuffer.loadFromFile("eat.wav"))
        ok = false;
    eatingFoodSound.setBuffer(eatingFoodBuffer);

    if (!pauseBuffer.loadFromFile("pause.wav"))
        ok = false;
    pauseSound.setBuffer(pauseBuffer);

    if (!bonusBuffer.loadFromFile("bonus.wav"))
        ok = false;
    bonusSound.setBuffer(bonusBuffer);

    if (!bonusOnBuffer.loadFromFile("bonusOn.wav"))
        ok = false;
    bonusOnSound.setBuffer(bonusOnBuffer);

    // MUSIC
    if (!menuMusic.openFromFile("menu.ogg"))
        ok = false;
    menuMusic.setLoop(true);
    menuMusic.setVolume(40.f);
    menuMusic.play();

    if (!playMusic.openFromFile("play.ogg"))
        ok = false;
    playMusic.setLoop(true);
    playMusic.setVolume(40.f);
}

void Game::setupUi()
{
    LemonBonusTitle = sf::Text("Bonus timer: 5.00", font, 34);
    LemonBonusTitle.setFillColor(sf::Color::White);
    LemonBonusTitle.setPosition(550.f, 15.f);

    menuTitle = sf::Text("SNAKE", font, 110);
    menuTitle.setFillColor(sf::Color::White);

    menuHint = sf::Text("Press ENTER to start", font, 52);
    menuHint.setFillColor(sf::Color::White);

    gameOverTitle = sf::Text("Game Over", font, 100);
    gameOverTitle.setFillColor(sf::Color::White);

    gameOverHint = sf::Text("Press R to restart", font, 46);
    gameOverHint.setFillColor(sf::Color::White);

    pauseTitle = sf::Text("PAUSE", font, 95);
    pauseTitle.setFillColor(sf::Color::White);

    pauseHint = sf::Text("Press P to continue", font, 42);
    pauseHint.setFillColor(sf::Color::White);

    scoreText = sf::Text("Score: 0", font, 34);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20.f, 15.f);

    pointsRecordText = sf::Text("Points Record: 0", font, 34);
    pointsRecordText.setFillColor(sf::Color::White);
    pointsRecordText.setPosition(1100.f, 15.f);

    muteHintText = sf::Text("M: Sound ON", font, 24);
    muteHintText.setFillColor(sf::Color::White);
    muteHintText.setPosition(20.f, 55.f);
}

void Game::setupOverlay()
{
    overlay.setSize(sf::Vector2f(1440.f, 1080.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
}

// ======================================================
// UTILS
// ======================================================

void Game::centerText(sf::Text& t, float cx, float cy)
{
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(cx, cy);
}

bool Game::isOnSnake(const sf::Vector2i& p) const
{
    for (const auto& s : snake)
    {
        if (p == s)
            return true;
    }
    return false;
}

sf::Vector2i Game::foodSpawn()
{
    sf::Vector2i f;
    do
    {
        f = { distX(gen), distY(gen) };
    } while (isOnSnake(f));

    return f;
}

void Game::updateMuteText()
{
    muteHintText.setString(musicMuted ? "M: Sound OFF" : "M: Sound ON");
}

void Game::updateRecordText()
{
    pointsRecordText.setString("Points Record: " + std::to_string(pointsRecordValue));
}

void Game::resetGame()
{
    dir = { 1, 0 };
    nextDir = dir;
    applesSinceBonus = 0;
    directionChangedThisStep = false;

    snake.clear();

    int startX = gridWidth / 2;
    int startY = gridHeight / 2;

    snake.push_back({ startX, startY });
    snake.push_back({ startX - 1, startY });
    snake.push_back({ startX - 2, startY });
    snake.push_back({ startX - 3, startY });

    score = 0;
    timer = 0.f;
    food = foodSpawn();
    lemon = foodSpawn();
    
    scoreText.setString("Score: 0");
    LemonBonusTitle.setString("Bonus timer: 5.00");
    updateRecordText();
    updateMuteText();
}

// ======================================================
// EVENTS / INPUT
// ======================================================

void Game::events()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

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
                playMusic.play();
                state = GameState::Playing;
            }
            else if (event.key.code == sf::Keyboard::Escape)
            {
                state = GameState::Menu;
                playMusic.stop();
                menuMusic.play();
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

                float volume = musicMuted ? 0.f : 40.f;
                menuMusic.setVolume(volume);
                playMusic.setVolume(volume);

                updateMuteText();
            }
            else if (state == GameState::Playing && !directionChangedThisStep)
            {
                if (event.key.code == sf::Keyboard::Left && dir.x != 1)
                {
                    nextDir = { -1, 0 };
                    directionChangedThisStep = true;
                }
                else if (event.key.code == sf::Keyboard::Right && dir.x != -1)
                {
                    nextDir = { 1, 0 };
                    directionChangedThisStep = true;
                }
                else if (event.key.code == sf::Keyboard::Up && dir.y != 1)
                {
                    nextDir = { 0, -1 };
                    directionChangedThisStep = true;
                }
                else if (event.key.code == sf::Keyboard::Down && dir.y != -1)
                {
                    nextDir = { 0, 1 };
                    directionChangedThisStep = true;
                }
            }
        }
    }
}
// ======================================================
// UPDATE
// ======================================================

void Game::update(float dt)
{
    if (state != GameState::Playing)
        return;

    timer += dt;
    lemonAnimTimer += dt;

    if (lemonActive)
    {
        lemonLifeTimer += dt;

        float timeLeft = 5.f - lemonLifeTimer;
        if (timeLeft < 0.f)
            timeLeft = 0.f;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << timeLeft;

        LemonBonusTitle.setString("Bonus timer: " + ss.str());

        if (lemonLifeTimer >= 5.f)
        {
            bonusTimer = false;
            lemonActive = false;
            lemonLifeTimer = 0.f;
        }
    }

    if (lemonAnimTimer > 0.25f)
    {
        lemonFrame++;
        lemonFrame %= 2;

        lemonSprite.setTextureRect(
            sf::IntRect(
                0,
                lemonFrame * 512,
                512,
                512
            )
        );

        lemonAnimTimer = 0.f;
    }

    while (timer >= moveDelay)
    {
        timer -= moveDelay;

        dir = nextDir;
        directionChangedThisStep = false;

        sf::Vector2i head = snake.front();
        sf::Vector2i newHead = head + dir;

        if (newHead.x < 0 || newHead.y < 0 ||
            newHead.x >= gridWidth || newHead.y >= gridHeight)
        {
            gameOverSound.play();
            playMusic.stop();
            state = GameState::GameOver;
            break;
        }

        bool willGrow = (newHead == food);
        bool willGrowBonus = (newHead == lemon);
        bool hitSelf = isOnSnake(newHead) && !(newHead == snake.back() && !willGrow);

        if (hitSelf)
        {
            gameOverSound.play();
            playMusic.stop();
            state = GameState::GameOver;
            break;
        }

        snake.push_front(newHead);

        if (willGrowBonus)
        {
            bonusOnSound.play();
            lemon = foodSpawn();
            lemonActive = false;
            score+=3;
            bonusTimer = false;

            if (score > pointsRecordValue)
            {
                pointsRecordValue = score;
                updateRecordText();
            }

            scoreText.setString("Score: " + std::to_string(score));
        }

        if (willGrow)
        {
            eatingFoodSound.play();
            food = foodSpawn();
            score++;
            applesSinceBonus++;


            if (score > pointsRecordValue)
            {
                pointsRecordValue = score;
                updateRecordText();
            }
            if (!lemonActive && applesSinceBonus >= 5)
            {
                bonusSound.play();
                lemon = foodSpawn();

                while (lemon == food)
                {
                    lemon = foodSpawn();
                }

                bonusTimer = true;
                lemonActive = true;
                lemonLifeTimer = 0.f;

                applesSinceBonus = 0;
            }
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

void Game::drawTile(int col, int row, int gx, int gy)
{
    snakeSprite.setTextureRect(tileRect(col, row));
    snakeSprite.setOrigin(0.f, 0.f);
    snakeSprite.setRotation(0.f);
    snakeSprite.setPosition(
        static_cast<float>(gx * cellSize),
        static_cast<float>(gy * cellSize)
    );
    window.draw(snakeSprite);
}

void Game::drawHead(int gx, int gy, sf::Vector2i d)
{
    if (d.y == -1) drawTile(3, 0, gx, gy);
    else if (d.x == 1) drawTile(4, 0, gx, gy);
    else if (d.y == 1) drawTile(4, 1, gx, gy);
    else drawTile(3, 1, gx, gy);
}

void Game::drawTail(int gx, int gy, sf::Vector2i d)
{
    if (d.y == -1) drawTile(4, 3, gx, gy);
    else if (d.x == 1) drawTile(3, 3, gx, gy);
    else if (d.y == 1) drawTile(3, 2, gx, gy);
    else drawTile(4, 2, gx, gy);
}

void Game::drawBody(int gx, int gy, sf::Vector2i prev, sf::Vector2i cur, sf::Vector2i next)
{
    bool hasLeft = (prev.x == cur.x - 1 || next.x == cur.x - 1);
    bool hasRight = (prev.x == cur.x + 1 || next.x == cur.x + 1);
    bool hasUp = (prev.y == cur.y - 1 || next.y == cur.y - 1);
    bool hasDown = (prev.y == cur.y + 1 || next.y == cur.y + 1);

    if (hasLeft && hasRight)
    {
        drawTile(1, 0, gx, gy);
        return;
    }

    if (hasUp && hasDown)
    {
        drawTile(2, 1, gx, gy);
        return;
    }

    if (hasRight && hasDown)
    {
        drawTile(0, 0, gx, gy);
        return;
    }

    if (hasLeft && hasDown)
    {
        drawTile(2, 0, gx, gy);
        return;
    }

    if (hasRight && hasUp)
    {
        drawTile(0, 1, gx, gy);
        return;
    }

    if (hasLeft && hasUp)
    {
        drawTile(2, 2, gx, gy);
        return;
    }

    drawTile(1, 0, gx, gy);
}

// ======================================================
// RENDER
// ======================================================

void Game::render()
{
    window.clear();

    if (state == GameState::Menu)
        window.draw(bgMenuSprite);
    else
        window.draw(bgGameSprite);

    if (state == GameState::Playing || state == GameState::GameOver || state == GameState::Paused)
    {
        drawTile(0, 3, food.x, food.y);

        int n = static_cast<int>(snake.size());

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
        if (lemonActive)
        {
            lemonSprite.setPosition(
                lemon.x * cellSize,
                lemon.y * cellSize
            );

            window.draw(lemonSprite);
        }
        if (bonusTimer)
        {
            window.draw(LemonBonusTitle);
        }
        window.draw(scoreText);
        window.draw(pointsRecordText);
        window.draw(muteHintText);
        
    }

    if (state == GameState::Menu)
    {
        window.draw(overlay);
        centerText(menuTitle, 720.f, 450.f);
        centerText(menuHint, 720.f, 550.f);
        window.draw(menuTitle);
        window.draw(menuHint);
    }

    if (state == GameState::GameOver)
    {
        window.draw(overlay);
        centerText(gameOverTitle, 720.f, 450.f);
        centerText(gameOverHint, 720.f, 550.f);
        window.draw(gameOverTitle);
        window.draw(gameOverHint);
    }

    if (state == GameState::Paused)
    {
        window.draw(overlay);
        centerText(pauseTitle, 720.f, 450.f);
        centerText(pauseHint, 720.f, 550.f);
        window.draw(pauseTitle);
        window.draw(pauseHint);
    }

    window.display();
}