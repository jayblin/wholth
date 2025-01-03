#ifndef UI_STYLE_H_
#define UI_STYLE_H_

#include "imgui.h"
#include "utils/serializer.hpp"

namespace ui
{
struct Style
{
    ImFont* font_normal{nullptr};
    ImFont* font_heading{nullptr};

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(font_normal->ConfigData->Name)
                   << NVP(font_heading->ConfigData->Name);
    }
};
} // namespace ui

#endif // UI_STYLE_H_
