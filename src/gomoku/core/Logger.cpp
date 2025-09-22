// gomoku/core/Logger.cpp
#include "gomoku/core/Logger.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace gomoku {

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::~Logger()
{
    if (logFile_ && logFile_->is_open()) {
        logFile_->close();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    currentLevel_ = level;
}

void Logger::enableFileLogging(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_ && logFile_->is_open()) {
        logFile_->close();
    }

    // Extract directory from filename and create it if needed
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string directory = filename.substr(0, lastSlash);

        // Try to create the directory
        std::string command = "mkdir -p " + directory + " 2>/dev/null";
        int result = system(command.c_str());

        if (result != 0) {
            std::cerr << "[LOGGER WARNING] Could not create directory '" << directory
                      << "' - file logging may fail" << std::endl;
        }
    }

    // Try to open the file
    logFile_ = std::make_unique<std::ofstream>(filename, std::ios::app);
    fileLogging_ = logFile_->is_open();

    if (!fileLogging_) {
        std::cerr << "[LOGGER ERROR] Failed to open log file '" << filename
                  << "' - logging to console only" << std::endl;
        logFile_.reset();
    } else {
        std::cout << "[LOGGER INFO] Log file opened: " << filename << std::endl;
    }
}

void Logger::enableConsoleLogging(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    consoleLogging_ = enable;
}

void Logger::enableColoredOutput(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    coloredOutput_ = enable;
}

void Logger::log(LogLevel level, const std::string& message,
    const char* file, int line, const char* func)
{
    if (level < currentLevel_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream logEntry;
    logEntry << "[" << getCurrentTime() << "] ";

    if (coloredOutput_ && consoleLogging_) {
        logEntry << getColorCode(level);
    }

    logEntry << "[" << levelToString(level) << "] ";

    if (file && line > 0 && func) {
        logEntry << "[" << extractFileName(file) << ":" << line << " " << func << "()";
        logEntry << "] ";
    }

    logEntry << message;

    if (coloredOutput_ && consoleLogging_) {
        logEntry << getResetColor();
    }

    std::string finalMessage = logEntry.str();

    // Console output
    if (consoleLogging_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << finalMessage << std::endl;
        } else {
            std::cout << finalMessage << std::endl;
        }
    }

    // File output (without colors)
    if (fileLogging_ && logFile_ && logFile_->is_open()) {
        std::ostringstream fileEntry;
        fileEntry << "[" << getCurrentTime() << "] ";
        fileEntry << "[" << levelToString(level) << "] ";

        if (file && line > 0 && func) {
            fileEntry << "[" << extractFileName(file) << ":" << line << " " << func << "()";
            fileEntry << "] ";
        }

        fileEntry << message << std::endl;
        *logFile_ << fileEntry.str();
        logFile_->flush();
    }
}

void Logger::debug(const std::string& message, const char* file, int line, const char* func)
{
    log(LogLevel::DEBUG, message, file, line, func);
}

void Logger::info(const std::string& message, const char* file, int line, const char* func)
{
    log(LogLevel::INFO, message, file, line, func);
}

void Logger::warning(const std::string& message, const char* file, int line, const char* func)
{
    log(LogLevel::WARNING, message, file, line, func);
}

void Logger::error(const std::string& message, const char* file, int line, const char* func)
{
    log(LogLevel::ERROR, message, file, line, func);
}

std::string Logger::getCurrentTime() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
        % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) const
{
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO ";
    case LogLevel::WARNING:
        return "WARN ";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::NONE:
        return "NONE ";
    default:
        return "UNKN ";
    }
}

std::string Logger::getColorCode(LogLevel level) const
{
    switch (level) {
    case LogLevel::DEBUG:
        return "\033[36m"; // Cyan
    case LogLevel::INFO:
        return "\033[32m"; // Green
    case LogLevel::WARNING:
        return "\033[33m"; // Yellow
    case LogLevel::ERROR:
        return "\033[31m"; // Red
    default:
        return "";
    }
}

std::string Logger::getResetColor() const
{
    return "\033[0m";
}

std::string Logger::extractFileName(const char* filePath) const
{
    if (!filePath)
        return "";

    const char* lastSlash = std::strrchr(filePath, '/');
    if (lastSlash) {
        return std::string(lastSlash + 1);
    }

    const char* lastBackslash = std::strrchr(filePath, '\\');
    if (lastBackslash) {
        return std::string(lastBackslash + 1);
    }

    return std::string(filePath);
}

} // namespace gomoku