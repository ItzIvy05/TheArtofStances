#include "CustomSkillsAPI.h"

namespace Stances::CustomSkillsAPI
{
    namespace
    {
        CustomSkills::CustomSkillsInterface* g_customSkills{ nullptr };
    }

    void OnMessage(SKSE::MessagingInterface::Message* a_msg)
    {
        auto* oldInterface = g_customSkills;
        CustomSkills::QueryCustomSkillsInterface(a_msg, g_customSkills);

        if (g_customSkills && g_customSkills != oldInterface) {
            logger::info("Custom Skills Framework interface acquired. version={}", g_customSkills->Version());
        }
    }

    CustomSkills::CustomSkillsInterface* GetInterface() noexcept
    {
        return g_customSkills;
    }

    bool IsReady() noexcept
    {
        return g_customSkills != nullptr;
    }
}
