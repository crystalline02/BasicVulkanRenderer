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
class Model;
class ParticleGroup;
struct Vertex;
struct Texture;
class Particle;

class Resources
{
private:
    // band constructor
    Resources();

    // Internal class
    struct RequiredQueueFamilyIndices
    {
        std::optional<uint32_t> graphicFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> graphicComputeFamily;

        bool isComplete()
        {
            return graphicFamily.has_value() && presentFamily.has_value() & graphicComputeFamily.has_value();
        }

        std::set<uint32_t> getUniqueFamilyInidces()
        {
            return std::set<uint32_t>({graphicFamily.value(), presentFamily.value(), graphicComputeFamily.value()});
        }

        std::vector<uint32_t> getAllFamilyIndices()
        {
            return std::vector<uint32_t>({graphicFamily.value(), presentFamily.value(), graphicComputeFamily.value()});
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
    void createColorResources();
    void createDepthResources();
    void createCommandPool();
    void allocateCommandBuffers();
    void createDescriptorSetLayout();
    void createRenderPass();
    void createPipeline();
    void createSwapChainFrameBuffers();
    void createSyncObjects();
    void loadModel();
    void loadParticles();
    void createDrawUniformBuffers();
    void createDescriptorPool();
    void allocateDescriptorSets();
    void cleanUp();

    // public interface
    bool isValidationLayerEnbaled() const { return m_enableValidationLayer; }
    VkExtent2D windowSize() const { return {m_windowWidth, m_windowHeight}; }
    static Resources* get();
    bool m_complete = false;

    // public helper functions
    void createImage(int width, int height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevel, VkSampleCountFlagBits samples,
        VkMemoryPropertyFlags requiredMemoryProperty, VkImage& image, VkDeviceMemory& imageMemory) const;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredProperties, 
        VkBuffer& buffer, VkDeviceMemory& memory) const;
    void copyBuffer2Buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBuffer2Image(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const;
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image, uint32_t mipLevels) const;
    void createTexture(const char* filename, Texture& texture) const;
    void createSampler(VkSampler& sampler, uint32_t mipLevel) const;
    void createImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect) const;
    uint32_t findMemoryTypeIndex(uint32_t requiredMemoryTypeBit, VkMemoryPropertyFlags requirdMemoryPropertyFlags) const;
    VkCommandBuffer beginSingleTimeCommandBuffer() const;
    void endSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const;
    void createVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) const;
    void createIndexBuffer(std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) const;
    void createParticleUBOs(std::vector<VkBuffer>& particleUBOs, 
        std::vector<VkDeviceMemory>& paritcleUBOMemories,
        std::vector<void*>& particleUBOMapped);
    void createParticleSSBOs(std::vector<VkBuffer>& particleSSBOs, 
        std::vector<VkDeviceMemory>& particleSSBOMemories,
        const std::vector<Particle>& particles) const;
    void cleanUpTexture(const Texture& texture) const;
    void generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels) const;
    void allocateParticleDescriptorSets(std::vector<VkDescriptorSet>& particleDescriptorSets,
        const std::vector<VkBuffer>& particleUBOs, 
        const std::vector<VkBuffer>& particleSSBOs) const;
    void allocateDrawParticleCommanBuffers(std::vector<VkCommandBuffer> commandBuffers) const;
    void recordUpdateParticleCommandBuffer(VkCommandBuffer commandBuffer, uint32_t particleCount) const;
private:
    // Callback funtions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // private helper functions
    VkSampleCountFlagBits getMSAASampleCount() const;
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
    void recordDrawCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
    void cleanUpSwapChain();
    void recreateSwapChain();
    void createPipelineLayout(VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout descriptorSetLayout);
private:
    // class instance
    static Resources* instance;

    // Model
    Model* m_model;

    // Particle group
    ParticleGroup* m_particles;

    // window relate class variables
    GLFWwindow* m_window;
    uint32_t m_windowWidth = 1920, 
        m_windowHeight = 1080;
    uint32_t m_maxInflightFrames = 2;
    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;

    // vulkan instance
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

    // physcial device
    VkPhysicalDevice m_physicalDevice;
    VkPhysicalDeviceFeatures m_physicalDeviceFeature;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;

    // vulkan device
    VkDevice m_device;
    std::vector<const char*> m_deviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // vulkan queue families
    VkQueue m_graphicQueue;  // Queue supports graphic operations(and definitly supports transfer opeartions)
    VkQueue m_vkPresentQueue;  // Queue supports presenting images to a vulkan surface
    VkQueue m_graphicComputeQueue;  // Queue supports graphic operations and compution operations

    // swapchain resources
    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainImageExtent;
    VkPresentModeKHR m_presentMode;

    // renderpass
    VkRenderPass m_renderPass;

    // descriptor set
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_drawDescriptorSets; 

    // pipeline 
    VkPipelineLayout m_graphicPipelineLayout,
        m_computePipelineLayout;
    VkDescriptorSetLayout m_drawDescriptorSetLayout, 
        m_particleDescriptorSetLayout;
    VkPipeline m_graphicPipeline,
        m_computePipeline;

    // command buffers
    VkCommandPool m_graphicCommandPool, 
        m_computeCommandPool;
    std::vector<VkCommandBuffer> m_drawCommandBuffers,
        m_computeCommandBuffers;
    
    // synchronization primitives
    std::vector<VkSemaphore> m_acquireImageSemaphores,
        m_drawSemaphores,
        m_computeCompleteSemaphores;
    std::vector<VkFence> m_inFlightFences,
        m_computeInFlightFences;

    struct UBOProjectionMatrices
    {
        alignas(16) glm::mat4 model;  // alignment:16B, size:64B
        alignas(16) glm::mat4 view;  // alignment:16B, size:64B
        alignas(16) glm::mat4 projection;  // alignment:16B, size:64B
    };

    // Buffers and memories
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBufferMemories;
    std::vector<void*> m_uniformBuffersMapped;

    // Depth resources
    VkImage m_depthStencilImage;
    VkImageView m_depthStencilImageView;
    VkDeviceMemory m_depthStencilImageMemory;
    VkFormat m_depthStencilImageFormat;

    // Color resources
    VkImage m_colorMSAAImage;
    VkImageView m_colorMSAAImageView;
    VkDeviceMemory m_colorMSAAImageMemory;

    // MSAA sample count
    VkSampleCountFlagBits m_MSAASampleCount;
};