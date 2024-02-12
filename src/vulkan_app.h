#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    // Main vulkan application processing function
    void initWindow();
    void initVulkan();
    void createInstance();
    void createDebugMessenger();
    void createWindowSurface();
    void pickPhysicalDevice();
    void createLogicalDevices();
    void createSwapChain();
    void createSwapChainImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicPipeline();
    void createSwapChainFrameBuffers();
    void createGraphicCommandPool();
    void createTextureImage();
    void createTextureImageView();
    void createTextureImageSampler();
    void createDepthResources();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void allocateDescriptorSets();
    void createSyncObjects();
    void allocateCommandBuffers();
    void mainLoop();
    void drawFrame();
    void cleanUp();
    
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
    void recordDrawCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void cleanUpSwapChain();
    void recreateSwapChain();
    VkCommandBuffer beginSingleTimeCommandBuffer() const;
    void endSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const;
    uint32_t findMemoryTypeIndex(uint32_t requiredMemoryTypeBit, VkMemoryPropertyFlags requirdMemoryPropertyFlags) const;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredProperties, 
        VkBuffer& buffer, VkDeviceMemory& memory);
    void createImage(int width, int height, VkFormat format, VkImageUsageFlags usage, 
        VkMemoryPropertyFlags requiredMemoryProperty, VkImage& image, VkDeviceMemory& imageMemory);
    void copyBuffer2Buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBuffer2Image(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const;
    void updateUniformBuffers(uint32_t frameIndex) const;
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image) const;
    void createImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect) const;
    VkFormat findSupportedFormat(std::vector<VkFormat> formatCandidates, 
        VkImageTiling tiling, 
        VkFormatFeatureFlags desiredFeatures) const;

    // Callback funtions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // Private class members
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    GLFWwindow* m_window;
    uint32_t m_maxInflightFrames = 2;
    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;

    std::vector<float> m_vertices = {
        -0.5f, 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f,  // top left
        0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f,  // top right
        0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 0.f,  // bottom right
        -0.5f, -0.5f, 1.f, 1.f, 1.f, 0.f, 0.f  // bottom left
    };
    std::vector<uint16_t> m_indices = {
        0, 1, 2, 2, 3, 0
    };

    struct UBOMatrices
    {
        alignas(16) glm::mat4 model;  // alignment:16B, size:64B
        alignas(16) glm::mat4 view;  // alignment:16B, size:64B
        alignas(16) glm::mat4 projection;  // alignment:16B, size:64B
    };

    // Buffers and memories
    VkBuffer m_vertexBuffer;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkDeviceMemory m_indexBufferMemory;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBufferMemories;
    std::vector<void*> m_uniformBuffersMapped;

    // Images
    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkDeviceMemory m_textureImageMemory;
    VkSampler m_textureSampler;
    
    VkImage m_depthImage;
    VkImageView m_depthImageView;
    VkDeviceMemory m_depthImageMemory;

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
    VkPhysicalDeviceFeatures m_vkPhysicalDeviceFeature;
    VkPhysicalDeviceProperties m_vkPhysicalDeviceProperties;

    // VkDevice and its surboradinates
    VkDevice m_vkDevice;
    std::vector<const char*> m_deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkQueue m_graphicQueue;  // Queue supports graphic operations
    VkQueue m_vkPresentQueue;  // Queue supports presenting images to a vulkan surface
    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;
    VkFormat m_swapChainImageFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_swapChainImageExtent;
    VkRenderPass m_vkRenderPass;
    VkDescriptorSetLayout m_vkDescriptorSetLayout;
    VkDescriptorPool m_vkDescriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets; 
    VkPipelineLayout m_vkPipelineLayout;
    VkPipeline m_vkPipeline;
    VkCommandPool m_graphicCommandPool;
    
    std::vector<VkCommandBuffer> m_drawCommandBuffers;
    std::vector<VkSemaphore> m_acquireImageSemaphores;
    std::vector<VkSemaphore> m_drawSemaphores;
    std::vector<VkFence> m_inFlightFences;
};

