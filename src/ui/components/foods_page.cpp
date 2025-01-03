#include "ui/components/foods_page.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/style.hpp"
#include "utils/imgui.hpp"
#include "wholth/controller/foods_page.hpp"
#include "wholth/entity/food.hpp"
#include <cstdint>
#include <span>

using namespace std::chrono_literals;

static void render_food_nutrients(
    std::span<const wholth::entity::expanded::food::Nutrient> nutrients)
{
    ImGui::LabelText("##food_nutrients_label", "Nutrients");

    if (ImGui::BeginTable(
            "##food_nutrients",
            2,
            ImGuiTableFlags_NoBordersInBody |
                ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        for (const auto& n : nutrients)
        {
            if (n.title.size() <= 0)
            {
                break;
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(n.title);
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(n.value);

            ImGui::SameLine();

            ImGui::TextUnformatted(n.unit);
        }

        ImGui::EndTable();
    }
}

static void render_steps(
    std::span<const wholth::entity::expanded::food::RecipeStep> steps)
{
    if (steps.size() != 1)
    {
        return;
    }

    ImGui::LabelText("##food_steps_label", "Steps");
    ImGui::TextUnformatted(steps[0].time);
    ImGui::TextUnformatted(steps[0].description);
}

// @todo move to cpp-file
static int on_search_input(ImGuiInputTextCallbackData* data)
{
    wholth::controller::FoodsPage* f =
        static_cast<wholth::controller::FoodsPage*>(data->UserData);

    f->on_search(data->BufTextLen);

    return 0;
}

void ui::components::Foods::render(
    wholth::controller::FoodsPage& page,
    const std::chrono::duration<double>& delta,
    const ui::Style& style)
{
    int64_t color_pushes = 0;

    ImGui::Begin(
        "FoodsHeader",
        nullptr,
        ImGuiWindowFlags_NoCollapse
            /* | ImGuiWindowFlags_NoMove */
            /* | ImGuiWindowFlags_NoResize */
            | ImGuiWindowFlags_NoTitleBar
            /* | ImGuiWindowFlags_NoSavedSettings */
            | ImGuiWindowFlags_NoNav);

    ImGui::InputText(
        "Search",
        page.title_buffer(),
        page.sizeof_title_buffer(),
        ImGuiInputTextFlags_CallbackEdit,
        on_search_input,
        (void*)&page);

    ImGui::End();

    ImGui::Begin(
        "FoodsBody",
        nullptr,
        ImGuiWindowFlags_NoCollapse
            /* | ImGuiWindowFlags_NoMove */
            /* | ImGuiWindowFlags_NoResize */
            | ImGuiWindowFlags_NoTitleBar
            /* | ImGuiWindowFlags_NoSavedSettings */
            | ImGuiWindowFlags_NoNav);

    int i = 0;
    for (const auto& item : page.model().swappable_list.view_current().view)
    {
        i++;

        if (item.id.empty())
        {
            break;
        }

        ImGui::PushID(i);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushFont(style.font_heading);
        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(item.title);
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        ImGui::PopFont();

        if (ImGui::BeginTable(
                "##food_quick_data",
                2,
                ImGuiTableFlags_NoBordersInBody |
                    ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableSetupColumn("Prep. time");
            ImGui::TableSetupColumn("KCal");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(
                item.preparation_time.data(), item.preparation_time.end());

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(
                item.top_nutrient.data(), item.top_nutrient.end());

            ImGui::EndTable();
        }

        if (ImGui::Button("..."))
        {
            page.expand_food(item.id);
        }

        if (page.model().expanded_food.should_show &&
            page.model().expanded_food.food.id == item.id)
        {
            if (!page.model().expanded_food.food.description.empty())
            {
                ImGui::LabelText(
                    "##m_expanded_food.description", "Description");
                ImGui::TextUnformatted(page.model().expanded_food.food.description);
            }

            render_food_nutrients(page.model().expanded_food.nutrients);
            render_steps(page.model().expanded_food.steps);
        }

        ImGui::PopID();
    }

    ImGui::End();

    ImGui::Begin(
        "FoodsFooter",
        nullptr,
        ImGuiWindowFlags_NoCollapse
            /* | ImGuiWindowFlags_NoMove */
            /* | ImGuiWindowFlags_NoResize */
            | ImGuiWindowFlags_NoTitleBar
            /* | ImGuiWindowFlags_NoSavedSettings */
            | ImGuiWindowFlags_NoNav);

    if (ImGui::ArrowButtonEx(
            "##left", ImGuiDir_Left, {40, 40}, ImGuiButtonFlags_None))
    {
        page.retreat();
    }
    ImGui::SameLine();

    if (ImGui::ArrowButtonEx(
            "##right", ImGuiDir_Right, {40, 40}, ImGuiButtonFlags_None))
    {
        page.advance();
    }
    ImGui::SameLine();

    ImGui::TextUnformatted(page.model().page.pagination());
    ImGui::SameLine();

    if (page.model().is_fetching)
    {
        ImGui::TextUnformatted("...");
        ImGui::SameLine();
    }

    ImGui::PushID("#add_recipe");

    /* m_adder_modal.render(delta); */

    ImGui::PopID();

    if (color_pushes > 0)
    {
        ImGui::PopStyleColor(color_pushes);
    }

    ImGui::End();
}
