#include "CustomSkillsAPI.h"
#include "Events.h"
#include "Hooks.h"
#include "Manager.h"
#include "MCP.h"
#include "Plugin.h"
#include "Serialization.h"
#include "logger.h"

namespace
{
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        if (!a_msg) {
            return;
        }

        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoad:
            Stances::Hooks::Install();
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            Stances::Manager::GetSingleton()->OnDataLoaded();
            Stances::Events::Register();
            break;
        case SKSE::MessagingInterface::kNewGame:
            Stances::Manager::GetSingleton()->OnNewGame();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            Stances::Manager::GetSingleton()->OnPostLoadGame();
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    SetupLog();

    logger::info("Loading {}...", Stances::PLUGIN_NAME);

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

    Stances::Serialization::Register();
    Stances::MCP::Register();

    logger::info("{} loaded.", Stances::PLUGIN_NAME);
    return true;
}
