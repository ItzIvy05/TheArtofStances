#include "Manager.h"
#include "Settings.h"

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
        logger::info("Data loaded.");
    }

    void Manager::OnNewGame()
    {
        Settings::GetSingleton()->Load();
        logger::info("New game detected.");
    }

    void Manager::OnPostLoadGame()
    {
        Settings::GetSingleton()->Load();
        logger::info("Save loaded.");
    }
}
