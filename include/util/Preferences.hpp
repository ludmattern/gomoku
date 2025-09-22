#pragma once
#include <string>

namespace gomoku::util {

struct PreferencesData {
    bool sfxEnabled = true;
    bool musicEnabled = true;
    std::string theme = "default";
};

class Preferences {
public:
    static std::string configFilePath();
    static bool load(PreferencesData& outPrefs);
    static bool save(const PreferencesData& prefs);
};

} // namespace gomoku::util


