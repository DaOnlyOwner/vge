#include "app.h"
#include <cstdio>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../memory.h"

App::App()
{
	glfwInit();
	volk_init();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For now
	window = glfwCreateWindow(WIDTH, HEIGHT, "vge", nullptr, nullptr);
	renderer.create_instance();
	
	init_temporary_allocator(200 * 1024 * 1024);

	VkResult res = glfwCreateWindowSurface(renderer.instance, window, nullptr, &renderer.surface);
	if (res != VK_SUCCESS)
	{
		printf("ERROR: Surface creation failed");
	}

	renderer.create_device();
	VkExtent2D extent{ WIDTH,HEIGHT };
	renderer.create_swapchain(false, extent); // Options can later be implemented.
	//renderer.create_pipelines();
}

App::~App()
{
	renderer.destroy();
	glfwDestroyWindow(window);
}

void App::run()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		//renderer.draw();
	}
}
