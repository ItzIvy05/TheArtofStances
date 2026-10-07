#include "SettingsUI.h"

#include "Settings.h"
#include "StanceManager.h"
#include "Widget.h"

#include <FUCK_API.h>
#include <IconsFontAwesome6.h>

#include <Windows.h>
#include <shellapi.h>

namespace Stances::SettingsUI
{
    namespace
    {
        constexpr auto kGroup = "The Art of Stances";

        struct Bind
        {
            const char* label;
            REX::INI::U32<>* setting;
            std::uint32_t fallback;
            Stance stance;
            const char* help;
        };

        constexpr std::array kBinds{
            Bind{ "Bear Stance", &Settings::bearKey, Settings::kDefaultStanceKey, Stance::kBear, "Switches to Bear stance." },
            Bind{ "Wolf Stance", &Settings::wolfKey, Settings::kDefaultStanceKey, Stance::kWolf, "Switches to Wolf stance." },
            Bind{ "Hawk Stance", &Settings::hawkKey, Settings::kDefaultStanceKey, Stance::kHawk, "Switches to Hawk stance." },
            Bind{ "Neutral Stance", &Settings::neutralKey, Settings::kDefaultStanceKey, Stance::kNeutral, "Drops your current stance." },
            Bind{ "Cycle Stances", &Settings::cycleKey, Settings::kDefaultCycleKey, Stance::kNeutral, "Cycles Bear, Wolf and Hawk, skipping stances you have not unlocked." }
        };

        std::atomic<const Bind*> s_binding{ nullptr };

        [[nodiscard]] std::string VersionText()
        {
            const auto version = SKSE::GetPluginVersion();
            return std::format("{}.{}.{}", version.major(), version.minor(), version.patch());
        }

        void RestoreDefaults()
        {
            Settings::SetDefaults();
            Settings::Save();
            Widget::Pop();
        }

        // flick sizes these off their text, so they're measured as they're drawn
        float s_clearWidth = 0.0f;
        float s_helpWidth = 0.0f;

        [[nodiscard]] float Spacing()
        {
            return FUCK::GetStyleVarVec(ImGuiStyleVar_ItemSpacing).x;
        }

        // room between the (?) and the window edge, scaled like flick's own padding
        [[nodiscard]] float EdgePadding()
        {
            return FUCK::UIScale(8.0f);
        }

        // narrows the next widget so the (?) fits inside the window, returns where the (?) starts
        [[nodiscard]] float ReserveRow()
        {
            const float rowEnd = FUCK::GetCursorPos().x + FUCK::GetContentRegionAvail().x;
            FUCK::SetNextItemWidth(-(EdgePadding() + s_helpWidth + Spacing()));
            return rowEnd - EdgePadding() - s_helpWidth;
        }

        void Help(float a_x, const char* a_text)
        {
            FUCK::SameLine();
            FUCK::SetCursorPosX(a_x);
            FUCK::HelpMarker(a_text);
            s_helpWidth = FUCK::GetItemRectSize().x;
        }

        bool Section(const char* a_label)
        {
            return FUCK::CollapsingHeader(a_label, ImGuiTreeNodeFlags_DefaultOpen);
        }

        void Field(const char* a_label, const char* a_value)
        {
            FUCK::LeftLabel(a_label);
            FUCK::TextDisabled("%s", a_value);
        }

        // flick numbers controller buttons 266 higher than skse
        [[nodiscard]] std::uint32_t ToFlickKey(std::uint32_t a_key)
        {
            return a_key >= SKSE::InputMap::kMacro_GamepadOffset ? a_key + SKSE::InputMap::kMacro_GamepadOffset : a_key;
        }

        [[nodiscard]] std::uint32_t FromFlickKey(std::uint32_t a_key)
        {
            return a_key >= SKSE::InputMap::kMaxMacros ? a_key - SKSE::InputMap::kMacro_GamepadOffset : a_key;
        }

        void CancelBinding()
        {
            if (s_binding.exchange(nullptr)) {
                FUCK::AbortBinding();
            }
        }

        void DrawBind(const Bind& a_bind)
        {
            const bool binding = s_binding.load() == &a_bind;
            const auto label = binding ? std::string{ "Press any key..." } : std::format("{}{}", a_bind.label, StanceManager::IsStanceUnlocked(a_bind.stance) ? "" : "  (Locked)");
            const ModConfigUI::ButtonBinding key{ a_bind.setting->GetValue() };

            FUCK::PushID(a_bind.label);
            const float rowStart = FUCK::GetCursorPos().x;
            const float help = ReserveRow();
            // a bound key's hotkey draws its own label, so the blank one only finds flick's widget column
            FUCK::LeftLabel(key.IsSet() ? " " : label.c_str());

            // same column centre flick gives the checkboxes
            const float center = (FUCK::GetCursorPos().x + help - Spacing()) * 0.5f;
            // flick buttons pad their text 8px a side
            const float notBoundWidth = FUCK::CalcTextSize("Not Bound").x + FUCK::UIScale(16.0f);

            bool clicked = false;
            if (key.IsSet()) {
                FUCK::SetCursorPosX(rowStart);
                clicked = FUCK::Hotkey(std::format("{}###bind", label).c_str(), ToFlickKey(key.key), -1, -1, true, true, binding);
            } else {
                FUCK::SetCursorPosX(center - notBoundWidth * 0.5f);
                clicked = FUCK::Button("Not Bound");
            }

            if (clicked && !binding) {
                CancelBinding();
                s_binding = &a_bind;
                FUCK::StartBinding(key.IsSet() ? ToFlickKey(key.key) : 0, -1, -1, true);
            } else if (clicked) {
                CancelBinding();
            }

            // eraser centred in the gap between the key column and the (?)
            FUCK::SameLine();
            FUCK::SetCursorPosX((center + notBoundWidth * 0.5f + help - s_clearWidth) * 0.5f);
            if (FUCK::Button(ICON_FA_ERASER)) {
                CancelBinding();
                a_bind.setting->SetValue(ModConfigUI::ButtonBinding::NONE);
                Settings::Save();
            }
            s_clearWidth = FUCK::GetItemRectSize().x;
            FUCK::SetTooltip("Clears this hotkey.");
            Help(help, a_bind.help);
            FUCK::PopID();
        }

        void Toggle(const char* a_label, REX::INI::Bool<>& a_setting, const char* a_help)
        {
            const float help = ReserveRow();
            bool value = a_setting.GetValue();
            if (FUCK::Checkbox(a_label, &value)) {
                a_setting.SetValue(value);
                Settings::Save();
                Widget::Pop();
            }
            Help(help, a_help);
        }

        void Slider(const char* a_label, REX::INI::I32<>& a_setting, int a_min, int a_max, const char* a_help)
        {
            const float help = ReserveRow();
            int value = a_setting.GetValue();
            if (FUCK::SliderInt(a_label, &value, a_min, a_max)) {
                a_setting.SetValue(value);
                Widget::Pop();
            }
            if (FUCK::IsItemDeactivatedAfterEdit()) {
                Settings::Save();
            }
            Help(help, a_help);
        }

        class InfoPage final : public FUCK::ITool, public REX::Singleton<InfoPage>
        {
        public:
            const char* Name() const override
            {
                return "Mod Info";
            }

            const char* Group() const override
            {
                return kGroup;
            }

            void Draw() override
            {
                static const auto log = SKSE::log::log_directory().value_or(std::filesystem::path{}) / std::format("{}.log", SKSE::GetPluginName());
                FUCK::SeparatorText("Mod Info");
                Field("Name", kGroup);
                Field("Version", VersionText().c_str());
                Field("Author", SKSE::GetPluginAuthor().data());

                FUCK::SeparatorText("Resources");
                if (FUCK::Button("View Log File")) {
                    ShellExecuteW(nullptr, L"open", log.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
                FUCK::SetTooltip(log.string().c_str());
            }
        };

        class SettingsPage final : public FUCK::ITool, public REX::Singleton<SettingsPage>
        {
        public:
            const char* Name() const override
            {
                return "Settings";
            }

            const char* Group() const override
            {
                return kGroup;
            }

            void Draw() override
            {
                FUCK::Header(ICON_FA_GEAR "  Settings");

                if (Section(ICON_FA_KEYBOARD "  Hotkeys")) {
                    FUCK::Indent();
                    for (const auto& bind : kBinds) {
                        DrawBind(bind);
                    }
                    FUCK::Unindent();
                }

                if (Section(ICON_FA_DISPLAY "  HUD Widget")) {
                    FUCK::Indent();
                    Toggle("Show Stance Widget", Settings::showWidget, "Shows the stance icon on the HUD.");
                    Slider("Widget X", Settings::hudX, 0, 1280, "Moves the stance icon left and right.");
                    Slider("Widget Y", Settings::hudY, 0, 720, "Moves the stance icon up and down.");
                    Slider("Widget Scale %", Settings::hudScale, 25, 300, "Changes the size of the stance icon.");
                    Toggle("Auto-Hide Widget", Settings::autoHideWidget, "Shows the stance icon only for a few seconds after a stance change.");
                    if (Settings::autoHideWidget.GetValue()) {
                        Slider("Widget Popup Seconds", Settings::widgetSeconds, Settings::kMinWidgetSeconds, Settings::kMaxWidgetSeconds, "How long the stance icon stays on screen after a stance change.");
                    }
                    FUCK::Unindent();
                }

                if (FUCK::Button(ICON_FA_ROTATE_LEFT "  Reset")) {
                    CancelBinding();
                    RestoreDefaults();
                }
                FUCK::SetTooltip("Restores the default hotkeys and HUD widget settings.");
            }

            void OnClose() override
            {
                CancelBinding();
            }

            bool OnAsyncInput(const void* a_events) override
            {
                const auto bind = s_binding.load();
                if (!bind) {
                    return false;
                }

                std::uint32_t key = 0;
                std::int32_t mod1 = -1;
                std::int32_t mod2 = -1;
                const auto result = FUCK::UpdateBinding(a_events, &key, &mod1, &mod2);
                if (result == FUCK::BindResult::kBound) {
                    bind->setting->SetValue(FromFlickKey(key));
                    Settings::Save();
                }

                if (result != FUCK::BindResult::kNone) {
                    s_binding = nullptr;
                }

                return true;
            }
        };

        void FallbackBindRow(ModConfigUI::Renderer& a_renderer, const Bind& a_bind)
        {
            ModConfigUI::ButtonBinding binding{ a_bind.setting->GetValue() };
            if (a_renderer.BindButton(a_bind.label, &binding, ModConfigUI::ButtonBinding{ a_bind.fallback }, a_bind.help)) {
                a_bind.setting->SetValue(binding.key);
                Settings::Save();
            }

            a_renderer.SameLine();
            if (a_renderer.Button(std::format("Clear##{}", a_bind.label).c_str())) {
                a_bind.setting->SetValue(ModConfigUI::ButtonBinding::NONE);
                Settings::Save();
            }
            a_renderer.ItemTooltip("Clears this hotkey.");

            if (!StanceManager::IsStanceUnlocked(a_bind.stance)) {
                a_renderer.SameLine();
                a_renderer.TextDisabled("(Locked)");
            }
        }

        void FallbackToggle(ModConfigUI::Renderer& a_renderer, const char* a_label, REX::INI::Bool<>& a_setting, bool a_default, const char* a_help)
        {
            bool value = a_setting.GetValue();
            if (a_renderer.Checkbox(a_label, &value, a_default, a_help)) {
                a_setting.SetValue(value);
                Settings::Save();
                Widget::Pop();
            }
        }

        void FallbackSlider(ModConfigUI::Renderer& a_renderer, const char* a_label, REX::INI::I32<>& a_setting, std::int32_t a_min, std::int32_t a_max, std::int32_t a_default, const char* a_help)
        {
            std::int32_t value = a_setting.GetValue();
            const bool released = a_renderer.SliderInt(a_label, &value, a_min, a_max, "%d", a_default, a_help);
            if (value != a_setting.GetValue()) {
                a_setting.SetValue(value);
                Widget::Pop();
            }

            if (released) {
                Settings::Save();
            }
        }

        void DrawFallbackPage(ModConfigUI::Renderer& a_renderer)
        {
            a_renderer.SeparatorText("Hotkeys");
            for (const auto& bind : kBinds) {
                FallbackBindRow(a_renderer, bind);
            }

            a_renderer.SeparatorText("HUD Widget");
            FallbackToggle(a_renderer, "Show Stance Widget", Settings::showWidget, Settings::kDefaultShowWidget, "Shows the stance icon on the HUD.");
            FallbackSlider(a_renderer, "Widget X", Settings::hudX, 0, 1280, Settings::kDefaultHudX, "Moves the stance icon left and right.");
            FallbackSlider(a_renderer, "Widget Y", Settings::hudY, 0, 720, Settings::kDefaultHudY, "Moves the stance icon up and down.");
            FallbackSlider(a_renderer, "Widget Scale %", Settings::hudScale, 25, 300, Settings::kDefaultHudScale, "Changes the size of the stance icon.");
            FallbackToggle(a_renderer, "Auto-Hide Widget", Settings::autoHideWidget, Settings::kDefaultAutoHideWidget, "Shows the stance icon only for a few seconds after a stance change.");
            if (Settings::autoHideWidget.GetValue()) {
                FallbackSlider(a_renderer, "Widget Popup Seconds", Settings::widgetSeconds, Settings::kMinWidgetSeconds, Settings::kMaxWidgetSeconds, Settings::kDefaultWidgetSeconds, "How long the stance icon stays on screen after a stance change.");
            }
        }
    }

    void Register()
    {
        if (FUCK::Connect(SKSE::GetPluginName().data())) {
            FUCK::RegisterTool(InfoPage::GetSingleton());
            FUCK::RegisterTool(SettingsPage::GetSingleton());
            logger::info("Registered FLICK menu.");
            return;
        }

        static const std::string version = VersionText();
        static const ModConfigUI::ModInfo modInfo{ SKSE::GetPluginName().data(), kGroup, version.c_str(), SKSE::GetPluginAuthor().data(), nullptr, nullptr, nullptr };
        static constexpr ModConfigUI::Page pages[] = { { "Settings", DrawFallbackPage } };
        ModConfigUI::Install(modInfo, pages, RestoreDefaults);
    }
}
