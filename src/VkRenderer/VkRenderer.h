#pragma once
#include "volk.h"


#define VK_API_VERSION VK_API_VERSION_1_2 // Or what do we need
#define VK_APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

struct VirtualFrame {
	VkFramebuffer framebuffer;
	VkSemaphore imageAvailableSemahpore;
	VkSemaphore renderingFinishedSemaphore;
};

struct VkRenderer {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkDevice device;

	VkQueue presentQueue;
	VkQueue graphicsQueue;
	VkQueue computeQueue;

	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	VkImage *swapchainImages;
	VkImageView *swapchainImageViews;
	uint32_t swapchainImageCount;
	bool vsync;

	VirtualFrame *virtualFrames;
	int64_t virtualFrameCount;

	void create_instance();
	void create_device();

	bool create_swapchain(bool vsync, VkExtent2D extent);
	int32_t get_next_swapchain_image(VkSemaphore imageAvailableSemaphore);
	void present(VkSemaphore renderingFinishedSemaphore, int32_t imageIndex);

	void create_pipelines();
	void draw();

	void destroy_swapchain();
	void destroy();
};

