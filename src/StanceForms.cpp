#include "StanceForms.h"

#include "Settings.h"
#include "Utils.h"

namespace Stances
{
    namespace
    {
        constexpr auto STANCE_PLUGIN = "The Art of Stances.esp"sv;

        constexpr RE::FormID BEAR_STANCE_ID = 0x900;
        constexpr RE::FormID WOLF_STANCE_ID = 0x901;
        constexpr RE::FormID HAWK_STANCE_ID = 0x902;
        constexpr RE::FormID PREVIOUS_STANCE_GLOBAL_ID = 0x916;
        constexpr RE::FormID CURRENT_STANCE_GLOBAL_ID = 0x917;
        constexpr RE::FormID SAVAGE_CONTROL_EFFECT_ID = 0x93B;
        constexpr RE::FormID SAVAGE_SPELL_ID = 0x93C;
        constexpr RE::FormID BEAR_PERK_ID = 0x920;
        constexpr RE::FormID WOLF_PERK_ID = 0x924;
        constexpr RE::FormID HAWK_PERK_ID = 0x928;
    }

    bool StanceForms::Load()
    {
        ready = false;

        if (!Utils::IsModLoaded(STANCE_PLUGIN)) {
            logger::warn("{} is not loaded. Stance switching is disabled; kill XP still works.", STANCE_PLUGIN);
            return false;
        }

        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            logger::error("TESDataHandler unavailable.");
            return false;
        }

        bearSpell = dataHandler->LookupForm<RE::SpellItem>(BEAR_STANCE_ID, STANCE_PLUGIN);
        wolfSpell = dataHandler->LookupForm<RE::SpellItem>(WOLF_STANCE_ID, STANCE_PLUGIN);
        hawkSpell = dataHandler->LookupForm<RE::SpellItem>(HAWK_STANCE_ID, STANCE_PLUGIN);
        previousStanceGlobal = dataHandler->LookupForm<RE::TESGlobal>(PREVIOUS_STANCE_GLOBAL_ID, STANCE_PLUGIN);
        currentStanceGlobal = dataHandler->LookupForm<RE::TESGlobal>(CURRENT_STANCE_GLOBAL_ID, STANCE_PLUGIN);

        if (!bearSpell || !wolfSpell || !hawkSpell || !previousStanceGlobal || !currentStanceGlobal) {
            logger::error("Stance forms are missing from {}. Stance switching is disabled.", STANCE_PLUGIN);
            return false;
        }

        savageInstinctControlEffect = dataHandler->LookupForm<RE::EffectSetting>(SAVAGE_CONTROL_EFFECT_ID, STANCE_PLUGIN);
        savageInstinctSpell = dataHandler->LookupForm<RE::SpellItem>(SAVAGE_SPELL_ID, STANCE_PLUGIN);
        if (!savageInstinctControlEffect || !savageInstinctSpell) {
            logger::warn("Savage Instinct forms are missing; its kill bonus is disabled.");
        }

        bearPerk = dataHandler->LookupForm<RE::BGSPerk>(BEAR_PERK_ID, STANCE_PLUGIN);
        wolfPerk = dataHandler->LookupForm<RE::BGSPerk>(WOLF_PERK_ID, STANCE_PLUGIN);
        hawkPerk = dataHandler->LookupForm<RE::BGSPerk>(HAWK_PERK_ID, STANCE_PLUGIN);
        if (!bearPerk || !wolfPerk || !hawkPerk) {
            logger::warn("Aspect perks are missing; stances are not perk-gated.");
        }

        stanceSpells.clear();
        stanceSpells.emplace_back(bearSpell);
        stanceSpells.emplace_back(wolfSpell);
        stanceSpells.emplace_back(hawkSpell);

        ready = true;
        logger::info("Stance forms loaded.");
        return true;
    }
}
