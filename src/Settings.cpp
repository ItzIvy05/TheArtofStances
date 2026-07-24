#include "Settings.h"

#include <cctype>
#include <charconv>
#include <fstream>

#include <spdlog/spdlog.h>

namespace Stances
{
    namespace
    {
        constexpr auto INI_PATH = "Data/SKSE/Plugins/Stances.ini"sv;

        [[nodiscard]] std::string Trim(std::string_view a_value)
        {
            const auto first = a_value.find_first_not_of(" \t\r\n");
            if (first == std::string_view::npos) {
                return {};
            }

            const auto last = a_value.find_last_not_of(" \t\r\n");
            return std::string(a_value.substr(first, last - first + 1));
        }

        [[nodiscard]] std::string StripComment(std::string_view a_value)
        {
            auto end = a_value.size();
            for (const auto marker : { ';', '#' }) {
                const auto pos = a_value.find(marker);
                if (pos != std::string_view::npos) {
                    end = std::min(end, pos);
                }
            }

            const auto slash = a_value.find("//");
            if (slash != std::string_view::npos) {
                end = std::min(end, slash);
            }

            return Trim(a_value.substr(0, end));
        }

        [[nodiscard]] bool ParseBool(std::string_view a_value, bool a_default)
        {
            auto value = StripComment(a_value);
            std::ranges::transform(value, value.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });

            if (value == "1" || value == "true" || value == "yes" || value == "on") {
                return true;
            }

            if (value == "0" || value == "false" || value == "no" || value == "off") {
                return false;
            }

            return a_default;
        }

        [[nodiscard]] float ParseFloat(std::string_view a_value, float a_default)
        {
            const auto stripped = StripComment(a_value);
            if (stripped.empty()) {
                return a_default;
            }

            float result{};
            const auto* begin = stripped.data();
            const auto* end = begin + stripped.size();
            if (std::from_chars(begin, end, result).ec != std::errc{}) {
                return a_default;
            }

            return result;
        }

        [[nodiscard]] int ParseInt(std::string_view a_value, int a_default)
        {
            const auto stripped = StripComment(a_value);
            if (stripped.empty()) {
                return a_default;
            }

            int result{};
            const auto* begin = stripped.data();
            const auto* end = begin + stripped.size();
            if (std::from_chars(begin, end, result).ec != std::errc{}) {
                return a_default;
            }

            return result;
        }

        [[nodiscard]] std::string ParseString(std::string_view a_value, const std::string& a_default)
        {
            auto stripped = StripComment(a_value);
            if (stripped.empty()) {
                return a_default;
            }

            return stripped;
        }

        [[nodiscard]] bool SameKey(std::string_view a_lhs, std::string_view a_rhs)
        {
            if (a_lhs.size() != a_rhs.size()) {
                return false;
            }

            for (std::size_t i = 0; i < a_lhs.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(a_lhs[i])) !=
                    std::tolower(static_cast<unsigned char>(a_rhs[i]))) {
                    return false;
                }
            }

            return true;
        }
    }

    Settings* Settings::GetSingleton()
    {
        static Settings singleton;
        return std::addressof(singleton);
    }

    void Settings::Load()
    {
        bool debugLogging = kDefaultDebugLogging;
        float skillUsePerKill = kDefaultSkillUsePerKill;
        bool scaleWithVictimLevel = kDefaultScaleWithVictimLevel;
        float victimLevelMult = kDefaultVictimLevelMult;
        bool applyStanceOnStart = kDefaultApplyStanceOnStart;
        bool showWidget = kDefaultShowWidget;
        int hudX = kDefaultHudX;
        int hudY = kDefaultHudY;
        int hudScale = kDefaultHudScale;
        bool autoHideWidget = kDefaultAutoHideWidget;
        int widgetSeconds = kDefaultWidgetSeconds;
        std::string bearKey = kDefaultBearKey;
        std::string wolfKey = kDefaultWolfKey;
        std::string hawkKey = kDefaultHawkKey;
        std::string neutralKey = kDefaultNeutralKey;
        std::string cycleKey = kDefaultCycleKey;

        std::ifstream file(INI_PATH.data());
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                const auto clean = StripComment(line);
                if (clean.empty() || clean.front() == '[') {
                    continue;
                }

                const auto equals = clean.find('=');
                if (equals == std::string::npos) {
                    continue;
                }

                const auto key = Trim(std::string_view(clean).substr(0, equals));
                const auto value = Trim(std::string_view(clean).substr(equals + 1));

                if (SameKey(key, "fSkillUsePerKill")) {
                    skillUsePerKill = ParseFloat(value, skillUsePerKill);
                } else if (SameKey(key, "bScaleWithVictimLevel")) {
                    scaleWithVictimLevel = ParseBool(value, scaleWithVictimLevel);
                } else if (SameKey(key, "fVictimLevelMult")) {
                    victimLevelMult = ParseFloat(value, victimLevelMult);
                } else if (SameKey(key, "bEnableLogging")) {
                    debugLogging = ParseBool(value, debugLogging);
                } else if (SameKey(key, "bApplyStanceOnStart")) {
                    applyStanceOnStart = ParseBool(value, applyStanceOnStart);
                } else if (SameKey(key, "bShowWidget")) {
                    showWidget = ParseBool(value, showWidget);
                } else if (SameKey(key, "iHudX")) {
                    hudX = ParseInt(value, hudX);
                } else if (SameKey(key, "iHudY")) {
                    hudY = ParseInt(value, hudY);
                } else if (SameKey(key, "iHudScale")) {
                    hudScale = ParseInt(value, hudScale);
                } else if (SameKey(key, "bAutoHideWidget")) {
                    autoHideWidget = ParseBool(value, autoHideWidget);
                } else if (SameKey(key, "iWidgetSeconds")) {
                    widgetSeconds = ParseInt(value, widgetSeconds);
                } else if (SameKey(key, "sBearStanceKey")) {
                    bearKey = ParseString(value, "");
                } else if (SameKey(key, "sWolfStanceKey")) {
                    wolfKey = ParseString(value, "");
                } else if (SameKey(key, "sHawkStanceKey")) {
                    hawkKey = ParseString(value, "");
                } else if (SameKey(key, "sNeutralStanceKey")) {
                    neutralKey = ParseString(value, "");
                } else if (SameKey(key, "sCycleStanceKey")) {
                    cycleKey = ParseString(value, "");
                }
            }
        } else {
            logger::warn("{} not found. Using default Stances settings.", INI_PATH);
        }

        debugLogging_.store(debugLogging, std::memory_order_relaxed);
        skillUsePerKill_.store(skillUsePerKill, std::memory_order_relaxed);
        scaleWithVictimLevel_.store(scaleWithVictimLevel, std::memory_order_relaxed);
        victimLevelMult_.store(victimLevelMult, std::memory_order_relaxed);
        applyStanceOnStart_.store(applyStanceOnStart, std::memory_order_relaxed);
        showWidget_.store(showWidget, std::memory_order_relaxed);
        hudX_.store(hudX, std::memory_order_relaxed);
        hudY_.store(hudY, std::memory_order_relaxed);
        hudScale_.store(hudScale, std::memory_order_relaxed);
        autoHideWidget_.store(autoHideWidget, std::memory_order_relaxed);
        widgetSeconds_.store(std::clamp(widgetSeconds, kMinWidgetSeconds, kMaxWidgetSeconds), std::memory_order_relaxed);

        {
            std::scoped_lock lock(stringLock_);
            bearKey_ = std::move(bearKey);
            wolfKey_ = std::move(wolfKey);
            hawkKey_ = std::move(hawkKey);
            neutralKey_ = std::move(neutralKey);
            cycleKey_ = std::move(cycleKey);
        }

        spdlog::set_level(debugLogging ? spdlog::level::info : spdlog::level::warn);

        logger::info(
            "Settings loaded. skillUsePerKill={} scaleWithVictimLevel={} victimLevelMult={} debugLogging={}",
            skillUsePerKill,
            scaleWithVictimLevel,
            victimLevelMult,
            debugLogging);
    }

    void Settings::Save() const
    {
        std::ofstream file(INI_PATH.data(), std::ios::trunc);
        if (!file.is_open()) {
            logger::warn("Could not write {}.", INI_PATH);
            return;
        }

        std::string bearKey;
        std::string wolfKey;
        std::string hawkKey;
        std::string neutralKey;
        std::string cycleKey;
        {
            std::scoped_lock lock(stringLock_);
            bearKey = bearKey_;
            wolfKey = wolfKey_;
            hawkKey = hawkKey_;
            neutralKey = neutralKey_;
            cycleKey = cycleKey_;
        }

        const auto boolText = [](bool a_value) { return a_value ? "true" : "false"; };

        file << "[XP]\n";
        file << "; Skill-use magnitude sent to Custom Skills Framework per valid player kill.\n";
        file << "; CSF applies the Stances.json formula after this.\n";
        file << "fSkillUsePerKill = " << skillUsePerKill_.load(std::memory_order_relaxed) << "\n\n";
        file << "; Adds victim level * fVictimLevelMult to fSkillUsePerKill.\n";
        file << "bScaleWithVictimLevel = " << boolText(scaleWithVictimLevel_.load(std::memory_order_relaxed)) << "\n";
        file << "fVictimLevelMult = " << victimLevelMult_.load(std::memory_order_relaxed) << "\n\n";

        file << "[Keybinds]\n";
        file << "; Key patterns like \"x\", \"shift+x\", \"ctrl+shift+e\". Empty = unbound.\n";
        file << "sBearStanceKey = " << bearKey << "\n";
        file << "sWolfStanceKey = " << wolfKey << "\n";
        file << "sHawkStanceKey = " << hawkKey << "\n";
        file << "sNeutralStanceKey = " << neutralKey << "\n";
        file << "; Cycles Bear > Wolf > Hawk. Leave empty to disable cycling.\n";
        file << "sCycleStanceKey = " << cycleKey << "\n\n";

        file << "[General]\n";
        file << "bApplyStanceOnStart = " << boolText(applyStanceOnStart_.load(std::memory_order_relaxed)) << "\n\n";

        file << "[HUD]\n";
        file << "; Stance widget. Position is on a 1280x720 stage, scale in percent.\n";
        file << "bShowWidget = " << boolText(showWidget_.load(std::memory_order_relaxed)) << "\n";
        file << "iHudX = " << hudX_.load(std::memory_order_relaxed) << "\n";
        file << "iHudY = " << hudY_.load(std::memory_order_relaxed) << "\n";
        file << "iHudScale = " << hudScale_.load(std::memory_order_relaxed) << "\n";
        file << "; Auto-hide: widget pops on stance changes and hides after iWidgetSeconds (3-15).\n";
        file << "; Disable to keep the widget on screen while a stance is active.\n";
        file << "bAutoHideWidget = " << boolText(autoHideWidget_.load(std::memory_order_relaxed)) << "\n";
        file << "iWidgetSeconds = " << widgetSeconds_.load(std::memory_order_relaxed) << "\n\n";

        file << "[Debug]\n";
        file << "; Master toggle for this DLL's informational logging, including the\n";
        file << "; one-log-line-per-valid-XP-award messages. Warnings and errors are\n";
        file << "; always written regardless. Keep false for normal gameplay.\n";
        file << "bEnableLogging = " << boolText(debugLogging_.load(std::memory_order_relaxed)) << "\n";
    }

    float Settings::CalculateKillXP(const RE::Actor* a_victim) const
    {
        float result = skillUsePerKill_.load(std::memory_order_relaxed);

        if (scaleWithVictimLevel_.load(std::memory_order_relaxed) && a_victim) {
            result += static_cast<float>(a_victim->GetLevel()) *
                      victimLevelMult_.load(std::memory_order_relaxed);
        }

        return std::max(result, 0.0f);
    }

    std::string Settings::BearKey() const
    {
        std::scoped_lock lock(stringLock_);
        return bearKey_;
    }

    void Settings::SetBearKey(std::string a_value)
    {
        std::scoped_lock lock(stringLock_);
        bearKey_ = std::move(a_value);
    }

    std::string Settings::WolfKey() const
    {
        std::scoped_lock lock(stringLock_);
        return wolfKey_;
    }

    void Settings::SetWolfKey(std::string a_value)
    {
        std::scoped_lock lock(stringLock_);
        wolfKey_ = std::move(a_value);
    }

    std::string Settings::HawkKey() const
    {
        std::scoped_lock lock(stringLock_);
        return hawkKey_;
    }

    void Settings::SetHawkKey(std::string a_value)
    {
        std::scoped_lock lock(stringLock_);
        hawkKey_ = std::move(a_value);
    }

    std::string Settings::NeutralKey() const
    {
        std::scoped_lock lock(stringLock_);
        return neutralKey_;
    }

    void Settings::SetNeutralKey(std::string a_value)
    {
        std::scoped_lock lock(stringLock_);
        neutralKey_ = std::move(a_value);
    }

    std::string Settings::CycleKey() const
    {
        std::scoped_lock lock(stringLock_);
        return cycleKey_;
    }

    void Settings::SetCycleKey(std::string a_value)
    {
        std::scoped_lock lock(stringLock_);
        cycleKey_ = std::move(a_value);
    }
}
