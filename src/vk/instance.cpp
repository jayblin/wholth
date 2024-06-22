#include <algorithm>
#include <string_view>
#include <vector>

/* #define GLFW_INCLUDE_VULKAN */
#include "GLFW/glfw3.h"
#include "fmt/color.h"
#include "vk/vk.hpp"

#include "vk/instance.hpp"

static VkBool32 debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData
)
{
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        /* LOG("Vulkan Info: " << pMessage); */
        fmt::print("Vulkan Info: {}\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        /* LOG_YELLOW("Vulkan Warning: " << pMessage); */
        fmt::print(fmt::fg(fmt::color::yellow), "Vulkan Warning: {}\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        /* LOG_YELLOW("Vulkan Performance Warning: " << pMessage); */
        fmt::print(fmt::fg(fmt::color::dark_orange), "Vulkan Performance Warning: {}\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        /* LOG_RED("Vulkan Error: " << pMessage); */
        fmt::print(fmt::fg(fmt::color::crimson), "Vulkan Error: {}\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        /* LOG_CYAN("Vulkan Performance Warning: " << pMessage); */
        fmt::print(fmt::fg(fmt::color::cyan), "Vulkan Debug: {}\n", pMessage);
    }

    return VK_FALSE;
}

static std::vector<const char*> get_instance_extensions()
{
    uint32_t glfw_extension_cout;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_cout);
    /* fmt::print(fmt::fg(fmt::color::yellow), "{}\n", glfw_extension_cout); */
    std::vector<const char*> extensions (glfw_extensions, glfw_extensions + glfw_extension_cout);

    if (extensions.end() == std::find_if(
        extensions.begin(),
        extensions.end(),
        [](std::string_view name) { return name.compare(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0; })
    )
    {
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }

    if (extensions.end() == std::find_if(
        extensions.begin(),
        extensions.end(),
        [](std::string_view name) { return name.compare("VK_KHR_surface") == 0; })
    )
    {
        extensions.emplace_back("VK_KHR_surface");
    }

    /* if (extensions.end() == std::find_if( */
    /*     extensions.begin(), */
    /*     extensions.end(), */
    /*     [](std::string_view name) { return name.compare("VK_MVK_macos_surface") == 0; }) */
    /* ) */
    /* { */
    /*     /1* extensions.emplace_back("VK_MVK_macos_surface"); *1/ */
    /* } */

    if (extensions.end() == std::find_if(
        extensions.begin(),
        extensions.end(),
        [](std::string_view name) { return name.compare("VK_EXT_metal_surface") == 0; })
    )
    {
        extensions.emplace_back("VK_EXT_metal_surface");
    }

    fmt::print(fmt::fg(fmt::color::yellow), "GLFW extensions to load:\n");
    for(auto e : extensions)
    {
        fmt::print(fmt::fg(fmt::color::yellow), "- {}\n", e);
    }

    return extensions;
}

static std::vector<const char*> get_layers()
{
    // @todo: what's this for?
    uint32_t e_layers_count;
    vkEnumerateInstanceLayerProperties(&e_layers_count, nullptr);
    std::vector<VkLayerProperties> e_layers (e_layers_count);
    vkEnumerateInstanceLayerProperties(&e_layers_count, e_layers.data());

    return {
        "VK_LAYER_KHRONOS_validation"
    };
}

vk::Instance::Instance()
{
}

vk::Instance::~Instance()
{
    vkDestroySurfaceKHR(this->handle, surface, vk::get_allocators());
    vkDestroyInstance(this->handle, vk::get_allocators());
}

void vk::Instance::init()
{
    if (VK_NULL_HANDLE != this->handle) {
        return;
    }

    VkApplicationInfo app_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Wholth",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        .pfnCallback = debug_callback,
        .pUserData = nullptr,
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

    auto extensions = get_instance_extensions();

    auto layers = get_layers();

    VkInstanceCreateInfo instance_create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debug_callback_create_info,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    vk::check(
        vkCreateInstance(&instance_create_info, vk::get_allocators(), &this->handle),
        "Couldn't create instance!"
    );
}

vk::Instance& vk::Instance::operator=(vk::Instance&& other)
{
    if (&other != this) {
        this->handle = other.handle;
        this->surface = other.surface;

        other.handle = VK_NULL_HANDLE;
        other.surface = VK_NULL_HANDLE;
    }

    return *this;
}
