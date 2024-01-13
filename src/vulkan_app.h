#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <set>
#include <optional> 

class VulkanApp
{
public:
    void run();
private:
    // Internal class
    struct RequiredQueueFamilyIndices
    {
        std::optional<uint32_t> graphicFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicFamily.has_value() && presentFamily.has_value();
        }

        std::set<uint32_t> getUniqueFamilyInidces()
        {
            return std::set<uint32_t>({graphicFamily.value(), presentFamily.value()});
        }

        std::vector<uint32_t> getAllFamilyIndices()
        {
            return std::vector<uint32_t>({graphicFamily.value(), presentFamily.value()});
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;  // To get the created swap chain extent
        std::vector<VkSurfaceFormatKHR> surfaceFormats;  // To get the created swap chain surface format
        std::vector<VkPresentModeKHR> presentModes;  // To get the created swao chain present mode

        bool isComplete()
        {
            return !surfaceFormats.empty() && !presentModes.empty();
        }
    };

    // Main processing functions
    void init_window();
    void init_vulkan();
    void createInstance();
    void createDebugMessenger();
    void createWindowSurface();
    void pickPhysicalDevice();
    void createLogicalDevices();
    void createSwapChain();
    void main_loop();
    void clean_up();

    // Some other helper functions
    void populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) const;
    uint64_t ratePhysicalDevice(const VkPhysicalDevice& physicalDevice) const;
    bool checkInstanceExtensionsSupported(std::vector<const char*> extensionNames) const;  
    bool checkInstanceValidationLayersSupported(std::vector<const char*> validationLayerNames) const;
    bool checkDeviceExtensionSupported(std::vector<const char*> extensionNames, const VkPhysicalDevice& physicalDevice) const;
    RequiredQueueFamilyIndices queryRequiredQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const;
    SwapChainSupportDetails querySwapChainSupportedDetails(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const;
    VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> avaliableSurfaceFormats);
    VkPresentModeKHR chooseSwapChainPresentMode(std::vector<VkPresentModeKHR> avaliablePresentModes);
    VkExtent2D chooseSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
    std::vector<const char*> getRequiredExtentions() const;

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
    std::vector<const char*> m_instanceExtensionNames;
#ifndef NDEBUG
    std::vector<const char*> m_instanceValidationLayerNames = {"VK_LAYER_KHRONOS_validation"};
    const bool m_enableValidationLayer = true;  // Debug
#else
    const bool m_enbaleValidationLayer = false;  // Release
#endif
    VkSurfaceKHR m_vkSurface;
    VkPhysicalDevice m_vkPhysicalDevice;
    VkDevice m_vkDevice;
    std::vector<const char*> m_deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkQueue m_vkDeviceGraphicQueue;  // Queue supports graphic operations
    VkQueue m_vkDevicePresentQueue;  // Queue supports presenting images to a vulkan surface
    VkSwapchainKHR m_vkSwapChain;
};

