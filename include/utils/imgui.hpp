#ifndef _UTILS_IMGUI_H_
#define _UTILS_IMGUI_H_

#include "imgui.h"
#include <string_view>

namespace ImGui
{
    inline void TextUnformatted(std::string_view text)
    {
        ImGui::TextUnformatted(text.data(), text.end());
    }
}

#endif // _UTILS_IMGUI_H_
