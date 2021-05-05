#pragma once
#include "volk.h"


#define VK_API_VERSION VK_API_VERSION_1_2 // Or what do we need
#define VK_APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

void volk_init();

struct VkRenderer {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkDevice device;
	VkSwapchainKHR swapchain;

	void create_instance();
	void create_device();
	void create_swapchain();
	void create_pipelines();
	void draw();

	void destroy_swapchain();
	void destroy();
};

