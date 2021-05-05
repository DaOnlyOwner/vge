#include "VkRenderer.h"

#include <cinttypes>
#include <cstdio>

#include "../memory.h"

bool enumerate_surface_formats(
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface,
		uint32_t *formatCount,
		VkSurfaceFormatKHR **surfaceFormats) {
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, formatCount, nullptr) != VK_SUCCESS
			|| !formatCount)
		return false;

	*surfaceFormats = allocate<VkSurfaceFormatKHR>(temporaryAllocator, *formatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, formatCount, *surfaceFormats) != VK_SUCCESS)
		return false;


	return true;
}


bool enumerate_present_modes(
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface,
		uint32_t *modeCount,
		VkPresentModeKHR **presentModes) {
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalDevice, surface, modeCount, nullptr) != VK_SUCCESS || !modeCount)
		return false;

	*presentModes = allocate<VkPresentModeKHR>(temporaryAllocator, *modeCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, modeCount, *presentModes) != VK_SUCCESS)
		return false;

	return true;
}


bool VkRenderer::create_swapchain(bool vsync_) {
	vsync = vsync_;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS) {
		printf("Couldn't query surface capabilities\n");
		return false;
	}

	uint32_t formatCount;
	VkSurfaceFormatKHR *surfaceFormats;
	if (!enumerate_surface_formats(physicalDevice, surface, &formatCount, &surfaceFormats)) {
		return false;
	}

	uint32_t modeCount;
	VkPresentModeKHR *presentModes;
	if (!enumerate_present_modes(physicalDevice, surface, &modeCount, &presentModes)) {
		printf("There are no present modes supported\n");
		return false;
	}

	uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0
			&& minImageCount > surfaceCapabilities.maxImageCount) {
		minImageCount = surfaceCapabilities.maxImageCount;
	}

	VkSurfaceFormatKHR surfaceFormat;
	if (formatCount == 1
			&& surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		// The device has no preferred surface format so select the one we want
		surfaceFormat = { VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	} else {
		// Select an engine supported surface format
		surfaceFormat = surfaceFormats[0];
		for (uint32_t i = 0; i < formatCount; ++i) {
			auto f = surfaceFormats[i];
			// Chose a non-linear srgb format if one exists
			if (f.format == VK_FORMAT_R8G8B8A8_SRGB || f.format == VK_FORMAT_B8G8R8A8_SRGB) {
				surfaceFormat = f;
				break;
			}
		}
	}

	swapchainExtent = surfaceCapabilities.currentExtent;

	if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
		auto minExtent = surfaceCapabilities.minImageExtent;
		auto maxExtent = surfaceCapabilities.maxImageExtent;
		swapchainExtent = { 1280, 720 };
		if (swapchainExtent.width < minExtent.width) swapchainExtent.width = minExtent.width;
		if (swapchainExtent.width > maxExtent.width) swapchainExtent.width = maxExtent.width;
		if (swapchainExtent.height < minExtent.height) swapchainExtent.height = minExtent.height;
		if (swapchainExtent.height > maxExtent.height) swapchainExtent.height = maxExtent.height;
	}

	VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkSurfaceTransformFlagBitsKHR transformFlags = surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		transformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!vsync) {
		for (uint32_t i = 0; i < modeCount; ++i) {
			auto p = presentModes[i];
			if (p == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = p;
				break;
			}
			if (presentMode != VK_PRESENT_MODE_MAILBOX_KHR
					&& p == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				presentMode = p;
			}
		}
	}
	printf("Chose present mode: %d\n", presentMode);

	auto oldSwapchain = swapchain;
	VkSwapchainCreateInfoKHR swapchainCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = imageUsageFlags,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = transformFlags,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = oldSwapchain,
	};

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
		printf("Unable to create swapchain\n");
		return false;
	}

	// Destroy old objects
	if (oldSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}
	for (int32_t i = 0; i < swapchainImageCount; ++i) {
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}

	// Get the images from the swapchain
	auto nImageCount = swapchainImageCount;
	if (vkGetSwapchainImagesKHR(device, swapchain, &nImageCount, nullptr) != VK_SUCCESS
			|| !nImageCount) {
		return false;
	}

	if (vkGetSwapchainImagesKHR(device, swapchain, &nImageCount, swapchainImages) != VK_SUCCESS) {
		return false;
	}

	swapchainImageCount = nImageCount;

	// Create image views
	for (int32_t i = 0; i < swapchainImageCount; ++i) {
		VkImageViewCreateInfo imageViewCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surfaceFormat.format,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		if (vkCreateImageView(
					device,
					&imageViewCreateInfo,
					nullptr,
					swapchainImageViews + i) != VK_SUCCESS) {
			printf("Unable to create image view\n");
			return false;
		}
	}

	return true;
}

int32_t VkRenderer::get_next_swapchain_image(VkSemaphore imageAvailableSemaphore) {
	uint32_t imageIndex;
	bool loop;
	do {
		loop = false;
		auto result = vkAcquireNextImageKHR(
				device, swapchain, 100,
				imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		switch (result) {
			case VK_SUCCESS:
			case VK_SUBOPTIMAL_KHR:
				break;
			case VK_ERROR_OUT_OF_DATE_KHR:
				vkDeviceWaitIdle(device);
				create_swapchain(vsync);
				break;
			case VK_TIMEOUT:
				loop = true;
				break;
			default:
				break;
		}
	} while(loop);

	return imageIndex;
}

void VkRenderer::present(VkSemaphore renderingFinishedSemaphore, int32_t imageIndex) {
	uint32_t index = imageIndex;

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderingFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &index,
		.pResults = nullptr,
	};
	auto result = vkQueuePresentKHR(presentQueue, &presentInfo);

	switch (result) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			vkDeviceWaitIdle(device);
			create_swapchain(vsync);
			break;
		default:
			break;
	}
}

