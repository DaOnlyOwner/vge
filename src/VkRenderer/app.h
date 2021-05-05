#pragma once
#include "GLFW/glfw3.h"
#include "VkRenderer.h"

#define WIDTH 640
#define HEIGHT 320

struct App
{
	App();
	~App();
	void run();
	GLFWwindow* window;
	VkRenderer renderer;
};