#include "MCP.h"

#include "Settings.h"
#include "StanceInput.h"
#include "StanceManager.h"
#include "Widget.h"

#include <set>

#include <ClibUtil/hotkeys.hpp>
#include <ClibUtil/string.hpp>
#include <SKSEMCP/SKSEMenuFramework.hpp>

namespace Stances::MCP
{
    namespace
    {
        constexpr int BIND_BEAR = 0;
        constexpr int BIND_WOLF = 1;
        constexpr int BIND_HAWK = 2;
        constexpr int BIND_NEUTRAL = 3;
        constexpr int BIND_CYCLE = 4;

        std::atomic<int> s_captureTarget{ -1 };
        std::set<std::uint32_t> s_captureHeld;
        bool s_registered = false;

        void ApplyPattern(int a_target, const std::string& a_pattern)
        {
            const auto settings = Settings::GetSingleton();
            switch (a_target) {
            case BIND_BEAR:
                settings->SetBearKey(a_pattern);
                break;
            case BIND_WOLF:
                settings->SetWolfKey(a_pattern);
                break;
            case BIND_HAWK:
                settings->SetHawkKey(a_pattern);
                break;
            case BIND_NEUTRAL:
                settings->SetNeutralKey(a_pattern);
                break;
            case BIND_CYCLE:
                settings->SetCycleKey(a_pattern);
                break;
            default:
                return;
            }

            settings->Save();
            StanceInput::ApplyKeys();
        }

        [[nodiscard]] std::string PatternFromKeys(const std::set<std::uint32_t>& a_keys)
        {
            std::vector<std::string> parts;
            try {
                for (const auto key : a_keys) {
                    const auto name = clib_util::hotkeys::details::GetNameByKey(key);
                    parts.emplace_back(name == "numplus"sv ? "num+"sv : name);
                }
            } catch (...) {
                return {};
            }

            return clib_util::string::join(parts, "+");
        }

        void FinalizeCapture()
        {
            const auto target = s_captureTarget.exchange(-1);
            if (target < 0) {
                return;
            }

            const auto pattern = PatternFromKeys(s_captureHeld);
            s_captureHeld.clear();
            if (!pattern.empty() && StanceInput::IsValidPattern(pattern)) {
                ApplyPattern(target, pattern);
            }
        }

        bool __stdcall CaptureInput(RE::InputEvent* a_event)
        {
            if (s_captureTarget.load() < 0) {
                return false;
            }

            for (auto event = a_event; event; event = event->next) {
                const auto button = event->AsButtonEvent();
                if (!button || !button->HasIDCode()) {
                    continue;
                }

                const auto key = StanceInput::NormalizeKey(button);
                if (button->IsDown()) {
                    if (key == 1) {
                        s_captureHeld.clear();
                        s_captureTarget.store(-1);
                        return true;
                    }
                    s_captureHeld.insert(key);
                } else if (button->IsUp() && s_captureHeld.contains(key)) {
                    FinalizeCapture();
                    return true;
                }
            }

            return true;
        }

        [[nodiscard]] std::string PatternLabel(const std::string& a_pattern)
        {
            return a_pattern.empty() ? "Unbound" : a_pattern;
        }

        void DrawBindRow(int a_target, const char* a_label, const std::string& a_pattern, bool a_locked)
        {
            const bool capturing = s_captureTarget.load() == a_target;
            const auto buttonText = std::format(
                "{}##bind{}", capturing ? "Press keys..." : PatternLabel(a_pattern), a_target);
            if (ImGuiMCP::Button(buttonText.c_str(), ImGuiMCP::ImVec2(230.0f, 0.0f))) {
                s_captureHeld.clear();
                s_captureTarget.store(capturing ? -1 : a_target);
            }

            ImGuiMCP::SameLine();
            const auto unbindText = std::format("X##unbind{}", a_target);
            if (ImGuiMCP::Button(unbindText.c_str())) {
                s_captureHeld.clear();
                s_captureTarget.store(-1);
                ApplyPattern(a_target, "");
            }

            ImGuiMCP::SameLine();
            ImGuiMCP::Text("%s", a_label);

            if (a_locked) {
                ImGuiMCP::SameLine();
                ImGuiMCP::TextDisabled("(locked - take its Aspect perk)");
            }
        }

        void DrawHotkeys()
        {
            ImGuiMCP::TextDisabled("Click a binding, hold your combo, release to save. Esc cancels.");
            ImGuiMCP::Spacing();

            const auto settings = Settings::GetSingleton();
            DrawBindRow(BIND_BEAR, "Bear Stance", settings->BearKey(), !StanceManager::IsStanceUnlocked(Stance::kBear));
            DrawBindRow(BIND_WOLF, "Wolf Stance", settings->WolfKey(), !StanceManager::IsStanceUnlocked(Stance::kWolf));
            DrawBindRow(BIND_HAWK, "Hawk Stance", settings->HawkKey(), !StanceManager::IsStanceUnlocked(Stance::kHawk));
            DrawBindRow(BIND_NEUTRAL, "Neutral Stance", settings->NeutralKey(), false);
            DrawBindRow(BIND_CYCLE, "Cycle Stances", settings->CycleKey(), false);
            ImGuiMCP::TextDisabled("Cycle goes Bear > Wolf > Hawk and skips stances you have not unlocked.");
        }

        void DrawGeneral()
        {
            const auto settings = Settings::GetSingleton();

            bool applyOnStart = settings->ApplyStanceOnStart();
            if (ImGuiMCP::Checkbox("Apply Stance on Start", &applyOnStart)) {
                settings->SetApplyStanceOnStart(applyOnStart);
                settings->Save();
            }
        }

        void DrawHud()
        {
            const auto settings = Settings::GetSingleton();
            bool changed = false;

            bool show = settings->ShowWidget();
            if (ImGuiMCP::Checkbox("Show Stance Widget", &show)) {
                settings->SetShowWidget(show);
                changed = true;
            }

            int x = settings->HudX();
            if (ImGuiMCP::SliderInt("Widget X", &x, 0, 1280)) {
                settings->SetHudX(x);
                changed = true;
            }

            int y = settings->HudY();
            if (ImGuiMCP::SliderInt("Widget Y", &y, 0, 720)) {
                settings->SetHudY(y);
                changed = true;
            }

            int scale = settings->HudScale();
            if (ImGuiMCP::SliderInt("Widget Scale %", &scale, 25, 300)) {
                settings->SetHudScale(scale);
                changed = true;
            }

            bool autoHide = settings->AutoHideWidget();
            if (ImGuiMCP::Checkbox("Auto-Hide Widget", &autoHide)) {
                settings->SetAutoHideWidget(autoHide);
                changed = true;
            }

            if (autoHide) {
                int seconds = settings->WidgetSeconds();
                if (ImGuiMCP::SliderInt("Widget Popup Seconds", &seconds, Settings::kMinWidgetSeconds, Settings::kMaxWidgetSeconds)) {
                    settings->SetWidgetSeconds(seconds);
                    changed = true;
                }
            }

            if (changed) {
                settings->Save();
                Widget::Pop();
            }
        }

        void ResetDefaults()
        {
            s_captureHeld.clear();
            s_captureTarget.store(-1);

            const auto settings = Settings::GetSingleton();
            settings->SetApplyStanceOnStart(false);
            settings->SetBearKey("shift+x");
            settings->SetWolfKey("x");
            settings->SetHawkKey("ctrl+x");
            settings->SetNeutralKey("alt+v");
            settings->SetCycleKey("");
            settings->SetShowWidget(true);
            settings->SetHudX(64);
            settings->SetHudY(620);
            settings->SetHudScale(100);
            settings->SetAutoHideWidget(true);
            settings->SetWidgetSeconds(Settings::kDefaultWidgetSeconds);
            settings->Save();
            StanceInput::ApplyKeys();
            Widget::Pop();
        }

        void __stdcall Render()
        {
            if (ImGuiMCP::CollapsingHeader("Hotkeys", ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGuiMCP::Indent(20);
                DrawHotkeys();
                ImGuiMCP::Unindent(20);
            }

            ImGuiMCP::Spacing();
            if (ImGuiMCP::CollapsingHeader("General", ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGuiMCP::Indent(20);
                DrawGeneral();
                ImGuiMCP::Unindent(20);
            }

            ImGuiMCP::Spacing();
            if (ImGuiMCP::CollapsingHeader("HUD Widget", ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGuiMCP::Indent(20);
                DrawHud();
                ImGuiMCP::Unindent(20);
            }

            ImGuiMCP::Separator();
            if (ImGuiMCP::Button("Reset Stance Settings")) {
                ResetDefaults();
            }
        }
    }

    void Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::info("SKSE Menu Framework is not installed. In-game settings menu disabled.");
            return;
        }

        if (!menuFramework) {
            menuFramework = GetModuleHandleW(L"SKSEMenuFramework");
        }

        if (!menuFramework) {
            logger::warn("SKSEMenuFramework.dll exists but is not loaded. In-game settings menu disabled.");
            return;
        }

        SKSEMenuFramework::SetSection("The Art of Stances");
        SKSEMenuFramework::AddSectionItem("Stances", Render);
        SKSEMenuFramework::AddInputEvent(CaptureInput);
        s_registered = true;
        logger::info("Registered SKSE Menu Framework page.");
    }

    bool IsMenuBlocking()
    {
        if (s_captureTarget.load() >= 0) {
            return true;
        }

        return s_registered && SKSEMenuFramework::IsAnyBlockingWindowOpened();
    }
}
