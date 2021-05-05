#include "VkRenderer.h"

void VkRenderer::destroy_swapchain() {
	// wait for fences, then destroy things like images and swapchain

};

// destroys absolutely everything
void VkRenderer::destroy() {
	destroy_swapchain();

	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	vkDestroyInstance(instance, NULL);
};