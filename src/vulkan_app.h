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
    void createInstance();
    void createDebugMessenger();
    void createPhysicalDevices();
    void main_loop();
    void clean_up();

    // Some other helper functions
    void populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo);
    uint64_t ratePhysicalDevice(const VkPhysicalDevice& physicalDevice);
    bool checkExtensionsSurpported(std::vector<const char*> extensionNames);  
    bool checkValidationLayersSurpported(std::vector<const char*> validationLayerNames);
    std::vector<const char*> getRequiredExtentions();

    // Callback funtions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // Private class members
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    GLFWwindow* m_window;

    // vkInstance and it's subordinate
    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_vkMessenger;
    std::vector<const char*> m_enabledExtensionNames;
#ifndef NDEBUG
    std::vector<const char*> m_validationLayerNames = {"VK_LAYER_KHRONOS_validation"};
    const bool m_enableValidationLayer = true;  // Debug
#else
    const bool m_enbaleValidationLayer = false;  // Release
#endif
    VkPhysicalDevice m_vkPhysicalDevice;
};

