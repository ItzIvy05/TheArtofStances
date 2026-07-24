#pragma once

namespace Stances
{
    class StanceForms final
    {
    public:
        static bool Load();

        [[nodiscard]] static bool IsReady() noexcept { return ready; }

        static inline RE::SpellItem* bearSpell{ nullptr };
        static inline RE::SpellItem* wolfSpell{ nullptr };
        static inline RE::SpellItem* hawkSpell{ nullptr };
        static inline RE::TESGlobal* previousStanceGlobal{ nullptr };
        static inline RE::TESGlobal* currentStanceGlobal{ nullptr };
        static inline RE::EffectSetting* savageInstinctControlEffect{ nullptr };
        static inline RE::SpellItem* savageInstinctSpell{ nullptr };
        static inline RE::BGSPerk* bearPerk{ nullptr };
        static inline RE::BGSPerk* wolfPerk{ nullptr };
        static inline RE::BGSPerk* hawkPerk{ nullptr };
        static inline std::vector<RE::SpellItem*> stanceSpells;

    private:
        static inline bool ready{ false };
    };
}
