#pragma once
#include "VkRenderer.h"

#define WIDTH 640
#define HEIGHT 320

struct App
{
	App();
	~App();
	void run();
	class GLFWwindow* window;
	VkRenderer renderer;
};