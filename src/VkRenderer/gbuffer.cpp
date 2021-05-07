
// FOLLOWS: https://github.com/SaschaWillems/Vulkan/blob/master/examples/deferred/deferred.cpp

#include "VkRenderer.h"
#include "volk.h"
#include <fstream>
#include <sstream>
#include "../helper.h"

VkShaderModule create_shader_module(VkDevice device, const char* filename)
{
	std::ifstream f(filename);
	if (!f) { printf("ERROR: Couldn't open shader file %s", filename); }
	std::stringstream srcbuf;
	srcbuf << f.rdbuf();
	f.close();
	std::string src = srcbuf.str();
	VkShaderModuleCreateInfo smci
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = src.size(),
		.pCode = reinterpret_cast<const uint32_t*>(src.data())
	};

	VkShaderModule mod;
	VkResult res = vkCreateShaderModule(device, &smci, nullptr, &mod);
	if (res != VK_SUCCESS) printf("ERROR: Creating the shadermodule from file %s failed", filename);
	return mod;
}


VkAttachmentDescription init_descr(VkImageLayout finalLayout, VkFormat format)
{
	VkAttachmentDescription descr
	{
		.format = format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = finalLayout,
	};

	return descr;
}


FrameBufferAttachment::FrameBufferAttachment(VkFormat format, VkImageUsageFlagBits usage, VkDevice device, const VkRenderer& renderer, int width, int height)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;
	this->format = format;
	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkImageCreateInfo ici
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = VkExtent3D{.width = (uint32_t)width,.height = (uint32_t)height,.depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = (VkImageUsageFlags)(usage | VK_IMAGE_USAGE_SAMPLED_BIT)
	};

	auto res = vkCreateImage(device, &ici, nullptr, &image);
	if (res != VK_SUCCESS) printf("ERROR: Couldnt create image");

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(device, image, &memReq);
	
	VkBool32 memTypeFound;
	VkMemoryAllocateInfo memAlloc
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReq.size,
		.memoryTypeIndex = renderer.getMemoryType(memReq.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,&memTypeFound)
	};
	res = vkAllocateMemory(device, &memAlloc, nullptr, &memory);
	auto res2 = vkBindImageMemory(device, image, memory, 0);
	if (res != VK_SUCCESS || res2 != VK_SUCCESS) printf("Error: Couldn't allocate and bind memory");
	VkImageViewCreateInfo ivci
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.format = format,
		.subresourceRange = {.aspectMask = aspectMask,.baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
	};

	res = vkCreateImageView(device, &ivci, nullptr, &view);

}

void VkRenderer::create_gbuffer(int width, int height)
{
	framebuffer.width = width;
	framebuffer.height = height;
	framebuffer.pos = FrameBufferAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,device,*this, width,height);
	framebuffer.normal = FrameBufferAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device, *this, width, height);
	framebuffer.diffuse = FrameBufferAttachment(VK_FORMAT_R8G8B8A8_SINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device, *this, width, height);
	framebuffer.depth = FrameBufferAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device, *this, width, height);
	auto diffuseAttDescr = init_descr(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, framebuffer.diffuse.format);
	auto normalAttDescr = init_descr(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, framebuffer.normal.format);
	auto posAttDescr = init_descr(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, framebuffer.pos.format);
	auto depthAttDescr = init_descr(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, framebuffer.depth.format);

	VkAttachmentReference colorRefs[] = { {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{1,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{2,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} };

	VkAttachmentReference depthRef = { .attachment = 3, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescr{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = ARRAY_SIZE(colorRefs),
		.pColorAttachments = colorRefs,
		.pDepthStencilAttachment = &depthRef,
	};

	VkSubpassDependency dep1
	{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};

	VkSubpassDependency dep2
	{
		.srcSubpass = 0,
		.dstSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};

	VkSubpassDependency dep[] = { dep1,dep2 };
	VkAttachmentDescription descr[] = { diffuseAttDescr,normalAttDescr,posAttDescr,depthAttDescr };
	VkRenderPassCreateInfo rpci
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = ARRAY_SIZE(descr),
		.pAttachments = descr,
		.subpassCount = 1,
		.pSubpasses = &subpassDescr,
		.dependencyCount = ARRAY_SIZE(dep),
		.pDependencies = dep
	};

	VkResult result = vkCreateRenderPass(device, &rpci, nullptr, &framebuffer.renderPass);
	if (result != VK_SUCCESS) printf("ERROR: Couldn't create renderpass");

	VkImageView attchs[] = { framebuffer.diffuse.view,framebuffer.normal.view,framebuffer.pos.view,framebuffer.depth.view };

	VkFramebufferCreateInfo fci
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = framebuffer.renderPass,
		.attachmentCount = ARRAY_SIZE(attchs),
		.pAttachments = attchs,
		.width = (uint32_t)framebuffer.width,
		.height = (uint32_t)framebuffer.height,
		.layers = 1
	};

	VkResult res = vkCreateFramebuffer(device, &fci, nullptr, &framebuffer.buffer);
	if (res != VK_SUCCESS) printf("ERROR: Couldn't create framebuffer");

	VkSamplerCreateInfo sci
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = 0.0f,
		.maxAnisotropy = 1.0f,
		.minLod = 0.0f,
		.maxLod = 1.0f,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	};

	res = vkCreateSampler(device, &sci, nullptr, &framebuffer.sampler);
	if (res != VK_SUCCESS) printf("ERROR: Couldn't create sampler");

}

// FROM: https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanDevice.cpp
uint32_t VkRenderer::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		throw std::runtime_error("Could not find a matching memory type");
	}
}