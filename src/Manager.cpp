#include "Manager.h"
#include "Settings.h"
#include "StanceForms.h"
#include "StanceInput.h"
#include "StanceManager.h"
#include "Widget.h"

namespace Stances
{
    Manager* Manager::GetSingleton()
    {
        static Manager singleton;
        return std::addressof(singleton);
    }

    void Manager::OnDataLoaded()
    {
        Settings::GetSingleton()->Load();
        StanceForms::Load();
        StanceInput::ApplyKeys();
        Widget::Init();
        logger::info("Data loaded.");
    }

    void Manager::OnNewGame()
    {
        Settings::GetSingleton()->Load();
        StanceInput::ApplyKeys();
        StanceManager::ApplyDefaultStance();
        logger::info("New game detected.");
    }

    void Manager::OnPostLoadGame()
    {
        Settings::GetSingleton()->Load();
        StanceInput::ApplyKeys();
        StanceManager::ApplyDefaultStance();
        logger::info("Save loaded.");
    }
}
