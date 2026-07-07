#pragma once

namespace Stances
{
    class Manager final
    {
    public:
        static Manager* GetSingleton();

        void OnDataLoaded();
        void OnNewGame();
        void OnPostLoadGame();

    private:
        Manager() = default;
    };
}
