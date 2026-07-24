#include "Events.h"

#include "KillXP.h"
#include "StanceManager.h"
#include "Widget.h"

namespace Stances::Events
{
    namespace
    {
        class MenuEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            static MenuEventSink* GetSingleton()
            {
                static MenuEventSink singleton;
                return std::addressof(singleton);
            }

            RE::BSEventNotifyControl ProcessEvent(
                const RE::MenuOpenCloseEvent* a_event,
                [[maybe_unused]] RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override
            {
                if (!a_event) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                if (a_event->menuName == RE::RaceSexMenu::MENU_NAME && !a_event->opening) {
                    StanceManager::ApplyDefaultStance();
                }

                if (a_event->menuName == RE::StatsMenu::MENU_NAME) {
                    StanceManager::NotifyStatsMenu(a_event->opening);
                }

                if (a_event->menuName == "HUD Menu" && a_event->opening) {
                    StanceManager::ApplyDefaultStance();
                }

                Widget::NotifyMenuEvent(a_event->menuName.c_str(), a_event->opening);
                return RE::BSEventNotifyControl::kContinue;
            }

        private:
            MenuEventSink() = default;
        };
    }

    void Register()
    {
        auto* source = RE::ActorKill::GetEventSource();
        if (!source) {
            logger::critical("Failed to get ActorKill event source.");
        } else {
            source->AddEventSink(KillXPEventSink::GetSingleton());
            logger::info("ActorKill event sink registered.");
        }

        const auto ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("UI singleton unavailable; menu event sink not registered.");
            return;
        }

        ui->AddEventSink(MenuEventSink::GetSingleton());
        logger::info("Menu event sink registered.");
    }
}
