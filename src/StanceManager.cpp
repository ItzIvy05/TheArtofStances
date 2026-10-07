#include "StanceManager.h"

#include "Settings.h"
#include "StanceForms.h"
#include "Widget.h"

namespace Stances
{
    namespace
    {
        void ApplySpell(RE::Actor* a_actor, RE::SpellItem* a_spell)
        {
            if (a_spell->IsPermanent()) {
                a_actor->AddSpell(a_spell);
                return;
            }

            const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
            if (caster) {
                caster->CastSpellImmediate(a_spell, false, a_actor, 1.0f, false, 0.0f, nullptr);
            }
        }
    }

    Stance StanceManager::CurrentStance()
    {
        if (!StanceForms::currentStanceGlobal) {
            return Stance::kNeutral;
        }

        const auto value = static_cast<std::uint32_t>(StanceForms::currentStanceGlobal->value);
        return value <= 3 ? static_cast<Stance>(value) : Stance::kNeutral;
    }

    const char* StanceManager::StanceName(Stance a_stance)
    {
        switch (a_stance) {
        case Stance::kBear:
            return "Bear";
        case Stance::kWolf:
            return "Wolf";
        case Stance::kHawk:
            return "Hawk";
        case Stance::kNeutral:
            return "Neutral";
        default:
            return "Invalid";
        }
    }

    bool StanceManager::IsStanceUnlocked(Stance a_stance)
    {
        if (a_stance == Stance::kNeutral) {
            return true;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return false;
        }

        const auto perk = GetAspectPerk(a_stance);
        return !perk || player->HasPerk(perk);
    }

    void StanceManager::UpdateStance(Stance a_stance, RE::Actor* a_actor)
    {
        if (!StanceForms::IsReady()) {
            return;
        }

        const auto previous = CurrentStance();
        if (Settings::debugLogging.GetValue()) {
            logger::info("Switching stance: {} -> {}", StanceName(previous), StanceName(a_stance));
        }

        StanceForms::previousStanceGlobal->value = static_cast<float>(std::to_underlying(previous));
        ApplyStance(a_stance, a_actor);
        StanceForms::currentStanceGlobal->value = static_cast<float>(std::to_underlying(a_stance));
        Widget::Pop();
    }

    void StanceManager::UpdateStancePlayer(Stance a_stance)
    {
        if (!IsStanceUnlocked(a_stance)) {
            if (Settings::debugLogging.GetValue()) {
                logger::info("{} stance is locked; Aspect perk missing.", StanceName(a_stance));
            }
            return;
        }

        if (a_stance == CurrentStance()) {
            Widget::Pop();
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (player) {
            UpdateStance(a_stance, player);
        }
    }

    void StanceManager::CycleStancesPlayer()
    {
        if (!StanceForms::IsReady()) {
            return;
        }

        auto candidate = static_cast<std::uint32_t>(CurrentStance());
        for (int i = 0; i < 3; ++i) {
            candidate = candidate % 3 + 1;
            const auto stance = static_cast<Stance>(candidate);
            if (IsStanceUnlocked(stance)) {
                UpdateStancePlayer(stance);
                return;
            }
        }
    }

    void StanceManager::HandlePlayerKill()
    {
        if (!StanceForms::savageInstinctControlEffect || !StanceForms::savageInstinctSpell || CurrentStance() != Stance::kWolf) {
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return;
        }

        const auto magicTarget = player->AsMagicTarget();
        if (!magicTarget || !magicTarget->HasMagicEffect(StanceForms::savageInstinctControlEffect)) {
            return;
        }

        ApplySpell(player, StanceForms::savageInstinctSpell);
        if (Settings::debugLogging.GetValue()) {
            logger::info("Savage Instinct kill bonus applied.");
        }
    }

    void StanceManager::NotifyPerkAdded(const RE::BGSPerk* a_perk)
    {
        for (const auto stance : { Stance::kBear, Stance::kWolf, Stance::kHawk }) {
            if (a_perk == GetAspectPerk(stance)) {
                // papyrus adds perks
                SKSE::GetTaskInterface()->AddTask(std::bind_front(UpdateStancePlayer, stance));
                return;
            }
        }
    }

    RE::BGSPerk* StanceManager::GetAspectPerk(Stance a_stance)
    {
        switch (a_stance) {
        case Stance::kBear:
            return StanceForms::bearPerk;
        case Stance::kWolf:
            return StanceForms::wolfPerk;
        case Stance::kHawk:
            return StanceForms::hawkPerk;
        default:
            return nullptr;
        }
    }

    RE::SpellItem* StanceManager::GetStanceSpell(Stance a_stance)
    {
        switch (a_stance) {
        case Stance::kBear:
            return StanceForms::bearSpell;
        case Stance::kWolf:
            return StanceForms::wolfSpell;
        case Stance::kHawk:
            return StanceForms::hawkSpell;
        default:
            return nullptr;
        }
    }

    void StanceManager::ApplyStance(Stance a_stance, RE::Actor* a_actor)
    {
        if (!a_actor) {
            return;
        }

        RemoveAllStances(a_actor);

        const auto spell = GetStanceSpell(a_stance);
        if (spell) {
            ApplySpell(a_actor, spell);
        }
    }

    void StanceManager::RemoveAllStances(RE::Actor* a_actor)
    {
        for (const auto spell : StanceForms::stanceSpells) {
            if (a_actor->HasSpell(spell)) {
                a_actor->RemoveSpell(spell);
            }
        }
    }
}
