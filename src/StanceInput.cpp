#include "StanceInput.h"

#include "MCP.h"
#include "Settings.h"
#include "StanceManager.h"

#include <ClibUtil/hotkeys.hpp>

namespace Stances
{
    namespace
    {
        constexpr std::array<std::uint32_t, 4> MOVEMENT_KEYS{ 17, 30, 31, 32 };

        [[nodiscard]] bool IsMovementKey(std::uint32_t a_key)
        {
            return std::ranges::find(MOVEMENT_KEYS, a_key) != MOVEMENT_KEYS.end();
        }

        [[nodiscard]] bool ShouldBlockInput()
        {
            const auto ui = RE::UI::GetSingleton();
            return !ui || ui->GameIsPaused() || ui->IsMenuOpen(RE::Console::MENU_NAME) || MCP::IsMenuBlocking();
        }
    }

    bool StanceHotkey::SetPattern(std::string_view a_pattern)
    {
        keys.clear();
        triggered = false;

        auto pattern = clib_util::string::tolower(a_pattern);
        clib_util::string::replace_all(pattern, " "sv, ""sv);
        clib_util::string::replace_all(pattern, "num+"sv, "numplus"sv);

        if (pattern.empty()) {
            return true;
        }

        try {
            for (const auto& part : clib_util::string::split(pattern, "+")) {
                keys.insert(clib_util::hotkeys::details::GetKeyByName(part));
            }
        } catch (...) {
            keys.clear();
            return false;
        }

        return !keys.empty();
    }

    bool StanceHotkey::Update(const std::set<std::uint32_t>& a_pressed)
    {
        if (keys.empty()) {
            triggered = false;
            return false;
        }

        if (a_pressed == keys) {
            if (!triggered) {
                triggered = true;
                return true;
            }
        } else {
            triggered = false;
        }

        return false;
    }

    StanceInput* StanceInput::GetSingleton()
    {
        static StanceInput singleton;
        return std::addressof(singleton);
    }

    void StanceInput::Register()
    {
        const auto manager = RE::BSInputDeviceManager::GetSingleton();
        if (!manager) {
            logger::error("BSInputDeviceManager unavailable; stance hotkeys disabled.");
            return;
        }

        manager->AddEventSink(GetSingleton());
        logger::info("Stance input sink registered.");
    }

    void StanceInput::ApplyKeys()
    {
        const auto settings = Settings::GetSingleton();
        const auto instance = GetSingleton();

        const auto apply = [](StanceHotkey& a_hotkey, const std::string& a_pattern, const char* a_name) {
            if (!a_hotkey.SetPattern(a_pattern)) {
                logger::warn("Invalid {} stance key pattern '{}'.", a_name, a_pattern);
            }
        };

        apply(instance->bearKey_, settings->BearKey(), "bear");
        apply(instance->wolfKey_, settings->WolfKey(), "wolf");
        apply(instance->hawkKey_, settings->HawkKey(), "hawk");
        apply(instance->neutralKey_, settings->NeutralKey(), "neutral");
        apply(instance->cycleKey_, settings->CycleKey(), "cycle");
    }

    bool StanceInput::IsValidPattern(std::string_view a_pattern)
    {
        StanceHotkey probe;
        return probe.SetPattern(a_pattern);
    }

    std::uint32_t StanceInput::NormalizeKey(const RE::ButtonEvent* a_button)
    {
        auto key = a_button->GetIDCode();
        switch (a_button->GetDevice()) {
        case RE::INPUT_DEVICE::kMouse:
            key += SKSE::InputMap::kMacro_MouseButtonOffset;
            break;
        case RE::INPUT_DEVICE::kGamepad:
            key = SKSE::InputMap::GamepadMaskToKeycode(key);
            break;
        default:
            break;
        }

        return key;
    }

    RE::BSEventNotifyControl StanceInput::ProcessEvent(
        RE::InputEvent* const* a_event,
        [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* a_source)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (ShouldBlockInput()) {
            bearKey_.Reset();
            wolfKey_.Reset();
            hawkKey_.Reset();
            neutralKey_.Reset();
            cycleKey_.Reset();
            return RE::BSEventNotifyControl::kContinue;
        }

        std::set<std::uint32_t> pressed;
        for (auto event = *a_event; event; event = event->next) {
            const auto button = event->AsButtonEvent();
            if (!button || !button->HasIDCode() || !button->IsPressed()) {
                continue;
            }

            if (button->GetDevice() == RE::INPUT_DEVICE::kKeyboard && IsMovementKey(button->GetIDCode())) {
                continue;
            }

            pressed.insert(NormalizeKey(button));
        }

        // Every hotkey must see every event, otherwise one that fired earlier in the
        // chain leaves the others latched and they stop responding.
        const bool cycle = cycleKey_.Update(pressed);
        const bool wolf = wolfKey_.Update(pressed);
        const bool bear = bearKey_.Update(pressed);
        const bool hawk = hawkKey_.Update(pressed);
        const bool neutral = neutralKey_.Update(pressed);

        if (cycle) {
            StanceManager::CycleStancesPlayer();
        } else if (wolf) {
            StanceManager::UpdateStancePlayer(Stance::kWolf);
        } else if (bear) {
            StanceManager::UpdateStancePlayer(Stance::kBear);
        } else if (hawk) {
            StanceManager::UpdateStancePlayer(Stance::kHawk);
        } else if (neutral) {
            StanceManager::UpdateStancePlayer(Stance::kNeutral);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
