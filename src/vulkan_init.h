#pragma once

#define VK_API_VERSION VK_API_VERSION_1_1 // Or what do we need
#define VK_APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

void create_instance();
