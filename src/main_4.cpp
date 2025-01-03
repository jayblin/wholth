#include <algorithm>
#include <array>
#include <atomic>
#include <cfenv>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gsl/assert>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>
#include <concepts>

/* #include "backends/imgui_impl_glfw.h" */
#include "backends/imgui_impl_vulkan.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "imgui_internal.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
/* #include "ui/components/foods_tab.hpp" */
#include "ui/components/foods_page.hpp"
#include "ui/platform.hpp"
#include "ui/style.hpp"
#include "utils/json_serializer.hpp"
#include "vk/device.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/physical_device.hpp"
#include "vk/swapchain.hpp"
#include "vk/render_pass.hpp"
#include "vk/vk.hpp"
#include "vulkan/vulkan.h"
/* #define GLFW_INCLUDE_VULKAN */
#include "GLFW/glfw3.h"
#include "db/db.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/concepts.hpp"
#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list/food.hpp"
#include "wholth/utils.hpp"
#include "vk/instance.hpp"

#include "imgui.h"

using namespace std::chrono_literals;

namespace vk
{
	VkAllocationCallbacks* allocators = nullptr;

	void check_silently(VkResult result, std::string_view msg = "")
	{
		if (VK_SUCCESS != result)
		{
            fmt::print(fmt::fg(fmt::color::crimson), "[Vulkan Error] [{}]\n", msg);
		}
	}
};

void setup_imgui_fonts(
    const vk::Device& device,
	VkQueue queue
) noexcept(false)
{
    if (!(device.command_pools.size() > 0)) {
        throw std::runtime_error("No command pools!");
    }

    if (!(device.command_buffers.size() > 0)) {
        throw std::runtime_error("No command buffers!");
    }

	VkCommandPool command_pool = device.command_pools[0];
	VkCommandBuffer command_buffer = device.command_buffers[0];

	vk::check(
		vkResetCommandPool(device.handle, command_pool, 0),
		"YUUUH"
	);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vk::check(
		vkBeginCommandBuffer(command_buffer, &begin_info),
		"NUUH"
	);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	vk::check(
		vkEndCommandBuffer(command_buffer),
		"BRRR"
	);

	vk::check(
		vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE),
		"SKRRR"
	);

	vk::check(
		vkDeviceWaitIdle(device.handle),
		"NYOWW"
	);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

uint32_t render_frame(
	const vk::Device& device,
    uint32_t sempahore_idx,
	/* VkSemaphore image_acquired_semaphore, */
	/* VkSemaphore render_complete_semaphore, */
	VkQueue queue,
	VkSurfaceCapabilitiesKHR& surface_capabilities,
	const VkClearValue* clear_value,
	ImDrawData* draw_data
)
{
	uint32_t frame_idx;

	vk::check_silently(
		vkAcquireNextImageKHR(
            device.handle,
            device.swapchain,
            UINT64_MAX,
            device.image_acquired_semaphores[sempahore_idx],
            VK_NULL_HANDLE,
            &frame_idx
        ),
		"vkAcquireNextImageKHR"
	);

    /* if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) */
    /* { */
    /*     g_SwapChainRebuild = true; */
    /*     return; */
    /* } */

	auto fence = device.fences[frame_idx];

	vk::check_silently(
		vkWaitForFences(
            device.handle,
            1,
            &fence,
            VK_TRUE, UINT64_MAX
        ),    // wait indefinitely instead of periodically checking
		"vkWaitForFences"
	);

	vk::check_silently(
		vkResetFences(device.handle, 1, &fence),
		"vkResetFences"
	);

	auto command_pool = device.command_pools[frame_idx];
	auto command_buffer = device.command_buffers[frame_idx];
	/* vk::check_silently( */
	/* 	vkResetCommandPool(device, command_pool, 0), */
	/* 	"vkResetCommandPool" */
	/* ); */
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vk::check_silently(
		vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info),
		"vkBeginCommandBuffer"
	);

	auto framebuffer = device.framebuffers[frame_idx];
	VkRect2D render_area = {
		.extent = surface_capabilities.currentExtent,
	};
	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = device.render_pass,
		.framebuffer = framebuffer,
		.renderArea = render_area,
		.clearValueCount = 1,
		.pClearValues = clear_value,
	};
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

    // Submit command buffer
    vkCmdEndRenderPass(command_buffer);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &device.image_acquired_semaphores[sempahore_idx],
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &device.render_complete_semaphores[sempahore_idx],
	};

	vk::check_silently(
		vkEndCommandBuffer(command_buffer),
		"vkEndCommandBuffer"
	);
	vk::check_silently(
		vkQueueSubmit(queue, 1, &submit_info, fence),
		"vkEndCommandBuffer"
	);

	return frame_idx;
}

static void present_frame(
	VkQueue queue,
	VkSwapchainKHR swapchain,
	std::span<VkSemaphore> render_complete_semaphores,
	uint32_t frame_idx,
	uint32_t semaphore_idx
)
{
    /* if (g_SwapChainRebuild) */
    /*     return; */
    VkPresentInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &render_complete_semaphores[semaphore_idx],
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &frame_idx,
	};
    VkResult err = vkQueuePresentKHR(queue, &info);

	vk::check_silently(err, "vkQueuePresentKHR");

    /* if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) */
    /* { */
    /*     g_SwapChainRebuild = true; */
    /*     return; */
    /* } */
}

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
    wholth::Context* ctx = static_cast<wholth::Context *>(pArg);
    fmt::print("{}\n", zMsg);
    // todo remake to array
    ctx->sql_errors.emplace_back(fmt::format("[{}] {}", iErrCode, zMsg));
}

/* static void glfw_error_callback(int error_code, const char* description) */
/* { */
/*     fmt::print(fmt::fg(fmt::color::red), "GLFW error [{}]: {}\n", error_code, description); */
/* } */

bool is_app_version_equal_db(sqlw::Connection& con) noexcept(false)
{
    using C = sqlw::status::Condition;
    bool are_versions_same = false;
    std::error_code ec = (sqlw::Statement {&con})
        .prepare("SELECT value FROM app_info WHERE field = 'version'")
        .exec([&are_versions_same](sqlw::Statement::ExecArgs e) { are_versions_same = e.column_value == WHOLTH_APP_VERSION; })
        .status();

    if (C::OK != ec) {
        throw std::system_error(ec);
    }

    return are_versions_same;
}

static std::filesystem::path get_db_filepath()
{
    std::filesystem::path path {DATABASE_DIR};
    path /= DATABASE_NAME;

    return path;
}

static bool does_db_file_exist() noexcept
{
    auto path = get_db_filepath();
    return std::filesystem::exists(path);
}

static void migrate(
    wholth::Context& ctx,
    sqlw::Connection& con
) noexcept(false)
{
    const auto list = db::migration::list_sorted_migrations(MIGRATIONS_DIR);

    ctx.migrate_result = db::migration::migrate({
        .con = &con,
        .migrations = list,
        .log = false,
    });
}

static void bump_app_version_in_db(sqlw::Connection& con) noexcept(false)
{
    auto ec = (sqlw::Statement {&con})
        .prepare(
            "INSERT OR REPLACE INTO app_info (field, value) "
            " VALUES ('version', :1)"
        )
        .bind(1, WHOLTH_APP_VERSION, sqlw::Type::SQL_TEXT)
        .exec()
        .status();

    if (sqlw::status::Condition::OK != ec) {
        throw std::system_error(ec);
    }
}

static sqlw::Connection connect_to_db() noexcept(false)
{
    using C = sqlw::status::Condition;

    auto path = get_db_filepath();
    sqlw::Connection con {path.string()};

    if (C::OK != con.status())
    {
        throw std::system_error(con.status());
    }

    return con;
}

static void setup_global_db_options(wholth::Context& ctx)
{
	sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, &ctx);
}

static void setup_db_instance_options(sqlw::Connection& con) noexcept(false)
{

    sqlw::Statement stmt {&con};
    std::error_code ec;

    ec = stmt("PRAGMA foreign_keys = ON");

    if (sqlw::status::Condition::OK != ec) {
        throw std::system_error{ec};
    }

    ec = stmt("PRAGMA automatic_index = OFF");

    if (sqlw::status::Condition::OK != ec) {
        throw std::system_error{ec};
    }

    sqlite3_create_function_v2(
        con.handle(),
        "seconds_to_readable_time",
        1,
        SQLITE_DETERMINISTIC | SQLITE_DIRECTONLY,
        nullptr,
        wholth::utils::sqlite::seconds_to_readable_time,
        nullptr,
        nullptr,
        nullptr
    );
}

// @todo test this workflow
static void setup_db(
    wholth::Context& ctx
) noexcept(false)
{
    sqlw::Connection con;
    std::error_code ec;

    if (!does_db_file_exist())
    {
        con = connect_to_db();

        ec = db::migration::make_migration_table(&con);

        if (db::status::Condition::OK != ec) {
            std::filesystem::remove(get_db_filepath());
            throw std::system_error(ec);
        }

        migrate(ctx, con);

        if (db::status::Condition::OK != ctx.migrate_result.error_code) {
            std::filesystem::remove(get_db_filepath());
            throw std::system_error(ctx.migrate_result.error_code);
        }

        bump_app_version_in_db(con);
    }
    else {
        con = connect_to_db();

        if (!is_app_version_equal_db(con)) {
            migrate(ctx, con);

            if (db::status::Condition::OK != ctx.migrate_result.error_code) {
                throw std::system_error(ctx.migrate_result.error_code);
            }

            bump_app_version_in_db(con);
        }
    }

    setup_db_instance_options(con);

    ctx.connection = std::move(con);
}

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ImGui::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;
    ImGui::ShowFontSelector("Fonts##Selector");

    // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    { bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder",  &border)) { style.FrameBorderSize  = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder",  &border)) { style.PopupBorderSize  = border ? 1.0f : 0.0f; } }

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();

    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Sizes"))
        {
            /* ImGui::SeparatorText("Main"); */
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");

            /* ImGui::SeparatorText("Borders"); */
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
            /* ImGui::SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f, 2.0f, "%.0f"); */

            /* ImGui::SeparatorText("Rounding"); */
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");

            /* ImGui::SeparatorText("Tables"); */
            ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
            /* ImGui::SliderAngle("TableAngledHeadersAngle", &style.TableAngledHeadersAngle, -50.0f, +50.0f); */

            /* ImGui::SeparatorText("Widgets"); */
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            /* ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content."); */
            ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            /* ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content."); */
            /* ImGui::SliderFloat("SeparatorTextBorderSize", &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.0f"); */
            /* ImGui::SliderFloat2("SeparatorTextAlign", (float*)&style.SeparatorTextAlign, 0.0f, 1.0f, "%.2f"); */
            /* ImGui::SliderFloat2("SeparatorTextPadding", (float*)&style.SeparatorTextPadding, 0.0f, 40.0f, "%.0f"); */
            ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");

            /* ImGui::SeparatorText("Docking"); */
            /* ImGui::SliderFloat("DockingSplitterSize", &style.DockingSeparatorSize, 0.0f, 12.0f, "%.0f"); */

            /* ImGui::SeparatorText("Tooltips"); */
            for (int n = 0; n < 2; n++)
                if (ImGui::TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse" : "HoverFlagsForTooltipNav"))
                {
                    /* ImGuiHoveredFlags* p = (n == 0) ? &style.HoverFlagsForTooltipMouse : &style.HoverFlagsForTooltipNav; */
                    /* ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNone", p, ImGuiHoveredFlags_DelayNone); */
                    /* ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayShort", p, ImGuiHoveredFlags_DelayShort); */
                    /* ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNormal", p, ImGuiHoveredFlags_DelayNormal); */
                    /* ImGui::CheckboxFlags("ImGuiHoveredFlags_Stationary", p, ImGuiHoveredFlags_Stationary); */
                    /* ImGui::CheckboxFlags("ImGuiHoveredFlags_NoSharedDelay", p, ImGuiHoveredFlags_NoSharedDelay); */
                    ImGui::TreePop();
                }

            /* ImGui::SeparatorText("Misc"); */
            ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f"); 
            /* ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured)."); */

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors"))
        {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (ImGui::Button("Export"))
            {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const ImVec4& col = style.Colors[i];
                    const char* name = ImGui::GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                        ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                ImGui::LogFinish();
            }
            ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = 0;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))             { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha",  alpha_flags == ImGuiColorEditFlags_AlphaPreview))     { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
            /* HelpMarker( */
            /*     "In the color list:\n" */
            /*     "Left-click on color square to open color picker,\n" */
            /*     "Right-click to open edit options menu."); */

            ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10), ImVec2(FLT_MAX, FLT_MAX));
            /* ImGui::BeginChild("##colors", ImVec2(0, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened); */
            ImGui::BeginChild("##colors", ImVec2(0, 0), 0, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fonts"))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontAtlas* atlas = io.Fonts;
            /* HelpMarker("Read FAQ and docs/FONTS.md for details on font loading."); */
            ImGui::ShowFontAtlas(atlas);

            // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            /* HelpMarker( */
            /*     "Those are old settings provided for convenience.\n" */
            /*     "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, " */
            /*     "rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n" */
            /*     "Using those settings here will give you poor quality results."); */
            static float window_scale = 1.0f;
            ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
            if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
                ImGui::SetWindowFontScale(window_scale);
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::SameLine();
            /* HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well."); */

            ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            ImGui::SameLine();
            /* HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering)."); */

            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

            // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
            ImGui::DragFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError , 0.005f, 0.10f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            /* const bool show_samples = ImGui::IsItemActive(); */
            /* if (show_samples) */
            /*     ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos()); */
            /* if (show_samples && ImGui::BeginTooltip()) */
            /* { */
            /*     ImGui::TextUnformatted("(R = radius, N = number of segments)"); */
            /*     ImGui::Spacing(); */
            /*     ImDrawList* draw_list = ImGui::GetWindowDrawList(); */
            /*     const float min_widget_width = ImGui::CalcTextSize("N: MMM\nR: MMM").x; */
            /*     for (int n = 0; n < 8; n++) */
            /*     { */
            /*         const float RAD_MIN = 5.0f; */
            /*         const float RAD_MAX = 70.0f; */
            /*         const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f); */

            /*         ImGui::BeginGroup(); */

            /*         ImGui::Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad)); */

            /*         const float canvas_width = IM_MAX(min_widget_width, rad * 2.0f); */
            /*         const float offset_x     = floorf(canvas_width * 0.5f); */
            /*         const float offset_y     = floorf(RAD_MAX); */

            /*         const ImVec2 p1 = ImGui::GetCursorScreenPos(); */
            /*         draw_list->AddCircle(ImVec2(p1.x + offset_x, p1.y + offset_y), rad, ImGui::GetColorU32(ImGuiCol_Text)); */
            /*         ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2)); */

            /*         /* */
            /*         const ImVec2 p2 = ImGui::GetCursorScreenPos(); */
            /*         draw_list->AddCircleFilled(ImVec2(p2.x + offset_x, p2.y + offset_y), rad, ImGui::GetColorU32(ImGuiCol_Text)); */
            /*         ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2)); */
            /*         *1/ */

            /*         ImGui::EndGroup(); */
            /*         ImGui::SameLine(); */
            /*     } */
            /*     ImGui::EndTooltip(); */
            /* } */
            ImGui::SameLine();
            /* HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically."); */

            ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            ImGui::DragFloat("Disabled Alpha", &style.DisabledAlpha, 0.005f, 0.0f, 1.0f, "%.2f");
            /* ImGui::SameLine(); HelpMarker("Additional alpha multiplier for disabled items (multiply over current value of Alpha)."); */
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}

void render_food_list(
    /* const std::chrono::duration<double>& delta, */
/* const ui::Style& style, */
std::span<const wholth::entity::shortened::Food> items
    /* std::function<void (const ui::Style& style, const wholth::entity::shortened::Food&)> render_item */
)
{
    /* m_timer.tick(delta); */

    /* if (!m_is_fetching_list && m_timer.has_expired()) */
    /* { */
    /*     fetch(m_query); */
    /* } */

}

int main()
{
    wholth::Context ctx;
    // todo redo
    ui::Style& ui_style = ctx.style;

    setup_global_db_options(ctx);

    // todo: move elsewhere
    /* glfwSetErrorCallback(glfw_error_callback); */

    vk::Instance instance_obj {};
    vk::Device device_obj {};

	const float height = 736.0f;
	const float width = 414.0f;
	/* const float height = 720.0f; */
	/* const float width = 1280.0f; */

	try {
        ui::PlatformFactory platform_factory {};
        auto platform = platform_factory.create();

        setup_db(ctx);
        sqlw::Connection& con = ctx.connection;
        
        ui::WindowFactory window_factory {};

        instance_obj.init();

        const ui::WindowOptions window_opts {
            .width = width,
            .height = height,
        };

		auto window = window_factory.create(window_opts);

        auto surface = instance_obj.create_surface(window);

		auto physical_devices = vk::physical_device::list(instance_obj.handle);

		auto physical_device_info = vk::physical_device::query_info(
            instance_obj.handle,
            physical_devices,
            surface
        );
		auto physical_device = std::get<VkPhysicalDevice>(physical_device_info);
        // @todo move QueueFamilies to physical_device namespace.
		auto queue_families = std::get<vk::device::QueueFamilies>(physical_device_info);

		VkSurfaceCapabilitiesKHR surface_capabilities;
		vk::check(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                physical_device,
                surface,
                &surface_capabilities
            ),
			"Couldn't get surface capabilities!"
		);

        device_obj.init(physical_device, queue_families.graphics, surface, surface_capabilities);

        VkQueue queue;
        vkGetDeviceQueue(
            device_obj.handle,
            queue_families.graphics.index,
            0,
            &queue
        );

		{
			VkSemaphoreTypeCreateInfo type_create_info {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
				/* .initialValue = , */
			};
			VkSemaphoreCreateInfo semaphore_create_info {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};
			/* vk::check( */
			/* vkCreateSemaphore(device, &semaphore_create_info, vk::allocators, &render_complete_semaphores[i]), */
			/* 	"Couldn't create semaphore" */
			/* ); */
		}

        ui::Imgui::implementation_detail_t impl_detail {window};
        ui::Imgui imgui {
            {
                instance_obj,
                physical_device,
                device_obj,
                queue_families.graphics,
                queue,
                surface_capabilities,
            },
            impl_detail
		};

		ImGuiIO& io = ImGui::GetIO();

        ImFontConfig font_config;
        font_config.OversampleH = 1; //or 2 is the same
        font_config.OversampleV = 1;
        font_config.PixelSnapH = 1;
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x044F, // Cyrillic
            0,
        };

        if (std::filesystem::exists(DEFAULT_FONT_PATH)) {
            ImFontAtlas* atlas = io.Fonts;
            ui_style.font_normal = atlas->AddFontFromFileTTF(DEFAULT_FONT_PATH, 16, &font_config, ranges);
            ui_style.font_heading = atlas->AddFontFromFileTTF(DEFAULT_FONT_PATH, 20, &font_config, ranges);
        }
        else {
            ui_style.font_normal = io.FontDefault;
            ui_style.font_heading = io.FontDefault;
        }

		setup_imgui_fonts(device_obj, queue);

		bool show_demo_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		VkClearValue clear_value;
		uint32_t semaphore_idx = 0;

        // https://colorhunt.co/palette/f0ebe3e4dccf7d9d9c576f72
        ImVec4* colors = ImGui::GetStyle().Colors;
        const ImVec4 main {0.94f, 0.92f, 0.89f, 1.00f}; // 240 235 227
        const ImVec4 secondary {0.89f, 0.86f, 0.81f, 1.00f}; // 228 220 207
        const ImVec4 accent {0.49f, 0.62f, 0.61f, 1.00f};
        const ImVec4 dark {0.34f, 0.44f, 0.45f, 1.00f};
        colors[ImGuiCol_Text]                   = dark;
        colors[ImGuiCol_TextDisabled]           = accent;
        colors[ImGuiCol_WindowBg]               = main;
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = main;
        colors[ImGuiCol_Border]                 = accent;
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = secondary;
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(secondary.x, secondary.y, secondary.z, 0.50f);
        colors[ImGuiCol_FrameBgActive]          = secondary;
        colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = secondary;
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(accent.x, accent.y, accent.z, 0.50f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(accent.x, accent.y, accent.z, 0.75f);
        colors[ImGuiCol_ScrollbarGrabActive]    = accent;
        colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(accent.x, accent.y, accent.z, 0.50f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(accent.x, accent.y, accent.z, 0.75f);
        colors[ImGuiCol_ButtonActive]           = accent;
        colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered]          = secondary;
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]              = dark;
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab]                    = secondary;
        colors[ImGuiCol_TabHovered]             = ImVec4(accent.x, accent.y, accent.z, 0.75f);
        colors[ImGuiCol_TabActive]              = ImVec4(accent.x, accent.y, accent.z, 0.50f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(secondary.x, secondary.y, secondary.z,  0.50f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]          = ImVec4(secondary.x, secondary.y, secondary.z,  0.50f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f; 
        style.WindowBorderSize = 0.0f;
        style.FrameRounding = 6.0f;

		auto now {std::chrono::steady_clock{}.now()};
		auto start {now};
		auto end {now};

        ui::components::Foods foods_tab;
        ctx.foods_page_ctrl.fetch(ctx.locale_id(), ctx.connection);

		while (!window.should_close())
		{
			const std::chrono::duration<double> delta {end - start};

			start = std::chrono::steady_clock{}.now();

            ctx.update(delta);

			/* glfwPollEvents(); */
            platform.handle_events();

            imgui.new_frame(width, height);

            ImGui::PushFont(ui_style.font_normal);

			/* ImGui_ImplVulkan_NewFrame(); */
			/* ImGui_ImplGlfw_NewFrame(); */
			/* io.DisplaySize.x = width;             // set the current display width */
			/* io.DisplaySize.y = height;             // set the current display height here */
			/* ImGui::NewFrame(); */

			/* if (show_demo_window) */
			/* { */
				/* ImGui::ShowDemoWindow(&show_demo_window); */
			/* } */

			/* { */
			/* 	static float f = 0.0f; */
			/* 	static int counter = 0; */

			/* 	ImGui::Begin("Window Title"); */

			/* 	// Display some text (you can use a format strings too) */
			/* 	ImGui::Text("This is some useful text."); */
			/* 	// Edit bools storing our window open/close state */
			/* 	ImGui::Checkbox("Demo Window", &show_demo_window); */

			/* 	// Edit 1 float using a slider from 0.0f to 1.0f */
			/* 	ImGui::SliderFloat("float", &f, 0.0f, 1.0f); */
			/* 	// Edit 3 floats representing a color */
			/* 	ImGui::ColorEdit3("clear color", (float*)&clear_color); */

			/* 	// Buttons return true when clicked (most widgets return true when edited/activated) */
			/* 	if (ImGui::Button("Button")) */
			/* 	{ */
			/* 		counter++; */
			/* 	} */

			/* 	ImGui::SameLine(); */
			/* 	ImGui::Text("counter = %d", counter); */

			/* 	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); */
			/* 	ImGui::End(); */
			/* } */

            /* ImGui::ShowStyleEditor(); */

			{
                const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(
                    main_viewport->Pos,
                    ImGuiCond_FirstUseEver
                );
                ImGui::SetNextWindowSize(
                    main_viewport->Size,
                    ImGuiCond_FirstUseEver
                );

                foods_tab.render(ctx.foods_page_ctrl, delta, ctx.style);

                ImGui::PopFont();

                /* ImGui::Begin("pagination!"); */

                /* ImGui::End(); */
			}

			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();

            clear_value.color.float32[0] = clear_color.x * clear_color.w;
            clear_value.color.float32[1] = clear_color.y * clear_color.w;
            clear_value.color.float32[2] = clear_color.z * clear_color.w;
            clear_value.color.float32[3] = clear_color.w;

            auto frame_idx = render_frame(
				device_obj,
                semaphore_idx,
				queue,
				surface_capabilities,
				&clear_value,
				draw_data
			);

            present_frame(
				queue,
				device_obj.swapchain,
				device_obj.render_complete_semaphores,
				frame_idx,
				semaphore_idx
			);

			semaphore_idx = (semaphore_idx + 1) % device_obj.images.size();

			end = std::chrono::steady_clock{}.now();
		}
	}
    catch (const std::system_error& err)
    {
        ctx.exception_message = fmt::format(
            "Caught an exception: {} [{}] {}\n",
            err.code().category().name(),
            err.code().value(),
            err.what()
        );
    }

    std::string serialized_ctx = (utils::JsonSerializer {}).serialize(ctx);
    fmt::print(
        fmt::fg(fmt::color::orange),
        "{}\n",
        serialized_ctx
    );

	vk::check_silently(
		vkDeviceWaitIdle(device_obj.handle),
		"vkDeviceWaitIdle"
	);

	return 0;
}
