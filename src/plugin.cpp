#include "CustomSkillsAPI.h"
#include "Events.h"
#include "Settings.h"
#include "SettingsUI.h"
#include "StanceForms.h"
#include "StanceInput.h"
#include "Widget.h"

namespace
{
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        if (!a_msg) {
            return;
        }

        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoad:
            Stances::SettingsUI::Register();
            break;
        case SKSE::MessagingInterface::kInputLoaded:
            Stances::StanceInput::Register();
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            Stances::Settings::Load();
            Stances::StanceForms::Load();
            Stances::Widget::Init();
            Stances::Events::Register();
            break;
        case SKSE::MessagingInterface::kNewGame:
        case SKSE::MessagingInterface::kPostLoadGame:
            Stances::Settings::Load();
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    logger::info("Loading {}...", SKSE::GetPluginName());

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        logger::critical("Failed to get SKSE messaging interface.");
        return false;
    }

    if (!messaging->RegisterListener(MessageHandler)) {
        logger::critical("Failed to register SKSE messaging listener.");
        return false;
    }

    if (!messaging->RegisterListener("CustomSkills", Stances::CustomSkillsAPI::OnMessage)) {
        logger::warn("Failed to register Custom Skills Framework message listener. Kill XP will wait for interface availability.");
    }

    logger::info("{} loaded.", SKSE::GetPluginName());
    return true;
}
