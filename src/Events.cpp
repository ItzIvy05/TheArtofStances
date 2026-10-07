#include "Events.h"

#include "KillXP.h"
#include "StanceManager.h"
#include "Widget.h"

namespace Stances::Events
{
    namespace
    {
        class MenuEventSink final : public REX::Singleton<MenuEventSink>, public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override
            {
                if (!a_event) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                if (a_event->menuName == RE::StatsMenu::MENU_NAME) {
                    StanceManager::NotifyStatsMenu(a_event->opening);
                }

                Widget::NotifyMenuEvent(a_event->menuName.c_str(), a_event->opening);
                return RE::BSEventNotifyControl::kContinue;
            }
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
