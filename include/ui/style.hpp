#ifndef UI_STYLE_H_
#define UI_STYLE_H_

#include "imgui.h"

namespace ui
{
struct Style
{
    ImFont* font_normal{nullptr};
    ImFont* font_heading{nullptr};
};
} // namespace ui

#endif // UI_STYLE_H_
