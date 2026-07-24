#pragma once

#include <atomic>
#include <mutex>
#include <string>

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
        void Save() const;

        [[nodiscard]] bool DebugLogging() const noexcept
        {
            return debugLogging_.load(std::memory_order_relaxed);
        }

        [[nodiscard]] float CalculateKillXP(const RE::Actor* a_victim) const;

        [[nodiscard]] bool ApplyStanceOnStart() const noexcept
        {
            return applyStanceOnStart_.load(std::memory_order_relaxed);
        }

        void SetApplyStanceOnStart(bool a_value) noexcept
        {
            applyStanceOnStart_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] bool ShowWidget() const noexcept
        {
            return showWidget_.load(std::memory_order_relaxed);
        }

        void SetShowWidget(bool a_value) noexcept
        {
            showWidget_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] int HudX() const noexcept
        {
            return hudX_.load(std::memory_order_relaxed);
        }

        void SetHudX(int a_value) noexcept
        {
            hudX_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] int HudY() const noexcept
        {
            return hudY_.load(std::memory_order_relaxed);
        }

        void SetHudY(int a_value) noexcept
        {
            hudY_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] int HudScale() const noexcept
        {
            return hudScale_.load(std::memory_order_relaxed);
        }

        void SetHudScale(int a_value) noexcept
        {
            hudScale_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] bool AutoHideWidget() const noexcept
        {
            return autoHideWidget_.load(std::memory_order_relaxed);
        }

        void SetAutoHideWidget(bool a_value) noexcept
        {
            autoHideWidget_.store(a_value, std::memory_order_relaxed);
        }

        [[nodiscard]] int WidgetSeconds() const noexcept
        {
            return widgetSeconds_.load(std::memory_order_relaxed);
        }

        void SetWidgetSeconds(int a_value) noexcept
        {
            widgetSeconds_.store(std::clamp(a_value, kMinWidgetSeconds, kMaxWidgetSeconds), std::memory_order_relaxed);
        }

        [[nodiscard]] std::string BearKey() const;
        void SetBearKey(std::string a_value);
        [[nodiscard]] std::string WolfKey() const;
        void SetWolfKey(std::string a_value);
        [[nodiscard]] std::string HawkKey() const;
        void SetHawkKey(std::string a_value);
        [[nodiscard]] std::string NeutralKey() const;
        void SetNeutralKey(std::string a_value);
        [[nodiscard]] std::string CycleKey() const;
        void SetCycleKey(std::string a_value);

    private:
        Settings() = default;

        static constexpr bool kDefaultDebugLogging = false;
        static constexpr float kDefaultSkillUsePerKill = 8.0f;
        static constexpr bool kDefaultScaleWithVictimLevel = true;
        static constexpr float kDefaultVictimLevelMult = 0.35f;
        static constexpr bool kDefaultApplyStanceOnStart = false;
        static constexpr bool kDefaultShowWidget = true;
        static constexpr int kDefaultHudX = 64;
        static constexpr int kDefaultHudY = 620;
        static constexpr int kDefaultHudScale = 100;
        static constexpr bool kDefaultAutoHideWidget = true;

    public:
        static constexpr int kMinWidgetSeconds = 3;
        static constexpr int kMaxWidgetSeconds = 15;
        static constexpr int kDefaultWidgetSeconds = 10;

    private:
        static constexpr auto kDefaultBearKey = "shift+x";
        static constexpr auto kDefaultWolfKey = "x";
        static constexpr auto kDefaultHawkKey = "ctrl+x";
        static constexpr auto kDefaultNeutralKey = "alt+v";
        static constexpr auto kDefaultCycleKey = "";

        std::atomic_bool debugLogging_{ kDefaultDebugLogging };
        std::atomic<float> skillUsePerKill_{ kDefaultSkillUsePerKill };
        std::atomic_bool scaleWithVictimLevel_{ kDefaultScaleWithVictimLevel };
        std::atomic<float> victimLevelMult_{ kDefaultVictimLevelMult };

        std::atomic_bool applyStanceOnStart_{ kDefaultApplyStanceOnStart };
        std::atomic_bool showWidget_{ kDefaultShowWidget };
        std::atomic<int> hudX_{ kDefaultHudX };
        std::atomic<int> hudY_{ kDefaultHudY };
        std::atomic<int> hudScale_{ kDefaultHudScale };
        std::atomic_bool autoHideWidget_{ kDefaultAutoHideWidget };
        std::atomic<int> widgetSeconds_{ kDefaultWidgetSeconds };

        mutable std::mutex stringLock_;
        std::string bearKey_{ kDefaultBearKey };
        std::string wolfKey_{ kDefaultWolfKey };
        std::string hawkKey_{ kDefaultHawkKey };
        std::string neutralKey_{ kDefaultNeutralKey };
        std::string cycleKey_{ kDefaultCycleKey };
    };
}
