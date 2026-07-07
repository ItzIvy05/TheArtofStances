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
                }
            }
        } else {
            logger::warn("{} not found. Using default Stances XP settings.", INI_PATH);
        }

        debugLogging_.store(debugLogging, std::memory_order_relaxed);
        skillUsePerKill_.store(skillUsePerKill, std::memory_order_relaxed);
        scaleWithVictimLevel_.store(scaleWithVictimLevel, std::memory_order_relaxed);
        victimLevelMult_.store(victimLevelMult, std::memory_order_relaxed);

        spdlog::set_level(debugLogging ? spdlog::level::info : spdlog::level::warn);

        logger::info(
            "Settings loaded. skillUsePerKill={} scaleWithVictimLevel={} victimLevelMult={} debugLogging={}",
            skillUsePerKill,
            scaleWithVictimLevel,
            victimLevelMult,
            debugLogging);
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
}
