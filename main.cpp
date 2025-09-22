#include "gomoku/core/Logger.hpp"
#include "gui/GameWindow.hpp"

int main()
{
    // Initialize logger for the game
    auto& logger = gomoku::Logger::getInstance();
    logger.enableConsoleLogging(true);
    logger.enableColoredOutput(true);
    logger.enableFileLogging("logs/gomoku.log");

#ifdef DEBUG
    logger.setLogLevel(gomoku::LogLevel::DEBUG);
    LOG_DEBUG("Gomoku started in DEBUG mode");
#else
    logger.setLogLevel(gomoku::LogLevel::INFO);
#endif

    LOG_INFO("=== GOMOKU GAME STARTED ===");
    LOG_INFO("Logger initialized successfully");

    gomoku::gui::GameWindow gameWindow;
    gameWindow.run();

    LOG_INFO("Game session ended");
    return 0;
}