#pragma once

namespace Stances
{
    class StanceInput final : public REX::Singleton<StanceInput>, public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static void Register();

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_source) override;
    };
}
