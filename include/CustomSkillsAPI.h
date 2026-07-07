#pragma once

#include "CustomSkills/Interfaces.h"

namespace Stances::CustomSkillsAPI
{
    void OnMessage(SKSE::MessagingInterface::Message* a_msg);

    [[nodiscard]] CustomSkills::CustomSkillsInterface* GetInterface() noexcept;
    [[nodiscard]] bool IsReady() noexcept;
}
