#pragma once
#include "volk.h"


#define VK_API_VERSION VK_API_VERSION_1_2 // Or what do we need
#define VK_APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

void volk_init();
struct VirtualFrame {
	VkFramebuffer framebuffer;
	VkSemaphore imageAvailableSemahpore;
	VkSemaphore renderingFinishedSemaphore;
};

struct FrameBufferAttachment
{
	FrameBufferAttachment() = default;
	FrameBufferAttachment(VkFormat format, VkImageUsageFlagBits usage, VkDevice device, const struct VkRenderer& renderer, int width, int height);
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkFormat format;
};

struct Framebuffer
{
	int width, height;
	VkFramebuffer buffer;
	FrameBufferAttachment pos, normal, diffuse, depth;
	VkRenderPass renderPass;
	VkSampler sampler;
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

	Framebuffer framebuffer;

	void create_instance();
	void create_device();

	bool create_swapchain(bool vsync, VkExtent2D extent);
	int32_t get_next_swapchain_image(VkSemaphore imageAvailableSemaphore);
	void present(VkSemaphore renderingFinishedSemaphore, int32_t imageIndex);

	void create_gbuffer(int width,int height);
	uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const;


	void create_pipelines();
	void draw();

	void destroy_swapchain();
	void destroy();
};

