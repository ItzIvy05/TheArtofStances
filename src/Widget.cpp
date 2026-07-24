#include "Widget.h"

#include "Settings.h"
#include "StanceForms.h"
#include "StanceManager.h"

#include <chrono>

namespace Stances::Widget
{
    namespace
    {
        constexpr const char* WIDGET_ROOT = "_root.widget";

        std::atomic_bool s_showQueued{ false };
        std::atomic_bool s_popupActive{ false };
        std::atomic<double> s_popupDeadline{ 0.0 };

        [[nodiscard]] double NowSeconds()
        {
            return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        class StancesWidgetMenu : public RE::IMenu
        {
        public:
            static constexpr const char* MENU_NAME = "StancesWidgetMenu";
            static constexpr const char* FILE_NAME = "exported/widgets/StancesHUD";

            StancesWidgetMenu()
            {
                depthPriority = 0;
                menuFlags.set(
                    RE::UI_MENU_FLAGS::kAlwaysOpen,
                    RE::UI_MENU_FLAGS::kAllowSaving,
                    RE::UI_MENU_FLAGS::kRequiresUpdate,
                    RE::UI_MENU_FLAGS::kAdvancesUnderPauseMenu);
                if (RE::BSScaleformManager::GetSingleton()->LoadMovie(this, uiMovie, FILE_NAME)) {
                    logger::info("Stance widget movie loaded.");
                } else {
                    logger::error("Could not load '{}.swf' into the stance widget menu.", FILE_NAME);
                }
                s_showQueued.store(false);
            }

            void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override
            {
                RE::IMenu::AdvanceMovie(a_interval, a_currentTime);
                Tick();
            }

            static RE::IMenu* Creator() { return new StancesWidgetMenu(); }
        };

        [[nodiscard]] RE::GPtr<RE::GFxMovieView> GetWidgetMovie()
        {
            const auto ui = RE::UI::GetSingleton();
            if (!ui || ui->IsMenuOpen("Loading Menu")) {
                return nullptr;
            }

            const auto menu = ui->GetMenu(StancesWidgetMenu::MENU_NAME);
            if (!menu || !menu->uiMovie) {
                return nullptr;
            }

            return menu->uiMovie;
        }

        void EnsureMenuShown()
        {
            const auto ui = RE::UI::GetSingleton();
            const auto queue = RE::UIMessageQueue::GetSingleton();
            if (!ui || !queue) {
                return;
            }

            if (ui->IsMenuOpen(StancesWidgetMenu::MENU_NAME)) {
                s_showQueued.store(false);
                return;
            }

            if (!ui->IsMenuOpen("HUD Menu")) {
                return;
            }

            if (s_showQueued.exchange(true)) {
                return;
            }

            queue->AddMessage(StancesWidgetMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }

        void HideMenu()
        {
            s_showQueued.store(false);
            const auto ui = RE::UI::GetSingleton();
            const auto queue = RE::UIMessageQueue::GetSingleton();
            if (!ui || !queue || !ui->IsMenuOpen(StancesWidgetMenu::MENU_NAME)) {
                return;
            }

            queue->AddMessage(StancesWidgetMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }

        void ForceRootVisible(RE::GFxMovieView* a_movie)
        {
            if (!a_movie) {
                return;
            }

            RE::GFxValue rootVisible;
            rootVisible.SetBoolean(true);
            a_movie->SetVariable("_root._visible", rootVisible);
        }

        void SetVar(RE::GFxMovieView* a_movie, const char* a_member, double a_value)
        {
            RE::GFxValue value;
            value.SetNumber(a_value);
            a_movie->SetVariable((std::string(WIDGET_ROOT) + a_member).c_str(), value);
        }

        void SetVarBool(RE::GFxMovieView* a_movie, const char* a_member, bool a_value)
        {
            RE::GFxValue value;
            value.SetBoolean(a_value);
            a_movie->SetVariable((std::string(WIDGET_ROOT) + a_member).c_str(), value);
        }

        [[nodiscard]] bool ShouldSuppressForMenus()
        {
            const auto player = RE::PlayerCharacter::GetSingleton();
            if (!player || !player->Is3DLoaded() || !player->GetParentCell()) {
                return true;
            }

            const auto ui = RE::UI::GetSingleton();
            if (!ui) {
                return true;
            }

            static constexpr std::array<std::string_view, 22> blockingMenus = {
                "BarterMenu", "Book Menu", "Console", "ContainerMenu", "Crafting Menu", "Dialogue Menu",
                "FavoritesMenu", "GiftMenu", "InventoryMenu", "Journal Menu", "Loading Menu", "Lockpicking Menu",
                "MagicMenu", "Main Menu", "MapMenu", "MessageBoxMenu", "RaceSex Menu", "Sleep/Wait Menu",
                "StatsMenu", "Training Menu", "TweenMenu", "Tutorial Menu"
            };
            for (const auto name : blockingMenus) {
                if (ui->IsMenuOpen(name.data())) {
                    return true;
                }
            }

            return false;
        }
    }

    void Init()
    {
        const auto ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("UI singleton unavailable; stance widget not registered.");
            return;
        }

        ui->Register(StancesWidgetMenu::MENU_NAME, StancesWidgetMenu::Creator);
        logger::info("Stance widget menu registered.");
    }

    void Refresh()
    {
        EnsureMenuShown();

        const auto movie = GetWidgetMovie();
        if (!movie) {
            return;
        }

        ForceRootVisible(movie.get());

        const auto settings = Settings::GetSingleton();
        const auto stance = StanceManager::CurrentStance();
        const bool popupOk = !settings->AutoHideWidget() || s_popupActive.load();
        const bool show = settings->ShowWidget() && StanceForms::IsReady() &&
                          stance != Stance::kNeutral && popupOk && !ShouldSuppressForMenus();

        SetVar(movie.get(), "._x", static_cast<double>(settings->HudX()));
        SetVar(movie.get(), "._y", static_cast<double>(settings->HudY()));
        const auto scale = std::clamp(settings->HudScale(), 10, 300);
        SetVar(movie.get(), "._xscale", static_cast<double>(scale));
        SetVar(movie.get(), "._yscale", static_cast<double>(scale));

        SetVarBool(movie.get(), ".bear._visible", stance == Stance::kBear);
        SetVarBool(movie.get(), ".wolf._visible", stance == Stance::kWolf);
        SetVarBool(movie.get(), ".hawk._visible", stance == Stance::kHawk);
        SetVarBool(movie.get(), "._visible", show);
        if (show) {
            SetVar(movie.get(), "._alpha", 100.0);
        }
    }

    void Pop()
    {
        s_popupDeadline.store(NowSeconds() + Settings::GetSingleton()->WidgetSeconds());
        s_popupActive.store(true);
        Refresh();
    }

    void Tick()
    {
        if (!Settings::GetSingleton()->AutoHideWidget()) {
            return;
        }

        if (s_popupActive.load() && NowSeconds() >= s_popupDeadline.load()) {
            s_popupActive.store(false);
            Refresh();
        }
    }

    void NotifyMenuEvent(std::string_view a_menuName, bool a_opening)
    {
        if (a_menuName == "HUD Menu") {
            if (a_opening) {
                EnsureMenuShown();
            } else {
                HideMenu();
            }
        }

        Refresh();
    }
}
