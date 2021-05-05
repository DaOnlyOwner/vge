#include "VkRenderer.h"
#include "GLFW/glfw3.h"

#include <vector>

#define VOLK_IMPLEMENTATION
#include "volk.h"

void volk_init()
{
	auto result = volkInitialize();
	if (!result) printf("ERROR: Cannot initialize volk");
}


#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
	if (severity & static_cast<unsigned>(VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
	{
		printf("Error: %s\n", callback_data->pMessage);
	}

	if (severity & static_cast<unsigned>(VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
	{
		printf("Info: %s\n", callback_data->pMessage);
	}

	if (severity & static_cast<unsigned>(VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
	{
		printf("Verbose: %s\n", callback_data->pMessage);
	}

	if (severity & static_cast<unsigned>(VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
	{
		printf("Warning: %s\n", callback_data->pMessage);
	}

	return VK_FALSE;

}

// FROM: https://vulkan-tutorial.com/code/02_validation_layers.cpp
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
#endif

void VkRenderer::create_instance()
{
    VkApplicationInfo appInfo{
        .sType{VK_STRUCTURE_TYPE_APPLICATION_INFO},
        .pNext = NULL,
        .pApplicationName{"vge"},
        .applicationVersion{VK_APP_VERSION},
        .pEngineName{"vge"},
        .engineVersion{VK_APP_VERSION},
        .apiVersion{VK_API_VERSION},
    };

	uint32_t ext_count;
	const char** glfw_ext = glfwGetRequiredInstanceExtensions(&ext_count);
	std::vector<const char*> all_ext(glfw_ext,glfw_ext + ext_count);
#ifndef NDEBUG
	all_ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	const char* enabledLayerNames[] = { VALIDATION_LAYER };
	uint32_t layerCount = 1;
#else
	const char* enabledLayerNames[] = {};
	int layerCount = 0;
#endif // !NDEBUG


	VkInstanceCreateInfo cInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = layerCount,
		.ppEnabledLayerNames = enabledLayerNames,
		.enabledExtensionCount = (uint32_t)all_ext.size(),
		.ppEnabledExtensionNames = all_ext.data()
	};

	vkCreateInstance(&cInfo, nullptr, &instance);
	volkLoadInstance(instance);


	
#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT dci{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = vulkan_debug
	};
	CreateDebugUtilsMessengerEXT(instance, &dci, nullptr, &debugMessenger);
#endif // !NDEBUG

}
