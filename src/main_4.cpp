#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <vector>
#include <concepts>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "ui/glfw_window.hpp"
#include "ui/imgui.hpp"
#include "utils/json_serializer.hpp"
#include "vk/device.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/physical_device.hpp"
#include "vk/swapchain.hpp"
#include "vk/render_pass.hpp"
#include "vk/vk.hpp"
#include "ui/window.hpp"
#include "vulkan/vulkan.h"
/* #define GLFW_INCLUDE_VULKAN */
#include "GLFW/glfw3.h"
#include "db/db.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list/food.hpp"
#include "wholth/utils.hpp"
#include "vk/instance.hpp"

#include "imgui.h"

#include "gsl/gsl"

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

template<size_t BufferSize>
class SearchBar
{
public:
	typedef void Callback(std::string_view buffer_view);

	void render(
		const std::chrono::duration<double>& delta,
		gsl::czstring label,
		std::function<Callback> callback
	)
	{
		if (0ms > m_search_timeout)
		{
			m_search_timeout = 0ms;

			callback({m_buffer, m_buffer_size});
		}
		else if (m_search_timeout > 0ms)
		{
			m_search_timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(delta);
		}

		ImGui::InputText(
			label,
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

		sb->m_search_timeout = 300ms;
		sb->m_buffer_size = data->BufTextLen;

		return 0;
	}

	char m_buffer[BufferSize] {""};
	size_t m_buffer_size {0};
	std::chrono::milliseconds m_search_timeout {0ms};
};

template<typename T>
class Table
{
public:
	Table(gsl::czstring table_name): m_table_name(table_name)
	{
		/* static_assert(is_renderable<Table<T>>); */
	}

	void render(
		const std::chrono::duration<double>& delta,
		const std::span<const T>& items
	)
	{
		if (ImGui::BeginTable(m_table_name, std::tuple_size_v<T>))
		{
			for (const auto& item : items)
			{
				int i = 0;
				ImGui::TableNextRow();

				std::apply(
					[&i](auto&& ...args) {
						([&i, &args]{

							ImGui::TableSetColumnIndex(i++);
							ImGui::TextUnformatted(args.begin(), args.end());
						 }(), ...);
					},
					item
				);
			}

			ImGui::EndTable();
		}
	}

private:
	const gsl::czstring m_table_name;
};

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
    wholth::Context* ctx = static_cast<wholth::Context *>(pArg);
    // todo remake to array
    ctx->sql_errors.emplace_back(fmt::format("[{}] {}", iErrCode, zMsg));
}

static void glfw_error_callback(int error_code, const char* description)
{
    fmt::print(fmt::fg(fmt::color::red), "GLFW error [{}]: {}\n", error_code, description);
}

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
) noexcept
{
    ctx.migrate_result = db::migration::migrate({
        .con = &con,
        .migrations_dir = MIGRATIONS_DIR,
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

    stmt("PRAGMA foreign_keys = ON");
    ec = stmt.status();

    if (sqlw::status::Condition::OK != ec) {
        throw std::system_error{ec};
    }

    stmt("PRAGMA automatic_index = OFF");
    ec = stmt.status();

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
static sqlw::Connection setup_db(
    wholth::Context& ctx
) noexcept(false)
{
    sqlw::Connection con;

    if (!does_db_file_exist())
    {
        con = connect_to_db();
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

    return con;
}

int main()
{
    wholth::Context ctx;

    setup_global_db_options(ctx);

    // todo: move elsewhere
    glfwSetErrorCallback(glfw_error_callback);

    vk::Instance instance_obj {};
    vk::Device device_obj {};

	const float width = 1280.0f;
	const float height = 720.0f;

	try {
        sqlw::Connection con = setup_db(ctx);
        
        ui::WindowFactory<ui::GlfwWindow> window_factory {};

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

        ui::Imgui imgui {
			instance_obj,
			physical_device,
            device_obj,
			queue_families.graphics,
			queue,
			surface_capabilities,
			window,
		};

		setup_imgui_fonts(device_obj, queue);

		bool show_demo_window = true;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		VkClearValue clear_value;
		uint32_t semaphore_idx = 0;
		ImGuiIO& io = ImGui::GetIO();

		/* SearchBar search_bar {"#search_bar"}; */
		/* RecipesTable recipes_table {&con}; */
		/* CookingActionsTable cooking_actions_table {&con}; */

        std::string food_list_buffer;
		std::array<wholth::entity::shortened::Food, 20> food_list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "1",
		};
		wholth::list::food::Lister lister {q, &con};

		wholth::StatusCode rc = lister.list(
			food_list,
			food_list_buffer
		);

		/* typedef std::tuple<std::string_view, std::string_view, std::string_view> _Food; */
		
		/* Table<_Food> idk_table {"idk_table"}; */

		/* std::vector<_Food> _foods { */
		/* 	{"1", "bob 1", "123"}, */
		/* 	{"2", "bob 2", "125"}, */
		/* 	{"3", "bob 3", "127"}, */
		/* 	{"4", "bob 4", "131"}, */
		/* }; */

		auto now {std::chrono::steady_clock{}.now()};
		auto start {now};
		auto end {now};

		while (!window.should_close())
		{
			const std::chrono::duration<double> delta {end - start};

			start = std::chrono::steady_clock{}.now();

			glfwPollEvents();

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			io.DisplaySize.x = width;             // set the current display width
			io.DisplaySize.y = height;             // set the current display height here
			ImGui::NewFrame();

			if (show_demo_window)
			{
				ImGui::ShowDemoWindow(&show_demo_window);
			}

			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Window Title");

				// Display some text (you can use a format strings too)
				ImGui::Text("This is some useful text.");
				// Edit bools storing our window open/close state
				ImGui::Checkbox("Demo Window", &show_demo_window);

				// Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
				// Edit 3 floats representing a color
				ImGui::ColorEdit3("clear color", (float*)&clear_color);

				// Buttons return true when clicked (most widgets return true when edited/activated)
				if (ImGui::Button("Button"))
				{
					counter++;
				}

				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			{
				ImGui::Begin("Wholth");

				if (ImGui::BeginTabBar("WholthTabBar"))
				{
					/* if (ImGui::BeginTabItem("Recipes")) */
					/* { */
                        /* /1* test_table.render(delta, food_list); *1/ */
                        /* if (ImGui::BeginTable( */
                            /* "food_table", */
                            /* 4 */
                        /* )) */
                        /* { */
                            /* for (const auto& item : food_list) */
                            /* { */
                                /* int i = 0; */
                                /* ImGui::TableNextRow(); */

                                /* ImGui::TableSetColumnIndex(i++); */
                                /* ImGui::TextUnformatted( */
                                    /* item.id.data(), */
                                    /* item.id.end() */
                                /* ); */

                                /* ImGui::TableSetColumnIndex(i++); */
                                /* ImGui::TextUnformatted( */
                                    /* item.title.data(), */
                                    /* item.title.end() */
                                /* ); */

                                /* ImGui::TableSetColumnIndex(i++); */
                                /* ImGui::TextUnformatted( */
                                    /* item.preparation_time.data(), */
                                    /* item.preparation_time.end() */
                                /* ); */
                            /* } */

                            /* ImGui::EndTable(); */
                        /* } */

					/* 	ImGui::EndTabItem(); */
					/* } */

			/* 		/1* if (ImGui::BeginTabItem("Recipes")) *1/ */
			/* 		/1* { *1/ */
			/* 		/1* 	recipes_table.render(delta); *1/ */

			/* 		/1* 	ImGui::EndTabItem(); *1/ */
			/* 		/1* } *1/ */

			/* 		if (ImGui::BeginTabItem("Foods")) */
			/* 		{ */
			/* 			/1* search_bar.render( *1/ */
			/* 			/1* 	delta, *1/ */
			/* 			/1* 	"#search_bar", // !!! *1/ */
			/* 			/1* 	food_title, // !!! *1/ */
			/* 			/1* 	[] (auto buffer_view) { *1/ */
			/* 			/1* 		foods_list.query(con, ) *1/ */
			/* 			/1* 	} *1/ */
			/* 			/1* ); *1/ */
			/* 			foods_table.render(delta); */

			/* 			ImGui::EndTabItem(); */
			/* 		} */

			/* 		if (ImGui::BeginTabItem("Cooking action")) */
			/* 		{ */
			/* 			/1* cooking_action_table.render(delta); *1/ */
			/* 			idk_table.render(delta, _foods); */

			/* 			ImGui::EndTabItem(); */
			/* 		} */

			/* 		if (ImGui::BeginTabItem("Debug")) */
			/* 		{ */
			/* 			if (ImGui::Button("Execute migrations")) */
			/* 			{ */
			/* 				con.close(); */
			/* 				db::migration::migrate( */
			/* 					std::filesystem::path {DATABASE_DIR} / DATABASE_NAME, */
			/* 					MIGRATIONS_DIR */
			/* 				); */

			/* 				con = {(std::filesystem::path {DATABASE_DIR} / DATABASE_NAME).string()}; */
			/* 			} */

			/* 			ImGui::EndTabItem(); */
			/* 		} */

					ImGui::EndTabBar();
				}

				ImGui::End();
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
        fmt::print(
            fmt::fg(fmt::color::dark_red),
            "{}\n",
            (utils::JsonSerializer {}).serialize(ctx)
        );
    }

	vk::check_silently(
		vkDeviceWaitIdle(device_obj.handle),
		"vkDeviceWaitIdle"
	);

	return 0;
}
