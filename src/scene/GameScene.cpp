#include "scene/GameScene.hpp"
#include "scene/MainMenu.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

GameScene::GameScene(Context& context, bool vsAi)
    : AScene(context)
    , _vsAi(vsAi)
    , _gameSession(gomoku::GameSession())
{
    // Initialisation du bouton Back
    _backButton.setPosition({ 100, 820 });
    _backButton.setSize({ 300, 70 });
    if (_context.ressourceManager && _context.ressourceManager->hasTexture("back_button"))
        _backButton.setTexture(&_context.ressourceManager->getTexture("back_button"));
    _backButton.setScale(1.0f);
    _backButton.setCallback([this]() { onBackClicked(); });

    // Initialisation du renderer de plateau
    if (_context.ressourceManager) {
        _boardRenderer.setTextures(
            _context.ressourceManager->getTexture("board"),
            _context.ressourceManager->getTexture("pawn1"),
            _context.ressourceManager->getTexture("pawn2"));
    }

    _rules = gomoku::RuleSet();

    // Configure controllers according to mode
    if (_vsAi) {
        // Default GameSession ctor is Black:Human, White:AI; keep as is for now
        _gameSession.setController(gomoku::Player::Black, gomoku::Controller::Human);
        _gameSession.setController(gomoku::Player::White, gomoku::Controller::AI);
    } else {
        _gameSession.setController(gomoku::Player::Black, gomoku::Controller::Human);
        _gameSession.setController(gomoku::Player::White, gomoku::Controller::Human);
    }

    // Initial board sync
    auto snap = _gameSession.snapshot();
    const_cast<GameBoardRenderer&>(_boardRenderer).applyBoard(*snap.view);

    // HUD setup (lazy font load)
    _fontOk = _font.loadFromFile("assets/ui/DejaVuSans.ttf");
    if (_fontOk) {
        _hudText.setFont(_font);
        _hudText.setCharacterSize(20);
        _hudText.setFillColor(sf::Color::White);
        _hudText.setPosition(20.f, 20.f);
        _msgText.setFont(_font);
        _msgText.setCharacterSize(20);
        _msgText.setFillColor(sf::Color(255, 80, 80));
        _msgText.setPosition(20.f, 48.f);
    }
}

GameScene::~GameScene(void)
{
}

bool GameScene::handleInput(sf::Event& event)
{
    if (_context.window && _backButton.handleInput(event, *_context.window))
        return true;

    // Prévisualisation temporairement désactivée pour debug

    // Placement des pions sur clic souris
    if (_context.window && event.type == sf::Event::MouseButtonPressed) {
        auto btn = event.mouseButton.button;
        if (btn == sf::Mouse::Left || btn == sf::Mouse::Right) {
            // Allow UI clicks, but ignore board placement while AI is thinking or during cooldown
            if (_aiThinking || _pendingAi || (_blockBoardClicksUntil > sf::Time::Zero && _inputClock.getElapsedTime() < _blockBoardClicksUntil)) {
                return true; // consume to avoid buffering a move
            }
            const auto size = _context.window->getSize();
            const float centerX = static_cast<float>(size.x) * 0.5f;
            const float centerY = static_cast<float>(size.y) * 0.5f;

            const int N = 19;
            const int C = (N - 1) / 2;

            const float tileW = std::min(size.x * 0.8f / 18.f, size.y * 0.8f * 2.f / 18.f);
            const float tileH = tileW * 0.5f;

            sf::Vector2f mp = _context.window->mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            const float X = mp.x - centerX;
            const float Y = mp.y - centerY;

            const float u = (Y / (tileH * 0.5f) + X / (tileW * 0.5f)) * 0.5f;
            const float v = (Y / (tileH * 0.5f) - X / (tileW * 0.5f)) * 0.5f;

            int i = static_cast<int>(std::lround(u)) + C;
            int j = static_cast<int>(std::lround(v)) + C;

            i = std::max(0, std::min(18, i));
            j = std::max(0, std::min(18, j));

            // Validation de proximité
            const float ui = static_cast<float>(i - C);
            const float vj = static_cast<float>(j - C);
            const float snappedX = centerX + (ui - vj) * (tileW * 0.5f);
            const float snappedY = centerY + (ui + vj) * (tileH * 0.5f);
            const float dx = snappedX - mp.x;
            const float dy = snappedY - mp.y;
            const float maxDist = std::min(tileW, tileH) * 0.9f; // Zone cliquable étendue (était 0.35f)

            // if ((dx * dx + dy * dy) <= (maxDist * maxDist))
            // {
            // 	if (btn == sf::Mouse::Left)
            // 		const_cast<GameBoardRenderer&>(_boardRenderer).updateCell(i, j, CellState::Player1);
            // 	else if (btn == sf::Mouse::Right)
            // 		const_cast<GameBoardRenderer&>(_boardRenderer).updateCell(i, j, CellState::Player2);
            // }
            if ((dx * dx + dy * dy) <= (maxDist * maxDist)) {
                if (btn == sf::Mouse::Left) {
                    // If it's human's turn, try to play
                    auto before = _gameSession.snapshot();
                    if (_gameSession.controller(before.toPlay) == gomoku::Controller::Human) {
                        auto result = _gameSession.playHuman(gomoku::Pos { (uint8_t)i, (uint8_t)j });
                        if (result.ok) {
                            auto snap1 = _gameSession.snapshot();
                            const_cast<GameBoardRenderer&>(_boardRenderer).applyBoard(*snap1.view);
                            // Check end of game
                            if (snap1.status != gomoku::GameStatus::Ongoing)
                                return true;
                            // If vs AI and AI to play, schedule it with a one-frame delay
                            if (_vsAi && _gameSession.controller(snap1.toPlay) == gomoku::Controller::AI) {
                                _pendingAi = true;
                                _framePresented = false; // wait for next render() before starting AI
                            }
                        } else {
                            // Show reason why illegal
                            _illegalMsg = result.why;
                            if (_fontOk) {
                                _msgText.setString(_illegalMsg);
                                _illegalClock.restart();
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}

void GameScene::update(sf::Time& deltaTime)
{
    _backButton.update(deltaTime);

    // Run pending AI move only after at least one frame has been presented
    if (_pendingAi && _framePresented) {
        _pendingAi = false;
        _aiThinking = true;
        auto t0 = std::chrono::steady_clock::now();
        auto aiResult = _gameSession.playAI(_aiBudgetMs);
        auto t1 = std::chrono::steady_clock::now();
        _lastAiMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        auto snap = _gameSession.snapshot();
        const_cast<GameBoardRenderer&>(_boardRenderer).applyBoard(*snap.view);
        _aiThinking = false;
        // Start short cooldown to swallow any clicks pressed during AI thinking
        _inputClock.restart();
        _blockBoardClicksUntil = sf::milliseconds(120);
    }
}

void GameScene::render(sf::RenderTarget& target) const
{
    // Fond de jeu
    if (_context.ressourceManager && _context.ressourceManager->hasTexture("gameBackground")) {
        sf::Sprite bg(_context.ressourceManager->getTexture("gameBackground"));
        bg.setScale(sf::Vector2f(1.0f, 1.0f));
        target.draw(bg);
    }
    // Plateau (cible est la fenêtre; cast suffisant ici)
    const_cast<GameBoardRenderer&>(_boardRenderer).render(static_cast<sf::RenderWindow&>(target));
    // UI
    _backButton.render(target);

    // HUD: toPlay, captures, last move, AI time
    auto snap = _gameSession.snapshot();
    if (_fontOk) {
        char buf[128];
        auto caps = snap.captures;
        std::snprintf(buf, sizeof(buf), "To play: %s   Captures ●:%d ○:%d%s%s%s",
            (snap.toPlay == gomoku::Player::Black ? "● Black" : "○ White"),
            caps.first, caps.second,
            (_lastAiMs >= 0 ? "   |  AI:" : ""),
            (_lastAiMs >= 0 ? " ms" : ""),
            "");
        std::string line(buf);
        if (snap.lastMove) {
            line += "   |  Last: ";
            line += std::to_string((int)snap.lastMove->x);
            line += ",";
            line += std::to_string((int)snap.lastMove->y);
        }
        if (_lastAiMs >= 0) {
            line += "   |  AI time: ";
            line += std::to_string(_lastAiMs);
            line += "ms";
        }
        _hudText.setString(line);
        target.draw(_hudText);
    }

    // Illegal message (timed)
    if (_fontOk && !_illegalMsg.empty() && _illegalClock.getElapsedTime().asSeconds() < 2.0f) {
        target.draw(_msgText);
    }

    // Endgame banner
    if (snap.status != gomoku::GameStatus::Ongoing && _fontOk) {
        sf::Text endTxt;
        endTxt.setFont(_font);
        endTxt.setCharacterSize(36);
        endTxt.setFillColor(sf::Color::Yellow);
        std::string msg = (snap.status == gomoku::GameStatus::Draw) ? "Draw" : "Victory";
        endTxt.setString(msg);
        endTxt.setPosition(20.f, 50.f);
        target.draw(endTxt);
    }

    // Mark that at least one frame has been presented; allows AI to start next update
    _framePresented = true;
}

void GameScene::onBackClicked(void)
{
    _context.inGame = false;
    _context.showMainMenu = true;
}