#pragma once

#include <atomic>

namespace RE
{
    class Actor;
}

namespace Stances
{
    class Settings final
    {
    public:
        static Settings* GetSingleton();

        void Load();

        [[nodiscard]] bool DebugLogging() const noexcept
        {
            return debugLogging_.load(std::memory_order_relaxed);
        }

        [[nodiscard]] float CalculateKillXP(const RE::Actor* a_victim) const;

    private:
        Settings() = default;

        static constexpr bool kDefaultDebugLogging = false;
        static constexpr float kDefaultSkillUsePerKill = 8.0f;
        static constexpr bool kDefaultScaleWithVictimLevel = true;
        static constexpr float kDefaultVictimLevelMult = 0.35f;

        std::atomic_bool debugLogging_{ kDefaultDebugLogging };
        std::atomic<float> skillUsePerKill_{ kDefaultSkillUsePerKill };
        std::atomic_bool scaleWithVictimLevel_{ kDefaultScaleWithVictimLevel };
        std::atomic<float> victimLevelMult_{ kDefaultVictimLevelMult };
    };
}
