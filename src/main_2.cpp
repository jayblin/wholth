#include <algorithm>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string_view>
#include <tuple>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

void report_error(std::string_view msg)
{
	throw std::runtime_error(msg.data());
}

namespace vk
{
	/* uint32_t g_vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 3, 0); */
	uint32_t g_vulkan_api_version;

	std::string to_readable_api_version(uint32_t version)
	{
		std::stringstream ss;

		ss
			<< (version >> 29U) << '.'
			<< ((version << 3) >> 25U) << '.'
			<< ((version << 3 << 7) >> 22U) << '.'
			<< ((version << 3 << 7 << 10) >> 20U)
		;

		return ss.str();
	}

	struct Instance
	{
		VkInstance handle;

		/**
		 * - Host access to instance must be externally synchronized 
		 * - Host access to all VkPhysicalDevice objects enumerated from instance must be externally synchronized
		 */
		Instance()
		{
			VkApplicationInfo app_info {
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = "Wholth",
				.applicationVersion = 1,
				.pEngineName = nullptr,
				.engineVersion = 0,
				.apiVersion = g_vulkan_api_version,
			};

			// @todo: Fill when not in dev.
			VkValidationFlagsEXT validation_flags {
				.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT,
				.pNext = nullptr,
				.disabledValidationCheckCount = 0,
				.pDisabledValidationChecks = nullptr,
			};

			// @todo: Fill when (not) in dev/prod.
			VkValidationFeaturesEXT validation_features {
				.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
				.pNext = nullptr,
				.enabledValidationFeatureCount = 0,
				.pEnabledValidationFeatures = nullptr,
				.disabledValidationFeatureCount = 0,
				.pDisabledValidationFeatures = nullptr,
			};

			// Maybe usefull???
			/* VkDirectDriverLoadingListLUNARG */ 

			VkInstanceCreateInfo create_info {
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
				.pApplicationInfo = &app_info,
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			};

			if (
				vkCreateInstance(
					&create_info,
					nullptr, // @todo: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap11.html#memory-allocation
					&this->handle
				) != VK_SUCCESS
			)
			{
				report_error("Failed to create VK instance!");
			}
		}

		~Instance()
		{
			vkDestroyInstance(
				this->handle,
				nullptr // @todo: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap11.html#memory-allocation
			);
		}
	};

	struct QueueFamily
	{
		uint32_t index;
		/* VkQueueFamilyProperties* props; */
	};
	/* using queue_family_index_t = uint32_t; */
	/* struct QueueFamilyIndices */
	struct QueueFamilies
	{
		/* std::optional<uint32_t> graphics; */
		QueueFamily graphics;
		/* std::tuple<queue_family_index_t> graphics; */
	};

	struct PhysicalDevice
	{
		VkPhysicalDevice handle;
		std::vector<VkQueueFamilyProperties> queue_family_properties {};

		PhysicalDevice(VkPhysicalDevice physical_device): handle(physical_device)
		{
			uint32_t queue_family_property_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(this->handle, &queue_family_property_count, nullptr);

			queue_family_properties.resize(queue_family_property_count);
			vkGetPhysicalDeviceQueueFamilyProperties(this->handle, &queue_family_property_count, queue_family_properties.data());

			/* vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR */
		}
	};

	std::vector<PhysicalDevice> list_physical_devices(vk::Instance& instance)
	{
		uint32_t physical_device_count = 0;
		if (vkEnumeratePhysicalDevices(instance.handle, &physical_device_count, nullptr) != VK_SUCCESS)
		{
			report_error("Couldn't get physical device count!");
		}

		std::vector<VkPhysicalDevice> physical_devices (physical_device_count);
		if (vkEnumeratePhysicalDevices(instance.handle, &physical_device_count, physical_devices.data()) != VK_SUCCESS)
		{
			report_error("Couldn't populate physical device vector!");
		}

		/* uint32_t physical_device_group_count; */
		/* if (vkEnumeratePhysicalDeviceGroups(instance.handle, &physical_device_group_count, nullptr) != VK_SUCCESS) */
		/* { */
		/* 	report_error("Couldn't physical device group count!"); */
		/* } */

		/* std::vector<VkPhysicalDeviceGroupProperties> physical_device_groups(physical_device_group_count); */
		/* if (vkEnumeratePhysicalDeviceGroups(instance.handle, &physical_device_group_count, physical_device_groups.data()) != VK_SUCCESS) */
		/* { */
		/* 	report_error("Couldn't enumerate physical device groups!"); */
		/* } */

		/* std::cout << "Enumerating physical device groups: " << '\n'; */
		/* for (const auto& group : physical_device_groups) */
		/* { */
		/* 	std::cout << "\t" << "Device count: " << group.physicalDeviceCount << '\n'; */
		/* 	std::cout << "\t" << "Subset Allocation: " << (VK_TRUE == group.subsetAllocation ? "yes" : "no") << '\n'; */
		/* } */

		return std::vector<PhysicalDevice>{physical_devices.begin(), physical_devices.end()};
	}

	/* const PhysicalDevice& query_physical_device(const std::vector<PhysicalDevice>& physical_devices) */
	std::tuple<
		const PhysicalDevice&,
		QueueFamilies
	> query_physical_device_info(const std::vector<PhysicalDevice>& physical_devices)
	{
		/* vkEnumerateDeviceExtensionProperties() */
		/* vkGetPhysicalDeviceFeatures */

		/* VkDeviceDeviceMemoryReportCreateInfoEXT // looks cool and usefull */

		const PhysicalDevice* luminary = nullptr;
		QueueFamilies queue_families {};
		int32_t max_score = 0;
		size_t i = 0;

		for (const auto& physical_device : physical_devices)
		{
			int32_t score = 1;
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(physical_device.handle, &props);

			if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == props.deviceType)
			{
				score += 10;
			}

			uint32_t j = 0;
			for (; j < physical_device.queue_family_properties.size(); j++)
			{
				const auto& queue_family_prop = physical_device.queue_family_properties[j];

				if (queue_family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					score += 10;
				}
			}

			if (score > max_score)
			{
				max_score = score;
				luminary = &physical_devices[i];
				queue_families = {
					.graphics = {
						.index = j
					}
				};
			}

			i++;
		}

		if (nullptr == luminary)
		{
			report_error("Couldn't find acceptable physical device!");
		}

		return {*luminary, queue_families};
	}

	struct Device
	{
		VkDevice handle;

		Device(const PhysicalDevice& physical_device)
		{
			const auto props_count = physical_device.queue_family_properties.size();
			std::vector<VkDeviceQueueCreateInfo> queue_infos (props_count);
			std::vector<std::vector<float>> priorities (props_count);
			for (uint32_t queue_family_idx = 0; queue_family_idx < props_count; queue_family_idx++)
			{
				priorities[queue_family_idx].resize(
					physical_device.queue_family_properties[queue_family_idx].queueCount,
					0.5
				);

				queue_infos.emplace_back(VkDeviceQueueCreateInfo {
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.queueFamilyIndex = queue_family_idx,
					.queueCount = physical_device.queue_family_properties[queue_family_idx].queueCount,
					.pQueuePriorities = priorities[queue_family_idx].data(),
				});
			}

			VkDeviceCreateInfo info {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = nullptr, // Can connect logigal device to nmultiple physical devices with this. As well as other things.
				/* .flags = , */
				.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
				.pQueueCreateInfos = queue_infos.data(),
				/* .enabledLayerCount = , // deprecated */
				/* .ppEnabledLayerNames = , // deprecated */
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
				.pEnabledFeatures = nullptr,
			};

			if (vkCreateDevice(physical_device.handle, &info, nullptr, &this->handle) != VK_SUCCESS)
			{
				report_error("Couldn't create logical device!");
			}
		}

		~Device()
		{
			vkDestroyDevice(handle, nullptr);
		}
	};

	struct CommandPool
	{
		VkCommandPool handle;
		Device* device {nullptr};
		uint32_t queue_family_idx;

		CommandPool()
		{
		}

		CommandPool(const CommandPool&) = delete;

		CommandPool& operator=(const CommandPool& other) = delete;

		CommandPool(CommandPool&& other)
		{
			*this = std::move(other);
		}

		CommandPool& operator=(CommandPool&& other)
		{
			if (this != &other)
			{
				this->queue_family_idx = other.queue_family_idx;
				this->handle = other.handle;
				this->device = other.device;

				other.device = nullptr;
			}

			return *this;
		}

		CommandPool(Device& device, uint32_t queue_family_idx):
			device(&device), queue_family_idx(queue_family_idx)
		{
			VkCommandPoolCreateInfo pool_create_info {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = queue_family_idx,
			};

			if (vkCreateCommandPool(this->device->handle, &pool_create_info, nullptr, &this->handle) != VK_SUCCESS)
			{
				report_error("Couldn't create command pool!");
			}
			// Pools can be "trimmed" https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap6.html#vkTrimCommandPool
		}

		~CommandPool()
		{
			if (this->device != nullptr)
			{
				vkDestroyCommandPool(this->device->handle, this->handle, nullptr);
			}
		}
	};
}

int main()
{
	try
	{
		// "...applications should determine the version of Vulkan available before calling vkCreateInstance"
		if (vkEnumerateInstanceVersion(&vk::g_vulkan_api_version) != VK_SUCCESS)
		{
			report_error("Couldn't enumerate Vulkan API version!");
		}

		std::cout << "Enumerated Vulkan API version: "
			<< vk::to_readable_api_version(vk::g_vulkan_api_version)
			<< '\n';

		vk::Instance instance {};

		auto physical_devices = vk::list_physical_devices(instance);
		auto main_device_info = vk::query_physical_device_info(physical_devices);
		const vk::PhysicalDevice& main_physical_device = std::get<0>(main_device_info);
		const vk::QueueFamilies queue_families = std::get<1>(main_device_info);

		vk::Device device {main_physical_device};

		vk::CommandPool graphics_command_pool {device, queue_families.graphics.index};
		std::vector<VkQueue> graphics_queue (main_physical_device.queue_family_properties[queue_families.graphics.index].queueCount);

		for (uint32_t queue_idx = 0; queue_idx < graphics_queue.size(); queue_idx++)
		{
			vkGetDeviceQueue(device.handle, queue_families.graphics.index, queue_idx, &graphics_queue[queue_idx]);
		}

		const size_t command_buffer_count = 1;
		const VkCommandBufferAllocateInfo allocate_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = graphics_command_pool.handle,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = command_buffer_count,
		};
		std::vector<VkCommandBuffer> command_buffers (command_buffer_count);

		if (vkAllocateCommandBuffers(device.handle, &allocate_info, command_buffers.data()) != VK_SUCCESS)
		{
			report_error("Couldn't allocate graphics command buffers!");
		}
		// vkResetCommandBuffer
		// vkFreeCommandBuffers

		for (auto command_buffer : command_buffers)
		{
			// Use when the command buffer is a secondary command buffer.
			/* const VkCommandBufferInheritanceInfo inhertiance_info { */
			/* 	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, */
			/* 	.pNext = nullptr, */
			/* 	.renderPass; */
			/* 	.subpass; */
			/* 	.framebuffer; */
			/* 	.occlusionQueryEnable; */
			/* 	.queryFlags; */
			/* 	.pipelineStatistics; */
			/* }; */
			const VkCommandBufferBeginInfo begin_info {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = 0,
				/* .pInheritanceInfo = &inhertiance_info, */
				.pInheritanceInfo = nullptr,
			};

			if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
			{
				report_error("Couldn't record commad buffer");
			}
		}

		const VkSubmitInfo submit_info {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = ,
			.commandBufferCount;
			.pCommandBuffers;
			.signalSemaphoreCount;
			.pSignalSemaphores;
		};
		// or vkQueueSubmit2
		if (vkQueueSubmit(graphics_queue, 1, &submit_info, fence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}
	}
	catch (const std::runtime_error& excp)
	{
		std::cout << "Runtime exception! " << excp.what() << '\n';
	}
}
