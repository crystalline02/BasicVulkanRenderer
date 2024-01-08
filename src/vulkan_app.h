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
    void create_messager_callback();
    void create_instance();
    void main_loop();
    void clean_up();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static VkResult createDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pMessenger);
    static void destoryDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks* pAllocator);
    

    bool checkExtensionsSurpported(std::vector<const char*> extensionNames);  
    bool checkValidationLayersSurpported(std::vector<const char*> validationLayerNames);
    std::vector<const char*> getRequiredExtentions();
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    GLFWwindow* m_window;

    VkInstance m_vkInstance;
    std::vector<const char*> m_enabledExtensionNames;
#ifndef NDEBUG
    std::vector<const char*> m_validationLayerNames = {"VK_LAYER_KHRONOS_validation"};
    const bool m_enableValidationLayer = true;  // Debug
    VkDebugUtilsMessengerEXT m_debugMessenger;
#else
    const bool m_enbaleValidationLayer = false;  // Release
#endif
};