#include "utils.h"

VkSemaphoreCreateInfo make_vulkan_semaphore_create_info() {
	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pNext = nullptr;

	return createInfo;
}

VkFramebufferCreateInfo make_vulkan_framebuffer_create_info(
		VkRenderPass renderPass,
		VkImageView *attachments, uint32_t attachmentCount,
		uint32_t width, uint32_t height, uint32_t layers) {
	VkFramebufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pNext = nullptr;
	createInfo.attachmentCount = attachmentCount;
	createInfo.renderPass = renderPass;
	createInfo.pAttachments = attachments;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = layers;

	return createInfo;
}
