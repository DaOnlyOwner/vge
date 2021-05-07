#pragma once

#include "volk.h"

VkSemaphoreCreateInfo make_vulkan_semaphore_create_info();
VkFramebufferCreateInfo make_vulkan_framebuffer_create_info(
		VkRenderPass renderPass,
		VkImageView *attachments, uint32_t attachmentCount,
		uint32_t width, uint32_t height, uint32_t layers);
