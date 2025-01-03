#ifndef UI_COMPONENTS_SEARCH_BAR_H_
#define UI_COMPONENTS_SEARCH_BAR_H_

#include "imgui.h"
#include "ui/components/timer.hpp"
#include <cstdint>
#include <gsl/gsl>
#include <string_view>

namespace ui::components
{
    template<int64_t BufferSize = 1024>
    class SearchBar
    {
    public:
        typedef void Callback(std::string_view buffer_view);

        SearchBar(gsl::czstring label) : m_label(label)
        {
        }

        void render(
            const std::chrono::duration<double>& delta,
            std::function<Callback> callback
        )
        {
            m_timer.tick(delta);

            if (m_timer.has_expired()) {
                callback({m_buffer, m_buffer_size});
            }
            /* if (0ms > m_search_timeout) */
            /* { */
            /* 	m_search_timeout = 0ms; */

            /* 	callback({m_buffer, m_buffer_size}); */
            /* } */
            /* else if (m_search_timeout > 0ms) */
            /* { */
            /* 	m_search_timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(delta); */
            /* } */

            ImGui::InputText(
                m_label,
                m_buffer,
                BufferSize,
                ImGuiInputTextFlags_CallbackEdit,
                this->on_search_input,
                (void*) this
            );
        }

    private:
        static int on_search_input(ImGuiInputTextCallbackData* data)
        {
            SearchBar* sb = static_cast<SearchBar*>(data->UserData);

            /* sb->m_search_timeout = 300ms; */
            sb->m_timer.start();
            sb->m_buffer_size = data->BufTextLen;

            return 0;
        }

        char m_buffer[BufferSize] {""};
        size_t m_buffer_size {0};
        gsl::czstring m_label;
        Timer m_timer {};
    };
}

#endif // UI_COMPONENTS_SEARCH_BAR_H_
