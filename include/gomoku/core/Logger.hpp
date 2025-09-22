// gomoku/core/Logger.hpp
#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace gomoku {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    NONE = 4
};

class Logger {
public:
    static Logger& getInstance();

    // Configuration
    void setLogLevel(LogLevel level);
    void enableFileLogging(const std::string& filename);
    void enableConsoleLogging(bool enable = true);
    void enableColoredOutput(bool enable = true);

    // Logging methods
    void log(LogLevel level, const std::string& message,
        const char* file = nullptr, int line = 0, const char* func = nullptr);

    template <typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args)
    {
        std::ostringstream oss;
        formatMessage(oss, format, std::forward<Args>(args)...);
        log(level, oss.str());
    }

    // Convenience methods
    void debug(const std::string& message, const char* file = nullptr, int line = 0, const char* func = nullptr);
    void info(const std::string& message, const char* file = nullptr, int line = 0, const char* func = nullptr);
    void warning(const std::string& message, const char* file = nullptr, int line = 0, const char* func = nullptr);
    void error(const std::string& message, const char* file = nullptr, int line = 0, const char* func = nullptr);

    template <typename... Args>
    void debug(const std::string& format, Args&&... args)
    {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(const std::string& format, Args&&... args)
    {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(const std::string& format, Args&&... args)
    {
        log(LogLevel::WARNING, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(const std::string& format, Args&&... args)
    {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getCurrentTime() const;
    std::string levelToString(LogLevel level) const;
    std::string getColorCode(LogLevel level) const;
    std::string getResetColor() const;
    std::string extractFileName(const char* filePath) const;

    template <typename T>
    void formatMessage(std::ostringstream& oss, const std::string& format, T&& value)
    {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            oss << format.substr(0, pos) << std::forward<T>(value) << format.substr(pos + 2);
        } else {
            oss << format << " " << std::forward<T>(value);
        }
    }

    template <typename T, typename... Args>
    void formatMessage(std::ostringstream& oss, const std::string& format, T&& value, Args&&... args)
    {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            std::string newFormat = format.substr(0, pos) + std::to_string(value) + format.substr(pos + 2);
            formatMessage(oss, newFormat, std::forward<Args>(args)...);
        } else {
            oss << format << " " << std::forward<T>(value);
            formatMessage(oss, " {}", std::forward<Args>(args)...);
        }
    }

    void formatMessage(std::ostringstream& oss, const std::string& format)
    {
        oss << format;
    }

    LogLevel currentLevel_ = LogLevel::INFO;
    bool consoleLogging_ = true;
    bool coloredOutput_ = true;
    bool fileLogging_ = false;
    std::unique_ptr<std::ofstream> logFile_;
    std::mutex mutex_;
};

} // namespace gomoku

// Convenience macros for easy logging
#define LOG_DEBUG(msg) gomoku::Logger::getInstance().debug(msg, __FILE__, __LINE__, __func__)
#define LOG_INFO(msg) gomoku::Logger::getInstance().info(msg, __FILE__, __LINE__, __func__)
#define LOG_WARNING(msg) gomoku::Logger::getInstance().warning(msg, __FILE__, __LINE__, __func__)
#define LOG_ERROR(msg) gomoku::Logger::getInstance().error(msg, __FILE__, __LINE__, __func__)
