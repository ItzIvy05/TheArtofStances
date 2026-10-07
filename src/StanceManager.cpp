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

        RE::BGSPerk* perk = nullptr;
        switch (a_stance) {
        case Stance::kBear:
            perk = StanceForms::bearPerk;
            break;
        case Stance::kWolf:
            perk = StanceForms::wolfPerk;
            break;
        case Stance::kHawk:
            perk = StanceForms::hawkPerk;
            break;
        default:
            break;
        }

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

    void StanceManager::NotifyStatsMenu(bool a_opening)
    {
        static std::array<bool, 3> hadPerk{ true, true, true };
        static bool snapshotValid = false;

        if (!StanceForms::IsReady()) {
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            snapshotValid = false;
            return;
        }

        const std::array<std::pair<RE::BGSPerk*, Stance>, 3> aspects{ {
            { StanceForms::bearPerk, Stance::kBear },
            { StanceForms::wolfPerk, Stance::kWolf },
            { StanceForms::hawkPerk, Stance::kHawk },
        } };

        if (a_opening) {
            for (std::size_t i = 0; i < aspects.size(); ++i) {
                hadPerk[i] = !aspects[i].first || player->HasPerk(aspects[i].first);
            }
            snapshotValid = true;
            return;
        }

        if (!snapshotValid) {
            return;
        }
        snapshotValid = false;

        for (std::size_t i = 0; i < aspects.size(); ++i) {
            const auto perk = aspects[i].first;
            if (!hadPerk[i] && perk && player->HasPerk(perk)) {
                UpdateStancePlayer(aspects[i].second);
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
