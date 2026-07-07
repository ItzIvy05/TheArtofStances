#include "Events.h"

#include "KillXP.h"

namespace Stances::Events
{
    void Register()
    {
        auto* source = RE::ActorKill::GetEventSource();
        if (!source) {
            logger::critical("Failed to get ActorKill event source.");
            return;
        }

        source->AddEventSink(KillXPEventSink::GetSingleton());
        logger::info("ActorKill event sink registered.");
    }
}
