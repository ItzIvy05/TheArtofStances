#pragma once

namespace Stances::Utils
{
    template <class T>
    [[nodiscard]] T* As(RE::TESForm* a_form)
    {
        return a_form ? a_form->As<T>() : nullptr;
    }
}
