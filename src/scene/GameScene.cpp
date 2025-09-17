#include "scene/GameScene.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace gomoku::scene {

GameScene::GameScene(Context& context, bool vsAi)
    : AScene(context)
    , vsAi_(vsAi)
    , gameSession_(gomoku::SessionController())
{
    // Initialisation du bouton Back
    backButton_.setPosition({ 100, 820 });
    backButton_.setSize({ 300, 70 });
    if (context_.resourceManager && context_.resourceManager->hasTexture("back_button"))
        backButton_.setTexture(&context_.resourceManager->getTexture("back_button"));
    backButton_.setScale(1.0f);
    backButton_.setCallback([this]() { onBackClicked(); });

    // Initialisation du renderer de plateau
    if (context_.resourceManager) {
        boardRenderer_.setTextures(
            context_.resourceManager->getTexture("board"),
            context_.resourceManager->getTexture("pawn1"),
            context_.resourceManager->getTexture("pawn2"));
    }

    rules_ = gomoku::RuleSet();

    // Configure controllers according to mode
    if (vsAi_) {
        // Default SessionController ctor is Black:Human, White:AI; keep as is for now
        gameSession_.setController(gomoku::Player::Black, gomoku::Controller::Human);
        gameSession_.setController(gomoku::Player::White, gomoku::Controller::AI);
    } else {
        gameSession_.setController(gomoku::Player::Black, gomoku::Controller::Human);
        gameSession_.setController(gomoku::Player::White, gomoku::Controller::Human);
    }

    // Initial board binding (renderer now reads directly from IBoardView)
    {
        auto snap = gameSession_.snapshot();
        const_cast<gomoku::gui::GameBoardRenderer&>(boardRenderer_).setBoardView(snap.view);
    }

    // HUD setup (lazy font load)
    fontOk_ = font_.loadFromFile("assets/ui/DejaVuSans.ttf");
    if (fontOk_) {
        hudText_.setFont(font_);
        hudText_.setCharacterSize(20);
        hudText_.setFillColor(sf::Color::White);
        hudText_.setPosition(20.f, 20.f);
        msgText_.setFont(font_);
        msgText_.setCharacterSize(20);
        msgText_.setFillColor(sf::Color(255, 80, 80));
        msgText_.setPosition(20.f, 48.f);
    }
}

GameScene::~GameScene() = default;

void GameScene::onThemeChanged()
{
    if (!context_.resourceManager)
        return;
    // Rebind textures sur le renderer
    const_cast<gomoku::gui::GameBoardRenderer&>(boardRenderer_).setTextures(
        context_.resourceManager->getTexture("board"),
        context_.resourceManager->getTexture("pawn1"),
        context_.resourceManager->getTexture("pawn2"));
}

bool GameScene::handleInput(sf::Event& event)
{
    if (context_.window && backButton_.handleInput(event, *context_.window))
        return true;

    // Prévisualisation temporairement désactivée pour debug

    // Placement des pions sur clic souris
    if (context_.window && event.type == sf::Event::MouseButtonPressed) {
        auto btn = event.mouseButton.button;
        if (btn == sf::Mouse::Left || btn == sf::Mouse::Right) {
            // Allow UI clicks, but ignore board placement while AI is thinking or during cooldown
            if (aiThinking_ || pendingAi_ || (blockBoardClicksUntil_ > sf::Time::Zero && inputClock_.getElapsedTime() < blockBoardClicksUntil_)) {
                return true; // consume to avoid buffering a move
            }
            const auto size = context_.window->getSize();
            const float centerX = static_cast<float>(size.x) * 0.5f;
            const float centerY = static_cast<float>(size.y) * 0.5f;

            const int N = 19;
            const int C = (N - 1) / 2;

            const float tileW = std::min(static_cast<float>(size.x) * 0.8f / 18.f,
                static_cast<float>(size.y) * 0.8f * 2.f / 18.f);
            const float tileH = tileW * 0.5f;

            sf::Vector2f mp = context_.window->mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
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
            // 		const_cast<GameBoardRenderer&>(boardRenderer_).updateCell(i, j, CellState::Player1);
            // 	else if (btn == sf::Mouse::Right)
            // 		const_cast<GameBoardRenderer&>(boardRenderer_).updateCell(i, j, CellState::Player2);
            // }
            if ((dx * dx + dy * dy) <= (maxDist * maxDist)) {
                if (btn == sf::Mouse::Left) {
                    // If it's human's turn, try to play
                    auto before = gameSession_.snapshot();
                    if (gameSession_.controller(before.toPlay) == gomoku::Controller::Human) {
                        auto result = gameSession_.playHuman(gomoku::Pos { (uint8_t)i, (uint8_t)j });
                        if (result.ok) {
                            auto snap1 = gameSession_.snapshot();
                            const_cast<gomoku::gui::GameBoardRenderer&>(boardRenderer_).setBoardView(snap1.view);
                            // SFX: pose de pion selon couleur jouée
                            playSfx(snap1.toPlay == gomoku::Player::Black ? "place_stone_white" : "place_stone_black", 70.f);
                            // Capture détectée ? compare les paires capturées avant/après
                            if (snap1.captures.first > before.captures.first || snap1.captures.second > before.captures.second) {
                                playSfx("capture", 80.f);
                            }
                            // Check end of game
                            if (snap1.status != gomoku::GameStatus::Ongoing)
                                return true;
                            // If vs AI and AI to play, schedule it with a one-frame delay
                            if (vsAi_ && gameSession_.controller(snap1.toPlay) == gomoku::Controller::AI) {
                                pendingAi_ = true;
                                framePresented_ = false; // wait for next render() before starting AI
                            }
                        } else {
                            // Show reason why illegal
                            illegalMsg_ = result.why;
                            if (fontOk_) {
                                msgText_.setString(illegalMsg_);
                                illegalClock_.restart();
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
    backButton_.update(deltaTime);

    // Run pending AI move only after at least one frame has been presented
    if (pendingAi_ && framePresented_) {
        pendingAi_ = false;
        aiThinking_ = true;
        auto before = gameSession_.snapshot();
        auto t0 = std::chrono::steady_clock::now();
        auto aiResult = gameSession_.playAI(aiBudgetMs_);
        auto t1 = std::chrono::steady_clock::now();
        lastAiMs_ = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        auto snap = gameSession_.snapshot();
        const_cast<gomoku::gui::GameBoardRenderer&>(boardRenderer_).setBoardView(snap.view);
        // Capture détectée côté IA ?
        if (snap.captures.first > before.captures.first || snap.captures.second > before.captures.second) {
            playSfx("capture", 80.f);
        }
        aiThinking_ = false;
        // Start short cooldown to swallow any clicks pressed during AI thinking
        inputClock_.restart();
        blockBoardClicksUntil_ = sf::milliseconds(120);
    }
}

void GameScene::render(sf::RenderTarget& target) const
{
    // Fond de jeu
    if (context_.resourceManager && context_.resourceManager->hasTexture("gameBackground")) {
        sf::Sprite bg(context_.resourceManager->getTexture("gameBackground"));
        bg.setScale(sf::Vector2f(1.0f, 1.0f));
        target.draw(bg);
    }
    // Plateau (cible est la fenêtre; cast suffisant ici)
    const_cast<gomoku::gui::GameBoardRenderer&>(boardRenderer_).render(static_cast<sf::RenderWindow&>(target));
    // UI
    backButton_.render(target);

    // HUD: toPlay, captures, last move, AI time
    auto snap = gameSession_.snapshot();
    if (fontOk_) {
        char buf[128];
        auto caps = snap.captures;
        std::snprintf(buf, sizeof(buf), "To play: %s   Captures ●:%d ○:%d%s%s%s",
            (snap.toPlay == gomoku::Player::Black ? "● Black" : "○ White"),
            caps.first, caps.second,
            (lastAiMs_ >= 0 ? "   |  AI:" : ""),
            (lastAiMs_ >= 0 ? " ms" : ""),
            "");
        std::string line(buf);
        if (snap.lastMove) {
            line += "   |  Last: ";
            line += std::to_string((int)snap.lastMove->x);
            line += ",";
            line += std::to_string((int)snap.lastMove->y);
        }
        if (lastAiMs_ >= 0) {
            line += "   |  AI time: ";
            line += std::to_string(lastAiMs_);
            line += "ms";
        }
        hudText_.setString(line);
        target.draw(hudText_);
    }

    // Illegal message (timed)
    if (fontOk_ && !illegalMsg_.empty() && illegalClock_.getElapsedTime().asSeconds() < 2.0f) {
        target.draw(msgText_);
    }

    // Endgame banner
    if (snap.status != gomoku::GameStatus::Ongoing && fontOk_) {
        sf::Text endTxt;
        endTxt.setFont(font_);
        endTxt.setCharacterSize(36);
        endTxt.setFillColor(sf::Color::Yellow);
        std::string msg = (snap.status == gomoku::GameStatus::Draw) ? "Draw" : "Victory";
        endTxt.setString(msg);
        endTxt.setPosition(20.f, 50.f);
        target.draw(endTxt);
    }

    // Mark that at least one frame has been presented; allows AI to start next update
    framePresented_ = true;
}

void GameScene::onBackClicked()
{
    context_.inGame = false;
    context_.showMainMenu = true;
	playMusic("assets/audio/menu_theme.ogg", true, 10.f);
}

} // namespace gomoku::scene