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

                Widget::NotifyMenuEvent(a_event->menuName.c_str(), a_event->opening);
                return RE::BSEventNotifyControl::kContinue;
            }
        };

        // the stats menu, papyrus and the console all add the player's perks through this
        struct AddPerk
        {
            static void thunk(RE::PlayerCharacter* a_this, RE::BGSPerk* a_perk, std::uint32_t a_rank)
            {
                func(a_this, a_perk, a_rank);
                StanceManager::NotifyPerkAdded(a_perk);
            }

            static inline REL::Relocation<decltype(thunk)> func;
        };
    }

    void Register()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
        AddPerk::func = vtbl.write_vfunc(0xFB, AddPerk::thunk);

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
