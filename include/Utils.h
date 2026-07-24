#pragma once

namespace Stances::Utils
{
    [[nodiscard]] inline bool IsModLoaded(std::string_view a_modName)
    {
        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return false;
        }

        const auto file = dataHandler->LookupModByName(a_modName);
        return file && file->compileIndex != 0xFF;
    }

    [[nodiscard]] inline bool IsPermanentSpell(const RE::MagicItem* a_item)
    {
        switch (a_item->GetSpellType()) {
        case RE::MagicSystem::SpellType::kDisease:
        case RE::MagicSystem::SpellType::kAbility:
        case RE::MagicSystem::SpellType::kAddiction:
            return true;
        default:
            return false;
        }
    }

    inline void ApplySpell(RE::Actor* a_caster, RE::Actor* a_target, RE::SpellItem* a_spell)
    {
        if (IsPermanentSpell(a_spell)) {
            a_target->AddSpell(a_spell);
            return;
        }

        const auto caster = a_caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
        if (caster) {
            caster->CastSpellImmediate(a_spell, false, a_target, 1.0f, false, 0.0f, nullptr);
        }
    }
}
