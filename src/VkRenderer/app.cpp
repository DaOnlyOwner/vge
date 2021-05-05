#include "app.h"

App::App()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For now
	window = glfwCreateWindow(WIDTH, HEIGHT, "vge", nullptr, nullptr);
	renderer.create_instance();
	renderer.create_device();
	renderer.create_swapchain();
	renderer.create_pipelines();
}

App::~App()
{
	renderer.destroy();
	renderer.destroy_swapchain();
	glfwDestroyWindow(window);
}

void App::run()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.draw();
	}
}
