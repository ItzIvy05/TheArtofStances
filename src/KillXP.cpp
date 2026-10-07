#include "KillXP.h"

#include "CustomSkillsAPI.h"
#include "Settings.h"
#include "StanceManager.h"

namespace Stances
{
    namespace
    {
        constexpr auto SKILL_ID = "TheArtofStance";

        std::atomic_bool g_warnedInterfaceNotReady{ false };

        [[nodiscard]] bool IsPlayerKill(RE::Actor* a_killer)
        {
            return a_killer && a_killer->IsPlayerRef();
        }

        [[nodiscard]] bool IsValidVictim(RE::Actor* a_victim)
        {
            if (!a_victim || a_victim->IsPlayerRef()) {
                return false;
            }

            // not exposed in the ini on purpose
            if (a_victim->IsPlayerTeammate()) {
                return false;
            }

            if (a_victim->IsCommandedActor()) {
                const auto commander = a_victim->GetCommandingActor();
                if (commander && commander->IsPlayerRef()) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] bool IsRangedOrMagic(RE::TESForm* a_object)
        {
            if (!a_object) {
                return false;
            }

            if (a_object->Is(RE::FormType::Spell, RE::FormType::Scroll)) {
                return true;
            }

            const auto weapon = a_object->As<RE::TESObjectWEAP>();
            return weapon && weapon->IsRanged();
        }

        // melee kills only
        [[nodiscard]] bool IsMeleeKill()
        {
            const auto player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                return false;
            }

            auto* right = player->GetEquippedObject(false);
            if (IsRangedOrMagic(right)) {
                return false;
            }

            if (!right && IsRangedOrMagic(player->GetEquippedObject(true))) {
                return false;
            }

            return true;
        }
    }

    RE::BSEventNotifyControl KillXPEventSink::ProcessEvent(const RE::ActorKill::Event* a_event, [[maybe_unused]] RE::BSTEventSource<RE::ActorKill::Event>* a_source)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* killer = a_event->killer;
        auto* victim = a_event->victim;

        if (!IsPlayerKill(killer) || !IsValidVictim(victim)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        StanceManager::HandlePlayerKill();

        if (!IsMeleeKill()) {
            if (Settings::debugLogging.GetValue()) {
                logger::info("Kill was not made in melee; no Stances XP awarded.");
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* customSkills = CustomSkillsAPI::GetInterface();
        if (!customSkills) {
            if (!g_warnedInterfaceNotReady.exchange(true)) {
                logger::warn("Custom Skills Framework interface is not ready; Stances kill XP cannot be awarded yet.");
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        if (g_warnedInterfaceNotReady.exchange(false)) {
            logger::info("Custom Skills Framework interface is now available; Stances kill XP awards resumed.");
        }

        const auto xp = Settings::CalculateKillXP(victim);
        if (xp <= 0.0f) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // skill-use magnitude, csf applies the Stances.json formula
        customSkills->AdvanceSkill(SKILL_ID, xp);

        if (Settings::debugLogging.GetValue()) {
            logger::info("Advanced {} by {} skill-use magnitude. killer=0x{:08X} victim=0x{:08X} victimLevel={}", SKILL_ID, xp, killer->GetFormID(), victim->GetFormID(), victim->GetLevel());
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
