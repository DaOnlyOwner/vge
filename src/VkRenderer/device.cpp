#include "VkRenderer.h"
#include "volk.h"
#include <cstdio>

void VkRenderer::create_device() {
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
	VkPhysicalDevice physicalDevices[16]; // assumption that no one has more than 16 gpus
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		uint32_t surfaceFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], surface, &surfaceFormatCount, NULL);
		VkSurfaceFormatKHR surfaceFormats[16]; // i have yet to see a gpu with more than 4 surface formats
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], surface, &surfaceFormatCount, surfaceFormats);

		// instead of enforcing non-linear colorspace, we might just try this first, and if it's not found, use the first
		// format that's available from the surface. realistically, all modern devices support these formats.
		bool supportsSurface = false;
		for (uint32_t j = 0; j < surfaceFormatCount; j++) {
			if (surfaceFormats[j].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				supportsSurface = true;
				break;
			}
		}
		if (!supportsSurface)
			continue;

		uint32_t queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyPropertyCount, NULL);
		VkQueueFamilyProperties queueFamilyProperties[16];
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyPropertyCount, queueFamilyProperties);

		int32_t graphicsQueueFamilyIndex = -1;
		int32_t transferQueueFamilyIndex = -1;
		for (uint32_t j = 0; j < queueFamilyPropertyCount; j++) {
			if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				VkBool32 presentSupport;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, surface, &presentSupport);
				if (presentSupport == VK_TRUE)
					graphicsQueueFamilyIndex = j;
			} else if (queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
				transferQueueFamilyIndex = j;
		}

		if (graphicsQueueFamilyIndex == -1)
			continue;
		// if no dedicated transfer queue is found (like on intel gpus), use the graphics queue index for transfer operations
		if (transferQueueFamilyIndex == -1)
			transferQueueFamilyIndex = graphicsQueueFamilyIndex;

		// device found, get all the memory info about it
		physicalDevice = physicalDevices[i];
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		// these two structs are only important inside this function, so we don't cache them into the main struct
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		const char* extNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkPhysicalDeviceVulkan12Features pdf12{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = NULL,
			.descriptorIndexing = VK_TRUE,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE
		};


		VkPhysicalDeviceFeatures2 pdf2
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &pdf12,
			.features = physicalDeviceFeatures
		};
		
		const float prios[] = { 1.0f };
		VkDeviceQueueCreateInfo graphicsQ
		{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.pNext = NULL,
					.flags = 0,
					.queueFamilyIndex = (uint32_t)graphicsQueueFamilyIndex,
					.queueCount = 1,
					.pQueuePriorities = prios
		};

		VkDeviceQueueCreateInfo transferQ
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueFamilyIndex = (uint32_t)transferQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = prios
		};

		VkDeviceQueueCreateInfo queues[] = { graphicsQ, transferQ };

	

		VkDeviceCreateInfo deviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &pdf2,
			.flags = 0,
			.queueCreateInfoCount = (graphicsQueueFamilyIndex == transferQueueFamilyIndex) ? (uint32_t)1 : (uint32_t)2,
			.pQueueCreateInfos = queues,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = NULL,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = extNames,
			.pEnabledFeatures = NULL
		};

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
			printf("Error: device creation failed %s", physicalDeviceProperties.deviceName);
			continue;
		}
	}

	// if no device was created, terminate the program

	volkLoadDevice(device);
};
