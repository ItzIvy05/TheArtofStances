#pragma once

namespace Stances::Settings
{
    inline constexpr const char* INI_PATH = "Data/SKSE/Plugins/Stances.ini";

    inline constexpr std::uint32_t kDefaultStanceKey = ModConfigUI::ButtonBinding::NONE;
    inline constexpr std::uint32_t kDefaultCycleKey = RE::BSKeyboardDevice::Key::kC;
    inline constexpr bool kDefaultShowWidget = true;
    inline constexpr int kDefaultHudX = 1237;
    inline constexpr int kDefaultHudY = 661;
    inline constexpr int kDefaultHudScale = 25;
    inline constexpr bool kDefaultAutoHideWidget = false;
    inline constexpr int kMinWidgetSeconds = 3;
    inline constexpr int kMaxWidgetSeconds = 15;
    inline constexpr int kDefaultWidgetSeconds = 10;

    inline REX::INI::F32<> skillUsePerKill{ "XP", "fSkillUsePerKill", 8.0f };
    inline REX::INI::Bool<> scaleWithVictimLevel{ "XP", "bScaleWithVictimLevel", true };
    inline REX::INI::F32<> victimLevelMult{ "XP", "fVictimLevelMult", 0.35f };
    inline REX::INI::U32<> bearKey{ "Keybinds", "iBearStanceKey", kDefaultStanceKey };
    inline REX::INI::U32<> wolfKey{ "Keybinds", "iWolfStanceKey", kDefaultStanceKey };
    inline REX::INI::U32<> hawkKey{ "Keybinds", "iHawkStanceKey", kDefaultStanceKey };
    inline REX::INI::U32<> neutralKey{ "Keybinds", "iNeutralStanceKey", kDefaultStanceKey };
    inline REX::INI::U32<> cycleKey{ "Keybinds", "iCycleStanceKey", kDefaultCycleKey };
    inline REX::INI::Bool<> showWidget{ "HUD", "bShowWidget", kDefaultShowWidget };
    inline REX::INI::I32<> hudX{ "HUD", "iHudX", kDefaultHudX };
    inline REX::INI::I32<> hudY{ "HUD", "iHudY", kDefaultHudY };
    inline REX::INI::I32<> hudScale{ "HUD", "iHudScale", kDefaultHudScale };
    inline REX::INI::Bool<> autoHideWidget{ "HUD", "bAutoHideWidget", kDefaultAutoHideWidget };
    inline REX::INI::I32<> widgetSeconds{ "HUD", "iWidgetSeconds", kDefaultWidgetSeconds };
    inline REX::INI::Bool<> debugLogging{ "Debug", "bEnableLogging", false };

    void Load();
    void Save();
    void SetDefaults();
    [[nodiscard]] float CalculateKillXP(const RE::Actor* a_victim);
}
