#pragma once
#include "volk.h"


#define VK_API_VERSION VK_API_VERSION_1_2 // Or what do we need
#define VK_APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

void volk_init();

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
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkDevice device = VK_NULL_HANDLE;

	// this queue should be able to do everything. graphics, present, compute
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue transferQueue = VK_NULL_HANDLE;

	VkCommandPool graphicsCmdPool = VK_NULL_HANDLE;
	VkCommandPool transferCmdPool = VK_NULL_HANDLE;
	VkCommandBuffer graphicsCmdBuffers[3];
	VkCommandBuffer transferCmdBuffer;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D swapchainExtent{ 0, 0 };
	VkImage *swapchainImages = nullptr;
	VkImageView *swapchainImageViews = nullptr;
	uint32_t swapchainImageCount = 0;
	bool vsync = false;

	VkSemaphore imageAvailableSemaphores[3];
	VkSemaphore renderingFinishedSemaphores[3];
	VkFramebuffer primaryFramebuffers[3];

	int32_t frameIdx = 0;

	Framebuffer framebuffer;

	void create_instance();
	void create_device();

	bool create_swapchain(bool vsync);
	int32_t get_next_swapchain_image(VkSemaphore imageAvailableSemaphore);
	void present_swapchain_image(VkSemaphore renderingFinishedSemaphore, int32_t imageIndex);

	void create_command_pools();

	void create_frame_resources();

	void create_gbuffer(int width,int height);
	uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const;

	void create_pipelines();
	void draw();

	void destroy_swapchain();
	void destroy();
};

