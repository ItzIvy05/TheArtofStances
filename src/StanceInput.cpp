#include "StanceInput.h"

#include "Settings.h"
#include "StanceManager.h"

#include <SKSEMenuFramework.h>

namespace Stances
{
    namespace
    {
        [[nodiscard]] bool ShouldBlockInput()
        {
            const auto ui = RE::UI::GetSingleton();
            return !ui || ui->GameIsPaused() || ui->IsMenuOpen(RE::Console::MENU_NAME) || SKSEMenuFramework::IsAnyBlockingWindowOpened();
        }

        void OnPress(std::uint32_t a_key)
        {
            if (a_key == Settings::cycleKey.GetValue()) {
                StanceManager::CycleStancesPlayer();
            } else if (a_key == Settings::wolfKey.GetValue()) {
                StanceManager::UpdateStancePlayer(Stance::kWolf);
            } else if (a_key == Settings::bearKey.GetValue()) {
                StanceManager::UpdateStancePlayer(Stance::kBear);
            } else if (a_key == Settings::hawkKey.GetValue()) {
                StanceManager::UpdateStancePlayer(Stance::kHawk);
            } else if (a_key == Settings::neutralKey.GetValue()) {
                StanceManager::UpdateStancePlayer(Stance::kNeutral);
            }
        }
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

    RE::BSEventNotifyControl StanceInput::ProcessEvent(RE::InputEvent* const* a_event, [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* a_source)
    {
        if (!a_event || ShouldBlockInput()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        for (auto event = *a_event; event; event = event->next) {
            const auto button = event->AsButtonEvent();
            if (!button || !button->HasIDCode() || !button->IsDown()) {
                continue;
            }

            const auto binding = ModConfigUI::ButtonBinding::FromEvent(button->GetDevice(), button->GetIDCode());
            if (binding.IsSet()) {
                OnPress(binding.key);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
