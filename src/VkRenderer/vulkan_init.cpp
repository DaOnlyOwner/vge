#include "VkRenderer.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"
#include "log.h"

void volk_init()
{
	auto result = volkInitialize();
	if (!result) log::fail("Cannot initialize volk");
}

void create_instance()
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

	const char* enabledLayerNames[] = { VALIDATION_LAYER };
	const char* enabledExtensionNames[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME };
	VkInstanceCreateInfo cInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = enabledLayerNames,
		.enabledExtensionCount = 3,
		.ppEnabledExtensionNames = enabledExtensionNames
	};
	
}
