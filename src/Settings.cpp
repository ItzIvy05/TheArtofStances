#include "Settings.h"

namespace Stances::Settings
{
    void Load()
    {
        if (!std::filesystem::exists(INI_PATH)) {
            logger::warn("{} not found. Using default Stances settings.", INI_PATH);
        }

        const auto ini = REX::INI::SettingStore::GetSingleton();
        ini->Init(INI_PATH, "");
        ini->Load();

        widgetSeconds.SetValue(std::clamp(widgetSeconds.GetValue(), kMinWidgetSeconds, kMaxWidgetSeconds));

        spdlog::set_level(debugLogging.GetValue() ? spdlog::level::info : spdlog::level::warn);

        logger::info("Settings loaded. skillUsePerKill={} scaleWithVictimLevel={} victimLevelMult={} debugLogging={}", skillUsePerKill.GetValue(), scaleWithVictimLevel.GetValue(), victimLevelMult.GetValue(), debugLogging.GetValue());
    }

    void Save()
    {
        REX::INI::SettingStore::GetSingleton()->Save();
    }

    void SetDefaults()
    {
        bearKey.SetValue(kDefaultStanceKey);
        wolfKey.SetValue(kDefaultStanceKey);
        hawkKey.SetValue(kDefaultStanceKey);
        neutralKey.SetValue(kDefaultStanceKey);
        cycleKey.SetValue(kDefaultCycleKey);
        showWidget.SetValue(kDefaultShowWidget);
        hudX.SetValue(kDefaultHudX);
        hudY.SetValue(kDefaultHudY);
        hudScale.SetValue(kDefaultHudScale);
        autoHideWidget.SetValue(kDefaultAutoHideWidget);
        widgetSeconds.SetValue(kDefaultWidgetSeconds);
    }

    float CalculateKillXP(const RE::Actor* a_victim)
    {
        float result = skillUsePerKill.GetValue();

        if (scaleWithVictimLevel.GetValue() && a_victim) {
            result += static_cast<float>(a_victim->GetLevel()) * victimLevelMult.GetValue();
        }

        return std::max(result, 0.0f);
    }
}
