#pragma once

namespace Stances::Widget
{
    void Init();
    void Refresh();
    void Pop();
    void Tick();
    void NotifyMenuEvent(std::string_view a_menuName, bool a_opening);
}
