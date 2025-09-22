#include "util/Preferences.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace gomoku::util {

static bool parseBool(const std::string& s, bool def) {
    if (s == "true" || s == "1") return true;
    if (s == "false" || s == "0") return false;
    return def;
}

std::string Preferences::configFilePath()
{
    const char* home = std::getenv("HOME");
    std::string base = home ? std::string(home) : std::string(".");
    fs::path dir = fs::path(base) / ".config" / "gomoku";
    fs::create_directories(dir);
    return (dir / "preferences.json").string();
}

bool Preferences::load(PreferencesData& outPrefs)
{
    std::ifstream in(configFilePath());
    if (!in.is_open())
        return false;
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string json = buffer.str();
    // Parsing JSON minimaliste (sans dépendance), on cherche des clés simples
    auto findString = [&](const char* key, std::string def) {
        std::string k = std::string("\"") + key + "\"";
        auto pos = json.find(k);
        if (pos == std::string::npos) return def;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return def;
        pos = json.find('"', pos);
        if (pos == std::string::npos) return def;
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return def;
        return json.substr(pos + 1, end - pos - 1);
    };
    auto findBool = [&](const char* key, bool def) {
        std::string k = std::string("\"") + key + "\"";
        auto pos = json.find(k);
        if (pos == std::string::npos) return def;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return def;
        auto start = json.find_first_not_of(" \t\n\r", pos + 1);
        if (start == std::string::npos) return def;
        auto end = json.find_first_of(",}\n\r\t ", start + 1);
        if (end == std::string::npos) end = json.size();
        return parseBool(json.substr(start, end - start), def);
    };

    outPrefs.theme = findString("theme", outPrefs.theme);
    outPrefs.sfxEnabled = findBool("sfxEnabled", outPrefs.sfxEnabled);
    outPrefs.musicEnabled = findBool("musicEnabled", outPrefs.musicEnabled);
    return true;
}

bool Preferences::save(const PreferencesData& prefs)
{
    std::ofstream out(configFilePath(), std::ios::trunc);
    if (!out.is_open())
        return false;
    out << "{\n";
    out << "  \"theme\": \"" << prefs.theme << "\",\n";
    out << "  \"sfxEnabled\": " << (prefs.sfxEnabled ? "true" : "false") << ",\n";
    out << "  \"musicEnabled\": " << (prefs.musicEnabled ? "true" : "false") << "\n";
    out << "}\n";
    return true;
}

} // namespace gomoku::util


