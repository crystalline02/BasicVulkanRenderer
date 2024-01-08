#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanApp
{
public:
    void run();
private:
    void init_window();
    void init_vulkan();
    void create_instance();
    void main_loop();
    void clean_up(); 

    bool checkExtensionsSurpported(const char** enabledExtensionNames, uint32_t enabeldExtensionCount);  
    bool checkValidationLayersSurpported(const char** validationLayers, uint32_t validaitonLayersCount);
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    GLFWwindow* m_window;

    VkInstance m_vkInstance;
    std::vector<const char*> m_enabledExtensionNames;
#ifndef NDEBUG
    std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const bool m_enableValidationLayer = true;  // Debug
#else
    const bool m_enbaleValidationLayer = false;  // Release
#endif
};