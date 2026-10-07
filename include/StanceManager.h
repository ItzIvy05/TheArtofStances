#pragma once

namespace Stances
{
    enum class Stance : std::uint32_t
    {
        kNeutral = 0,
        kBear = 1,
        kWolf = 2,
        kHawk = 3,
    };

    class StanceManager final
    {
    public:
        [[nodiscard]] static Stance CurrentStance();
        [[nodiscard]] static const char* StanceName(Stance a_stance);
        [[nodiscard]] static bool IsStanceUnlocked(Stance a_stance);

        static void UpdateStance(Stance a_stance, RE::Actor* a_actor);
        static void UpdateStancePlayer(Stance a_stance);
        static void CycleStancesPlayer();
        static void HandlePlayerKill();
        static void NotifyStatsMenu(bool a_opening);

    private:
        [[nodiscard]] static RE::SpellItem* GetStanceSpell(Stance a_stance);
        static void ApplyStance(Stance a_stance, RE::Actor* a_actor);
        static void RemoveAllStances(RE::Actor* a_actor);
    };
}
