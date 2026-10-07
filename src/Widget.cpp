#include "Widget.h"

#include "Settings.h"
#include "StanceForms.h"
#include "StanceManager.h"

namespace Stances::Widget
{
    namespace
    {
        std::atomic_bool s_showQueued{ false };
        std::atomic_bool s_popupActive{ false };
        std::atomic<std::uint32_t> s_popupDeadline{ 0 };

        class StancesWidgetMenu : public RE::IMenu
        {
        public:
            static constexpr const char* MENU_NAME = "StancesWidgetMenu";
            static constexpr const char* FILE_NAME = "exported/widgets/StancesHUD";

            StancesWidgetMenu()
            {
                depthPriority = 0;
                menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen, RE::UI_MENU_FLAGS::kAllowSaving, RE::UI_MENU_FLAGS::kRequiresUpdate, RE::UI_MENU_FLAGS::kAdvancesUnderPauseMenu);
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

            static RE::IMenu* Creator()
            {
                return new StancesWidgetMenu();
            }
        };

        [[nodiscard]] RE::GPtr<RE::GFxMovieView> GetWidgetMovie()
        {
            const auto ui = RE::UI::GetSingleton();
            if (!ui || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
                return nullptr;
            }

            return ui->GetMovieView(StancesWidgetMenu::MENU_NAME);
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

            if (!ui->IsMenuOpen(RE::HUDMenu::MENU_NAME)) {
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
                RE::BarterMenu::MENU_NAME,
                RE::BookMenu::MENU_NAME,
                RE::Console::MENU_NAME,
                RE::ContainerMenu::MENU_NAME,
                RE::CraftingMenu::MENU_NAME,
                RE::DialogueMenu::MENU_NAME,
                RE::FavoritesMenu::MENU_NAME,
                RE::GiftMenu::MENU_NAME,
                RE::InventoryMenu::MENU_NAME,
                RE::JournalMenu::MENU_NAME,
                RE::LoadingMenu::MENU_NAME,
                RE::LockpickingMenu::MENU_NAME,
                RE::MagicMenu::MENU_NAME,
                RE::MainMenu::MENU_NAME,
                RE::MapMenu::MENU_NAME,
                RE::MessageBoxMenu::MENU_NAME,
                RE::RaceSexMenu::MENU_NAME,
                RE::SleepWaitMenu::MENU_NAME,
                RE::StatsMenu::MENU_NAME,
                RE::TrainingMenu::MENU_NAME,
                RE::TweenMenu::MENU_NAME,
                RE::TutorialMenu::MENU_NAME
            };
            for (const auto name : blockingMenus) {
                if (ui->IsMenuOpen(name)) {
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

        movie->SetVariable("_root._visible", RE::GFxValue{ true });

        const auto stance = StanceManager::CurrentStance();
        const bool popupOk = !Settings::autoHideWidget.GetValue() || s_popupActive.load();
        const bool show = Settings::showWidget.GetValue() && StanceForms::IsReady() && stance != Stance::kNeutral && popupOk && !ShouldSuppressForMenus();

        movie->SetVariable("_root.widget._x", RE::GFxValue{ Settings::hudX.GetValue() });
        movie->SetVariable("_root.widget._y", RE::GFxValue{ Settings::hudY.GetValue() });
        const auto scale = std::clamp(Settings::hudScale.GetValue(), 10, 300);
        movie->SetVariable("_root.widget._xscale", RE::GFxValue{ scale });
        movie->SetVariable("_root.widget._yscale", RE::GFxValue{ scale });

        movie->SetVariable("_root.widget.bear._visible", RE::GFxValue{ stance == Stance::kBear });
        movie->SetVariable("_root.widget.wolf._visible", RE::GFxValue{ stance == Stance::kWolf });
        movie->SetVariable("_root.widget.hawk._visible", RE::GFxValue{ stance == Stance::kHawk });
        movie->SetVariable("_root.widget._visible", RE::GFxValue{ show });
        if (show) {
            movie->SetVariable("_root.widget._alpha", RE::GFxValue{ 100.0 });
        }
    }

    void Pop()
    {
        s_popupDeadline.store(RE::GetDurationOfApplicationRunTime() + static_cast<std::uint32_t>(Settings::widgetSeconds.GetValue() * 1000));
        s_popupActive.store(true);
        Refresh();
    }

    void Tick()
    {
        if (!Settings::autoHideWidget.GetValue()) {
            return;
        }

        if (s_popupActive.load() && RE::GetDurationOfApplicationRunTime() >= s_popupDeadline.load()) {
            s_popupActive.store(false);
            Refresh();
        }
    }

    void NotifyMenuEvent(std::string_view a_menuName, bool a_opening)
    {
        if (a_menuName == RE::HUDMenu::MENU_NAME) {
            if (a_opening) {
                EnsureMenuShown();
            } else {
                HideMenu();
            }
        }

        Refresh();
    }
}
