# pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <optional>
#include <set>
#include <map>

// Forward declaration
struct Vertex;
class Model;

class Resources
{
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

public:
    // functions to init vulkan resouces(in order)
    void initWindow();
    void mainLoop();
    void drawFrame();
    void createInstance();
    void createDebugMessenger();
    void createWindowSurface();
    void pickPhysicalDevice();
    void createLogicalDevices();
    void createSwapChain();
    void createSwapChainImageViews();
    void createDepthResources();
    void createGraphicCommandPool();
    void allocateDrawCommandBuffers();
    void createDescriptorSetLayout();
    void createRenderPass();
    void createGraphicPipeline();
    void createSwapChainFrameBuffers();
    void createSyncObjects();
    void loadModel();
    void createUniformBuffers();
    void createDescriptorPool();
    void allocateDescriptorSets();
    void cleanUp();

    // public interface
    bool isValidationLayerEnbaled() const{ return m_enableValidationLayer; }
    bool m_complete = false;

    // public helper functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredProperties, 
        VkBuffer& buffer, VkDeviceMemory& memory);
    void createImage(int width, int height, VkFormat format, VkImageUsageFlags usage, 
        VkMemoryPropertyFlags requiredMemoryProperty, VkImage& image, VkDeviceMemory& imageMemory);
    void copyBuffer2Buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBuffer2Image(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image);
    void createTexture(const char* filename, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkImageView& textureImageView);
    void createSampler(VkSampler& sampler);
    void createImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect);
    uint32_t findMemoryTypeIndex(uint32_t requiredMemoryTypeBit, VkMemoryPropertyFlags requirdMemoryPropertyFlags);
    VkCommandBuffer beginSingleTimeCommandBuffer();
    void endSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void createVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
    void createIndexBuffer(std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
    
private:
    // Callback funtions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // private helper functions
    bool checkInstanceValidationLayersSupported(std::vector<const char*> validationLayerNames) const;
    void populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) const;
    std::vector<const char*> getRequiredExtentions() const;
    bool checkInstanceExtensionsSupported(std::vector<const char*> extensionNames) const;
    uint64_t ratePhysicalDevice(const VkPhysicalDevice& physicalDevice) const;
    bool checkDeviceExtensionSupported(std::vector<const char*> extensionNames, const VkPhysicalDevice& physicalDevice) const;
    RequiredQueueFamilyIndices queryRequiredQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const;
    SwapChainSupportDetails querySwapChainSupportedDetails(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const;
    VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> avaliableSurfaceFormats) const;
    VkPresentModeKHR chooseSwapChainPresentMode(std::vector<VkPresentModeKHR> avaliablePresentModes) const;
    VkExtent2D chooseSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkFormat findSupportedFormat(std::vector<VkFormat> formatCandidates, 
        VkImageTiling tiling, 
        VkFormatFeatureFlags desiredFeatures) const;
    std::vector<char> readShaderFile(const std::string filePath) const;
    VkShaderModule createShaderModule(std::vector<char> shaderBytes) const;
    void updateUniformBuffers(uint32_t frameIndex) const;
    VkPipelineLayout createPipelineLayout() const;
    void recordDrawCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void cleanUpSwapChain();
    void recreateSwapChain();
private:
    // Model
    Model* m_model;

    // window relate class variables
    GLFWwindow* m_window;
    uint32_t m_windowWidth = 1920, 
    m_windowHeight = 1080;
    uint32_t m_maxInflightFrames = 2;
    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;

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
    
    // Semaphore and fence
    std::vector<VkSemaphore> m_acquireImageSemaphores;
    std::vector<VkSemaphore> m_drawSemaphores;
    std::vector<VkFence> m_inFlightFences;

    struct UBOMatrices
    {
        alignas(16) glm::mat4 model;  // alignment:16B, size:64B
        alignas(16) glm::mat4 view;  // alignment:16B, size:64B
        alignas(16) glm::mat4 projection;  // alignment:16B, size:64B
    };

    // Buffers and memories
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBufferMemories;
    std::vector<void*> m_uniformBuffersMapped;

    // Depth resouces
    VkImage m_depthStencilImage;
    VkImageView m_depthStencilImageView;
    VkDeviceMemory m_depthStencilImageMemory;
    VkFormat m_depthStencilImageFormat;

};