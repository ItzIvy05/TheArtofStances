#pragma once

#include <set>

namespace Stances
{
    struct StanceHotkey final
    {
        bool SetPattern(std::string_view a_pattern);
        bool Update(const std::set<std::uint32_t>& a_pressed);
        void Reset() noexcept { triggered = false; }

        std::set<std::uint32_t> keys;
        bool triggered{ false };
    };

    class StanceInput final : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static StanceInput* GetSingleton();
        static void Register();
        static void ApplyKeys();
        [[nodiscard]] static bool IsValidPattern(std::string_view a_pattern);
        [[nodiscard]] static std::uint32_t NormalizeKey(const RE::ButtonEvent* a_button);

        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>* a_source) override;

    private:
        StanceInput() = default;

        StanceHotkey bearKey_;
        StanceHotkey wolfKey_;
        StanceHotkey hawkKey_;
        StanceHotkey neutralKey_;
        StanceHotkey cycleKey_;
    };
}
