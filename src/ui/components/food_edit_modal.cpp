#include "ui/components/food_edit_modal.hpp"
#include "utils/imgui.hpp"
#include <chrono>

void ui::components::FoodEditModal::render(const std::chrono::duration<double>& delta)
{
    /* if (ImGui::Button("Add")) { */
    /*     ImGui::OpenPopup("Add recipe"); */
    /* } */

    /* ImVec2 center = ImGui::GetMainViewport()->GetCenter(); */
    /* ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)); */

    /* if (ImGui::BeginPopupModal( */
    /*     "Add recipe", */
    /*     nullptr, */
    /*     ImGuiWindowFlags_NoTitleBar */
    /* )) { */
    /*     ImGui::InputText( */
    /*         "Title", */
    /*         m_title_buffer, */
    /*         255 */
    /*     ); */
    /*     ImGui::InputTextMultiline( */
    /*         "Description", */
    /*         m_description_buffer, */
    /*         1024, */
    /*         ImVec2{0, 0}, */
    /*         ImGuiInputTextFlags_AllowTabInput */
    /*     ); */
    /*     ImGui::InputTextMultiline( */
    /*         "Recipe", */
    /*         m_recipe_buffer, */
    /*         1024 * 10, */
    /*         ImVec2{0, 0}, */
    /*         ImGuiInputTextFlags_AllowTabInput */
    /*     ); */
    /*     ImGui::PushItemWidth(80.0f); */
    /*     uint8_t step = 1; */
    /*     uint8_t step_fast = 5; */
    /*     ImGui::InputScalar( */
    /*         "Hours", */
    /*         ImGuiDataType_U8, */
    /*         &m_hours, */
    /*         &step, */
    /*         &step_fast */
    /*     ); */
    /*     ImGui::InputScalar( */
    /*         "Minutes", */
    /*         ImGuiDataType_U8, */
    /*         &m_minutes, */
    /*         &step, */
    /*         &step_fast */
    /*     ); */
    /*     ImGui::InputScalar( */
    /*         "Seconds", */
    /*         ImGuiDataType_U8, */
    /*         &m_seconds, */
    /*         &step, */
    /*         &step_fast */
    /*     ); */
    /*     ImGui::PopItemWidth(); */

    /*     m_search_bar.render( */
    /*         delta, */
    /*         [&] (std::string_view buffer_view) { */
    /*             m_ingredient_list.fetch({ */
    /*                 .page = 0, */
    /*                 // todo locale_id */
    /*                 .locale_id = m_ctx.locale_id.ref(), */
    /*                 .title = buffer_view */
    /*             }); */
    /*         } */
    /*     ); */

    /*     m_ingredient_list.render( */
    /*         delta, */
    /*         [&](wholth::entity::shortened::Food item) { */
    /*             fmt::print("{}\n", item.title); */
    /*             ImGui::TextUnformatted(item.title); */ 
    /*         } */
    /*     ); */

    /*     if (ImGui::Button("Save")) { */
    /*         std::string result_id; */
    /*         auto rc = wholth::insert_food( */
    /*             { */
    /*                 .title = m_title_buffer, */
    /*                 .description = m_description_buffer, */
    /*             }, */
    /*             result_id, */
    /*             m_ctx.connection, */
    /*             // todo: THIS SUCKS */
    /*             /1* query.locale_id *1/ */
    /*             m_ctx.locale_id.ref() */
    /*         ); */

    /*         const uint64_t total_seconds = m_seconds + m_minutes * 60 + m_hours * 60 * 60; */
    /*         const auto str_seconds = std::to_string(total_seconds); */
    /*         const wholth::entity::editable::food::RecipeStep recipe_step { */
    /*             .seconds = str_seconds, */
    /*             .description = m_recipe_buffer, */
    /*         }; */
    /*         std::array<wholth::entity::editable::food::RecipeStep, 1> steps {recipe_step}; */
    /*         // todo */ 
    /*         /1* if (!rc) { *1/ */
    /*         /1*     //popup *1/ */
    /*         /1* } *1/ */

    /*         // todo add step's field validation. */
    /*         wholth::add_steps( */
    /*             result_id, */
    /*             steps, */
    /*             m_ctx.connection, */
    /*             // todo: THIS SUCKS */
    /*             /1* query.locale_id *1/ */
    /*             m_ctx.locale_id.ref() */
    /*         ); */
    /*         /1* wholth::add_ingredients( *1/ */
    /*         /1*     wholth::entity::recipe_step::id_t, *1/ */
    /*         /1*     const std::span<const wholth::entity::editable::food::Ingredient>, *1/ */
    /*         /1*     sqlw::Connection & *1/ */
    /*         /1* ); *1/ */
    /*         ImGui::CloseCurrentPopup(); */
    /*     } */
    /*     ImGui::SameLine(); */

    /*     if (ImGui::Button("Close")) { */
    /*         ImGui::CloseCurrentPopup(); */
    /*     } */

    /*     ImGui::EndPopup(); */
    /* } */
}
