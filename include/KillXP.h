#pragma once

#include "RE/A/ActorKill.h"

namespace Stances
{
    class KillXPEventSink final : public RE::BSTEventSink<RE::ActorKill::Event>
    {
    public:
        static KillXPEventSink* GetSingleton();

        RE::BSEventNotifyControl ProcessEvent(
            const RE::ActorKill::Event* a_event,
            RE::BSTEventSource<RE::ActorKill::Event>* a_source) override;

    private:
        KillXPEventSink() = default;
    };
}
