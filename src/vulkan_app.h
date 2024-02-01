#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <set>
#include <string>
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
    void createImageViews();
    void createRenderPass();
    void createGraphicPipeline();
    void createFrameBuffers();
    void createCommandPool();
    void createSyncObjects();
    void allocateCommandBuffers();
    void main_loop();
    void drawFrame();
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
    std::vector<char> readShaderFile(const std::string filePath) const;
    VkShaderModule createShaderModule(std::vector<char> shaderBytes) const;
    VkPipelineLayout createPipelineLayout() const;
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void cleanUpSwapChain();
    void recreateSwapChain();

    // Callback funtions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // Private class members
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    GLFWwindow* m_window;
    uint32_t m_maxInflightFrames = 2;
    uint32_t m_currentFrame = 0;

    // vkInstance and it's subordinates
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

    // VkDevice and its surboradinates
    VkDevice m_vkDevice;
    std::vector<const char*> m_deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkQueue m_vkGraphicQueue;  // Queue supports graphic operations
    VkQueue m_vkPresentQueue;  // Queue supports presenting images to a vulkan surface
    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;
    VkFormat m_swapChainImageFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_swapChainImageExtent;
    VkRenderPass m_vkRenderPass;
    VkPipelineLayout m_vkPipelineLayout;
    VkPipeline m_vkPipeline;
    VkCommandPool m_graphicCommandPool;
    std::vector<VkCommandBuffer> m_drawCommandBuffers;
    std::vector<VkSemaphore> m_acquireImageSemaphores;
    std::vector<VkSemaphore> m_drawSemaphores;
    std::vector<VkFence> m_inFlightFences;
};

