#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <limits>
#include <algorithm>
#include <fstream>

#include "vulkan_fn.h"
#include "./model/model.h"
#include "./model/texture.h"
#include "./model/mesh.h"
#include "./particle/particle.h"

Resources::Resources()
{

}

void Resources::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan Renderer", VK_NULL_HANDLE, VK_NULL_HANDLE);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetWindowUserPointer(m_window, this);
}

void Resources::drawFrame()
{
    // Wait for m_inFlightFence to be signaled.So that we won't record the command buffer in the next frame 
    // while the GPU is still using the command buffer in current frame.
    vkWaitForFences(m_vkDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire an image for swapchain
    uint32_t imageIndex;
    VkResult acquireImageResult = vkAcquireNextImageKHR(m_vkDevice, m_vkSwapChain, UINT64_MAX, m_acquireImageSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if(acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR ||
        acquireImageResult == VK_SUBOPTIMAL_KHR)
    {
        /* 
        It's important to know that if vkAcquireNextImageKHR doesn't return VK_SUCCESS,
        the semaphore specified won't be signaled
        */
        recreateSwapChain();
        /*
        Since the image acquired from the swapchain is not compatible with the window surface, do not present 
        the image, just return drawFrame function.

        drawFrame function returns with m_inFlightFences[m_currentFrame] signaled, avoiding deadlock from 
        vkWaitForFences.
        */
        return;  
    }
    else if(acquireImageResult != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to acquire an image for the swap chain.");
    
    // Reset fence
    vkResetFences(m_vkDevice, 1, &m_inFlightFences[m_currentFrame]);  // Reset fence to unsignaled

    // Reset commandbuffer
    vkResetCommandBuffer(m_drawCommandBuffers[m_currentFrame], 0);

    // Record commandbuffer
    recordDrawCommandBuffer(m_drawCommandBuffers[m_currentFrame], imageIndex);

    // Update uniform buffers
    updateUniformBuffers(m_currentFrame);

    // Submit commandbuffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_acquireImageSemaphores[m_currentFrame];
    VkPipelineStageFlags stageMask[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = stageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_drawCommandBuffers[m_currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_drawSemaphores[m_currentFrame];

    if(vkQueueSubmit(m_graphicQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to submit drawCommandBuffer to graphic queue.");

    // Present image to screen
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_drawSemaphores[m_currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &m_vkSwapChain,
        .pImageIndices = &imageIndex,
        .pResults = VK_NULL_HANDLE
        };
    VkResult queuePresentResult = vkQueuePresentKHR(m_vkPresentQueue, &presentInfo);
    if(queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || 
        queuePresentResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        recreateSwapChain();
    }
    else if(queuePresentResult != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to present an image to screen.");

    m_currentFrame = (m_currentFrame + 1) % m_maxInflightFrames;
}

void Resources::createInstance()
{
    // VkApplicationInfo
    VkApplicationInfo appInfo = {};
    appInfo.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;  // createInfo use applicationInfo as a part of its struct members

    // Add&check validation layers
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    if(m_enableValidationLayer)
    {
        if(!checkInstanceValidationLayersSupported(m_instanceValidationLayerNames))
            throw std::runtime_error("VK ERROR: Unsurpported vulkan instance validation layer.");
        instanceCreateInfo.enabledLayerCount = (uint32_t)m_instanceValidationLayerNames.size();
        instanceCreateInfo.ppEnabledLayerNames = m_instanceValidationLayerNames.data();
        // Populate messengerCreateInfo
        populateMessengerCreateInfo(messengerCreateInfo);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerCreateInfo;
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = VK_NULL_HANDLE;
    }

    // Add&check extensions
    m_instanceExtensionNames = getRequiredExtentions();
    if(!checkInstanceExtensionsSupported(m_instanceExtensionNames))
        throw std::runtime_error("VK ERROR: Unsurpported vulkan instance extensions.");
    instanceCreateInfo.enabledExtensionCount = (uint32_t)m_instanceExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensionNames.data();

    // finally create intance
    if(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkInstance.");
    
    if(load_vkInstanceFunctions(m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to load vulkan instance functions.");
}

void Resources::createDebugMessenger()
{
    if(!m_enableValidationLayer) return;
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    populateMessengerCreateInfo(messengerCreateInfo);
    if(vkCreateDebugUtilsMessengerEXT(m_vkInstance, &messengerCreateInfo, VK_NULL_HANDLE, &m_vkMessenger) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create vkDebugUtilsMessengerEXT.");
}

void Resources::createWindowSurface()
{
    if(glfwCreateWindowSurface(m_vkInstance, m_window, VK_NULL_HANDLE, &m_vkSurface) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkSurfaceKHR");
}

void Resources::pickPhysicalDevice()
{
    // Get all physical devices
    uint32_t vk_physicalDeviceCount;
    vkEnumeratePhysicalDevices(m_vkInstance, &vk_physicalDeviceCount, VK_NULL_HANDLE);
    if(vk_physicalDeviceCount == 0) 
        throw std::runtime_error("VK ERROR: Failed to find any GPUs with vulkan support.");
    std::vector<VkPhysicalDevice> vk_physicalDevices(vk_physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &vk_physicalDeviceCount, vk_physicalDevices.data());

    // Pick a physical device
    std::multimap<uint64_t, VkPhysicalDevice> score_devices;
    for(const VkPhysicalDevice& physicalDevice: vk_physicalDevices)
        score_devices.insert({ratePhysicalDevice(physicalDevice), physicalDevice});
    if(score_devices.rbegin()->first > 0) 
    {
        m_vkPhysicalDevice = score_devices.rbegin()->second;  // This line picks a VkPhysicalDevice
        
        vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &m_vkPhysicalDeviceFeature);
        vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties);

        // Log selected GPU
        std::cout << "VK INFO: " << m_vkPhysicalDeviceProperties.deviceName << " is selected as a vulkan physical device.Score: " 
            << score_devices.rbegin()->first <<  ".\n";
        
        // Select MASS sample count
        m_MSAASampleCount = getMSAASampleCount();
    }
    else throw std::runtime_error("VK ERROR: No suitable physical device for current application.");

    
}

void Resources::createLogicalDevices()
{
    RequiredQueueFamilyIndices queueFamilyIndices = queryRequiredQueueFamilies(m_vkPhysicalDevice, m_vkSurface);
    std::set<uint32_t> unqiueQueueFamilyIndices = queueFamilyIndices.getUniqueFamilyInidces();  // queue family indiecs that we need on our logical device
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const float queuePriorities = 1.f;
    for(uint32_t index: unqiueQueueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriorities;

        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.samplerAnisotropy = m_vkPhysicalDeviceFeature.samplerAnisotropy;
    physicalDeviceFeatures.sampleRateShading = m_vkPhysicalDeviceFeature.sampleRateShading;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)m_deviceExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensionNames.data();
    if(m_enableValidationLayer)
    {
        // Keep device validation layers the same as instance validation layers
        deviceCreateInfo.enabledLayerCount = (uint32_t)m_instanceValidationLayerNames.size();
        deviceCreateInfo.ppEnabledLayerNames = m_instanceValidationLayerNames.data();
    }
    else
        deviceCreateInfo.enabledExtensionCount = 0;

    if(vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_vkDevice) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDevice.");
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndices.graphicFamily.value(), 0, &m_graphicQueue);
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndices.presentFamily.value(), 0, &m_vkPresentQueue);
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndices.graphicComputeFamily.value(), 0, &m_graphicComputeQueue);
}

void Resources::createSwapChain()
{
    SwapChainSupportDetails swapChainSurpportedDetails = querySwapChainSupportedDetails(m_vkPhysicalDevice, m_vkSurface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat(swapChainSurpportedDetails.surfaceFormats);
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainImageExtent = chooseSwapchainExtent(swapChainSurpportedDetails.surfaceCapabilities);
    m_presentMode = chooseSwapChainPresentMode(swapChainSurpportedDetails.presentModes);

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = m_vkSurface;
    swapChainCreateInfo.minImageCount = std::clamp(swapChainSurpportedDetails.surfaceCapabilities.minImageCount + 1, 
        swapChainSurpportedDetails.surfaceCapabilities.minImageCount,
        swapChainSurpportedDetails.surfaceCapabilities.maxImageCount);
    swapChainCreateInfo.imageFormat = m_swapChainImageFormat;  // VK_FORMAT_B8G8R8A8_SRGB
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace; // CVK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    swapChainCreateInfo.imageExtent = m_swapChainImageExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    RequiredQueueFamilyIndices requiredQueueFamilyIndices = queryRequiredQueueFamilies(m_vkPhysicalDevice, m_vkSurface);
    std::vector<uint32_t> queueFamilyIndices = requiredQueueFamilyIndices.getAllFamilyIndices();
    if(requiredQueueFamilyIndices.graphicFamily != requiredQueueFamilyIndices.presentFamily)
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
    }
    swapChainCreateInfo.preTransform = swapChainSurpportedDetails.surfaceCapabilities.currentTransform;  // VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = m_presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(m_vkDevice, &swapChainCreateInfo, VK_NULL_HANDLE, &m_vkSwapChain) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkSwapChainKHR.");
    
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &swapChainImageCount, VK_NULL_HANDLE);
    m_swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &swapChainImageCount, m_swapChainImages.data());
}

void Resources::createSwapChainImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for(uint32_t i = 0; i < m_swapChainImageViews.size(); ++i)
        createImageView(m_swapChainImageViews[i], m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Resources::createDepthResources()
{
    m_depthStencilImageFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    createImage(m_swapChainImageExtent.width, m_swapChainImageExtent.height, m_depthStencilImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, 
        m_MSAASampleCount, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthStencilImage, m_depthStencilImageMemory);
    createImageView(m_depthStencilImageView, m_depthStencilImage, m_depthStencilImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Resources::createGraphicCommandPool()
{
    RequiredQueueFamilyIndices requriedQueueFamilyIndices = queryRequiredQueueFamilies(m_vkPhysicalDevice, m_vkSurface);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // Reset only the relevant command buffer(No influence for other command buffers).
    commandPoolCreateInfo.queueFamilyIndex = requriedQueueFamilyIndices.graphicFamily.value();

    if(vkCreateCommandPool(m_vkDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &m_graphicCommandPool) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkCommandPool.");
}

void Resources::allocateDrawCommandBuffers()
{
    m_drawCommandBuffers.resize(m_maxInflightFrames);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_graphicCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = m_maxInflightFrames;

    if(vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, m_drawCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate VkCommandBuffer");
}

void Resources::createDescriptorSetLayout()
{
    // Descriptor set for drawing
    VkDescriptorSetLayoutBinding drawSetPBindings[2] = {
        // mvp matrices
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        },
        // texture
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        }
    };

    VkDescriptorSetLayoutCreateInfo drawDescriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = drawSetPBindings
    };
    if(vkCreateDescriptorSetLayout(m_vkDevice, &drawDescriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_drawDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDescriptorSetLayout.");

    // Descriptor set for updating particle status
    VkDescriptorSetLayoutBinding particlePBindings[3] = {
        // delta time
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        },
        // particle ssbo for read
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        },
        // particle ssbo for write
        {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        }
    };
    VkDescriptorSetLayoutCreateInfo particleDescriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings = particlePBindings
    };
    if(vkCreateDescriptorSetLayout(m_vkDevice, &particleDescriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_particleDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDescriptorSetLayout.");
}

void Resources::createRenderPass()
{
    VkAttachmentDescription colorAttachmentDescription = {
        .format = m_swapChainImageFormat,
        .samples = m_MSAASampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,  // What to do before the renderpass for color&depth attachment
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,  // What to do after the renderpass for color&depth attachment
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // What to do before the renderpass for stencil attachment
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,  // What to do after the renderpass for stencil attachment
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,  // Image layout before the renderpass
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image layout after the renderpass
    };
    VkAttachmentDescription depthStencilAttachmentDescription = {
        .format = m_depthStencilImageFormat,
        .samples = m_MSAASampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentDescription resolveAttachmentDescription = {
        .format = m_swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentDescription attachmentDescriptions[3] = {colorAttachmentDescription, depthStencilAttachmentDescription, resolveAttachmentDescription};

    VkAttachmentReference colorAttachmentReference = {
        .attachment = 0,  // index corresponding renderPassCreateInfo.pAttachments
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Image layout during the render pass
    };
    VkAttachmentReference depthStencilAttachmentReference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference resolveAttachmentReference = {
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpassDescription = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = VK_NULL_HANDLE,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = &resolveAttachmentReference,
        .pDepthStencilAttachment = &depthStencilAttachmentReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = VK_NULL_HANDLE
    };

    VkSubpassDependency subpassDependency;
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 3,
        .pAttachments = attachmentDescriptions,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    if(vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, VK_NULL_HANDLE, &m_vkRenderPass) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkRenderPass");
}

void Resources::createGraphicPipeline()
{
    // Create info for Shader stage
    std::vector<char> vertexShaderBytes = readShaderFile("./shaders/triangle_vert.spv");
    std::vector<char> fragmentShaderBytes = readShaderFile("./shaders/triangle_frag.spv");
    std::vector<char> computeShaderBytes = readShaderFile("./shaders/particle_comp.spv");
    
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderBytes);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderBytes);
    VkShaderModule computeShaderModule = createShaderModule(computeShaderBytes);
    
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShaderModule,
        .pName = "main",  // Entry point
        .pSpecializationInfo = VK_NULL_HANDLE  // This member is useful in some cases
    };
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShaderModule,
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE
    };
    VkPipelineShaderStageCreateInfo computeShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = computeShaderModule,
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE
    };
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[3] = {vertexShaderStageCreateInfo, 
        fragmentShaderStageCreateInfo,
        computeShaderStageCreateInfo};

    // Create info for vertex input state
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(float) * 8,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attributeDescriptions[3];
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset = 0;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset = 3 * sizeof(float);
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;  // vec2
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].offset = 6 * sizeof(float);

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 3;
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

    // Create info for input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Create info for dynamic state: viewport state&scissor state
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
    
        // Create info for viewport state
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    // Create info for rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.f;  // optional as depthBiasEnable is set to false
    rasterizationStateCreateInfo.depthBiasClamp = 0.f;  // optional
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.f;  // optional
    rasterizationStateCreateInfo.lineWidth = 1.f;

    // Create info for multisample state(Disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.rasterizationSamples = m_MSAASampleCount;  // 1 sample means multisample disabled
    multisampleStateCreateInfo.sampleShadingEnable = m_vkPhysicalDeviceFeature.sampleRateShading;
    multisampleStateCreateInfo.minSampleShading = 0.25f;
    multisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE; // VK_NULL_HANDLE means do not mask any sample
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;  // optional
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;  // optional

    // Create info for depth stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.f,
        .maxDepthBounds = 1.f,
    };

    // Create info for Color blending state 
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // optional as we disabled color blending
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // optional
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;  // optional
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // optional
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // optional
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;  // This means: finalcolor = finalcolor & 0xFFFFFFFF
    
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;  // optional as we disable logic operation for blending color
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.blendConstants[0] = 0.f;  // optional as we set no blend factor to VK_BLEND_FACTOR_CONSTANT_COLOR or VK_BLEND_FACTOR_CONSTANT_ALPHA
    colorBlendStateCreateInfo.blendConstants[1] = 0.f;  // optional
    colorBlendStateCreateInfo.blendConstants[2] = 0.f;  // optional
    colorBlendStateCreateInfo.blendConstants[3] = 0.f;  // optional

    // Create info for graphic pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStageCreateInfos,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = VK_NULL_HANDLE,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_vkPipelineLayout,
        .renderPass = m_vkRenderPass,
        .subpass = 0,  // subpass index
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };  

    if(vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, 
        &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &m_vkPipeline) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkGraphicPipeline.");
    
    vkDestroyShaderModule(m_vkDevice, vertexShaderModule, VK_NULL_HANDLE);
    vkDestroyShaderModule(m_vkDevice, fragmentShaderModule, VK_NULL_HANDLE);
}

void Resources::createSwapChainFrameBuffers()
{
    m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());
    for(uint32_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        VkImageView frameBufferImageViews[3] = {m_colorMSAAImageView, m_depthStencilImageView, m_swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_vkRenderPass,
            .attachmentCount = 3,
            .pAttachments = frameBufferImageViews,
            .width = m_swapChainImageExtent.width,
            .height =  m_swapChainImageExtent.height,
            .layers = 1
        };
    
        if(vkCreateFramebuffer(m_vkDevice, &framebufferCreateInfo, VK_NULL_HANDLE, &m_swapChainFrameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("VK ERROR: Failed to create VkFrameBuffer.");
    }
}

void Resources::createSyncObjects()
{
    m_acquireImageSemaphores.resize(m_maxInflightFrames);
    m_drawSemaphores.resize(m_maxInflightFrames);
    m_inFlightFences.resize(m_maxInflightFrames);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        if((vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_acquireImageSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_drawSemaphores[i]) != VK_SUCCESS) || 
            (vkCreateFence(m_vkDevice, &fenceCreateInfo, VK_NULL_HANDLE, &m_inFlightFences[i]) != VK_SUCCESS))
            throw std::runtime_error("VK ERROR: Failed to create VkSemaphore or VkFence.");
    }
}

void Resources::createUniformBuffers()
{
    VkDeviceSize uniformBufferSize = sizeof(UBOMatrices);

    m_uniformBuffers.resize(m_maxInflightFrames);
    m_uniformBufferMemories.resize(m_maxInflightFrames);
    m_uniformBuffersMapped.resize(m_maxInflightFrames);

    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        createBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBuffers[i], m_uniformBufferMemories[i]);
        vkMapMemory(m_vkDevice, m_uniformBufferMemories[i], 0, uniformBufferSize, 0, &m_uniformBuffersMapped[i]);
    }
}

void Resources::createDescriptorPool()
{
    VkDescriptorPoolSize descriptorPoolSize[3];
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = m_maxInflightFrames * 2;
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize[1].descriptorCount = m_maxInflightFrames;
    descriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize[2].descriptorCount = m_maxInflightFrames * 2;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = m_maxInflightFrames * 2;
    descriptorPoolCreateInfo.poolSizeCount = 3;
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize;
    if(vkCreateDescriptorPool(m_vkDevice, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &m_vkDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDescriptorPool.");
}

void Resources::allocateDescriptorSets()
{
    // allocate draw descriptor sets
    m_drawDescriptorSets.resize(m_maxInflightFrames);
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_maxInflightFrames, m_drawDescriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_vkDescriptorPool,
        .descriptorSetCount = m_maxInflightFrames,
        .pSetLayouts = descriptorSetLayouts.data()
    };
    if(vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, m_drawDescriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate VkDescriptorSet.");
    
    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = m_uniformBuffers[i];  // 此处指明了一个descriptor set对应到哪一个buffer
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(UBOMatrices);

        VkDescriptorImageInfo descriptorImageInfo = m_model->getTextureDescriptorImageInfo();

        VkWriteDescriptorSet writeDescriptorSets[2];
        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].pNext = VK_NULL_HANDLE;
        writeDescriptorSets[0].dstSet = m_drawDescriptorSets[i];
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[0].pImageInfo = VK_NULL_HANDLE;
        writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfo;
        writeDescriptorSets[0].pTexelBufferView = VK_NULL_HANDLE;
        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].pNext = VK_NULL_HANDLE;
        writeDescriptorSets[1].dstSet = m_drawDescriptorSets[i];
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSets[1].pImageInfo = &descriptorImageInfo;
        writeDescriptorSets[1].pBufferInfo = VK_NULL_HANDLE;
        writeDescriptorSets[1].pTexelBufferView = VK_NULL_HANDLE;
        vkUpdateDescriptorSets(m_vkDevice, 2, writeDescriptorSets, 0, VK_NULL_HANDLE);
    }

    // allocate particle descriptor sets
}

void Resources::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredProperties, 
    VkBuffer& buffer, VkDeviceMemory& memory) const
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if(vkCreateBuffer(m_vkDevice, &bufferCreateInfo, VK_NULL_HANDLE, &buffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create buffer.");
    
    VkMemoryRequirements bufferMemoryRequirements = {};
    vkGetBufferMemoryRequirements(m_vkDevice, buffer, &bufferMemoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(bufferMemoryRequirements.memoryTypeBits, requiredProperties);
    if(vkAllocateMemory(m_vkDevice, &memoryAllocateInfo, VK_NULL_HANDLE, &memory) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate memory for VkBuffer.");
    
    // Bind memory for that buffer
    vkBindBufferMemory(m_vkDevice, buffer, memory, 0);
}

void Resources::createImage(int width, int height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevel,
    VkSampleCountFlagBits samples, VkMemoryPropertyFlags requiredMemoryProperty, VkImage& image, VkDeviceMemory& imageMemory) const
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipLevel;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = samples;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if(vkCreateImage(m_vkDevice, &imageCreateInfo, VK_NULL_HANDLE, &image) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkImage.");

    VkMemoryRequirements memoryRequirement = {};
    vkGetImageMemoryRequirements(m_vkDevice, image, &memoryRequirement);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirement.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirement.memoryTypeBits, requiredMemoryProperty);
    if(vkAllocateMemory(m_vkDevice, &memoryAllocateInfo, VK_NULL_HANDLE, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate memory for VkImage.");
    
    vkBindImageMemory(m_vkDevice, image, imageMemory, 0);
}

void Resources::copyBuffer2Buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    VkCommandBuffer copyCommandBuffer = beginSingleTimeCommandBuffer();

    VkBufferCopy bufferCopy = {};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = size;
    vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    endSingleTimeCommandBuffer(copyCommandBuffer);
}

void Resources::copyBuffer2Image(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const
{
    VkCommandBuffer copyCommandBuffer = beginSingleTimeCommandBuffer();

    VkBufferImageCopy bufferImageCopy = {};
    bufferImageCopy.bufferOffset = 0;
    bufferImageCopy.bufferRowLength = 0;
    bufferImageCopy.bufferImageHeight = 0;
    bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferImageCopy.imageSubresource.mipLevel = 0;
    bufferImageCopy.imageSubresource.baseArrayLayer = 0;
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageOffset = {0, 0, 0};
    bufferImageCopy.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(copyCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, &bufferImageCopy);

    endSingleTimeCommandBuffer(copyCommandBuffer);
}

void Resources::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image, uint32_t mipLevels) const
{
    VkCommandBuffer transitionLayoutCommandBuffer = beginSingleTimeCommandBuffer();
    VkAccessFlags barrierSrcAccessMask, barrierDstAccessMask;
    VkPipelineStageFlags srcStageMask, dstStageMask;
    VkImageAspectFlags aspect;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
    {
        aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrierSrcAccessMask = 0;
        barrierDstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrierSrcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierDstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
        throw std::runtime_error("VK ERROR: Specified image layout transition not implemented.");

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask = barrierSrcAccessMask;
    imageMemoryBarrier.dstAccessMask = barrierDstAccessMask;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(transitionLayoutCommandBuffer, srcStageMask, dstStageMask, 0, 
        0, VK_NULL_HANDLE,
        0, VK_NULL_HANDLE,
        1, &imageMemoryBarrier);
    
    endSingleTimeCommandBuffer(transitionLayoutCommandBuffer);
}

void Resources::createImageView(VkImageView& imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect) const
{
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = aspect;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    if(vkCreateImageView(m_vkDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkImageView.");
}

uint32_t Resources::findMemoryTypeIndex(uint32_t requiredMemoryTypeBit, 
    VkMemoryPropertyFlags requiredMemoryPropertyFlags) const
{
    VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &supportedMemoryProperties);
    for(uint32_t i = 0; i < supportedMemoryProperties.memoryTypeCount; ++i)
    {
        if((requiredMemoryTypeBit & (1 << i)) && 
            ((supportedMemoryProperties.memoryTypes[i].propertyFlags & requiredMemoryPropertyFlags) == requiredMemoryPropertyFlags))
            return i;
    }
    throw std::runtime_error("VK ERROR: Failed to find suitable memory type.");
}

VkCommandBuffer Resources::beginSingleTimeCommandBuffer() const
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_graphicCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer singleTimeCommandBuffer;
    if(vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, &singleTimeCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate single time command buffer.");
    
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if(vkBeginCommandBuffer(singleTimeCommandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to begin single time command buffer.");
    
    return singleTimeCommandBuffer;
}

void Resources::endSingleTimeCommandBuffer(VkCommandBuffer commandBuffer) const
{
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to end single time command buffer.");
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

    if(vkQueueSubmit(m_graphicQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to submit single time command buffer.");

    // Synchronization between command buffer achived by vkQueueWaitIdle
    vkQueueWaitIdle(m_graphicQueue);
    vkFreeCommandBuffers(m_vkDevice, m_graphicCommandPool, 1, &commandBuffer);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Resources::debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if(messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
    std::cerr << pCallbackData->pMessage << std::endl;
    std::cerr << "SEVERITY:";
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: std::cerr << "WARNNING\n"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   std::cerr << "ERROR\n"; break;
    }

    std::cerr << "TYPE:";
    if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        std::cerr << "GENERAL ";
    if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        std::cerr << "VALIDATION ";
    if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        std::cerr << "PERFORMANCE ";
    std::cerr << std::endl;
    return VK_FALSE;
}

void Resources::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    Resources* app = reinterpret_cast<Resources*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}

bool Resources::checkInstanceValidationLayersSupported(std::vector<const char*> validationLayerNames) const
{
    uint32_t vk_avalibleValidationLayerCount;
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, VK_NULL_HANDLE);
    std::vector<VkLayerProperties> vk_avaliableValidationLayers(vk_avalibleValidationLayerCount);
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, vk_avaliableValidationLayers.data());

    for(const char*& validationLayerName : validationLayerNames)
    {
        bool layerSupported = false;
        for(const VkLayerProperties& vk_layerProperty : vk_avaliableValidationLayers)
        {
            if(std::string(vk_layerProperty.layerName) == std::string(validationLayerName))
            {
                layerSupported = true;
                break;
            }
        }
        if(!layerSupported) return false;
    }
    return true;
}

void Resources::populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) const
{
    if(!m_enableValidationLayer) return;
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = Resources::debugMessageCallback;
    messengerCreateInfo.pUserData = VK_NULL_HANDLE;
}

std::vector<const char*> Resources::getRequiredExtentions() const
{
    uint32_t glfw_enabledExtentionCount;
    const char** glfw_enabledExtentionNames;
    glfw_enabledExtentionNames = glfwGetRequiredInstanceExtensions(&glfw_enabledExtentionCount);
    std::vector<const char*> enabledExtension(glfw_enabledExtentionNames, glfw_enabledExtentionNames + glfw_enabledExtentionCount);
    if(m_enableValidationLayer)
        enabledExtension.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // enabledExtension.emplace_back(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);  // Extension "VK_EXT_pageable_device_local_memory" is not surpported
    return enabledExtension;
}

bool Resources::checkInstanceExtensionsSupported(std::vector<const char*> extensionNames) const
{
    uint32_t vk_avaliableExtensionCount;
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &vk_avaliableExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> vk_avalibaleExtensions(vk_avaliableExtensionCount);
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &vk_avaliableExtensionCount, &vk_avalibaleExtensions[0]);
    for(const char*& extensionName : extensionNames)
    {
        bool extensionFound = false;
        for(const VkExtensionProperties& vk_extension : vk_avalibaleExtensions)
        {
            if(std::string(extensionName) == std::string(vk_extension.extensionName)) 
            {
                extensionFound = true;
                break;
            }
        }
        if(!extensionFound) return extensionFound;
    }
    return true;
}

uint64_t Resources::ratePhysicalDevice(const VkPhysicalDevice& physicalDevice) const
{
    uint64_t rate = 0;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatrues;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatrues);
    
    // Does this physical device suppport gemoetry shader?
    if(!physicalDeviceFeatrues.geometryShader) return 0;
    // Does this physical device have required queue families for operations?
    // (In our case: graphic operations and presenting operatings)?
    if(!queryRequiredQueueFamilies(physicalDevice, m_vkSurface).isComplete()) return 0;
    // Does this physical device have required logical device extensions?
    /* 
    (If a physical device supports presentation, it supports VK_KHR_swapchain extension.
    So this check is redundent.We specified here to make vulkan more exlicit.
    In our case: VK_KHR_swapchain) 
    */
    if(!checkDeviceExtensionSupported(m_deviceExtensionNames, physicalDevice)) return 0;
    // Does this physical device have required swapchain details support?That is, is the avaliable swapchain compilable with our window surface?
    // (In our case: does this physical device support at least one surface format and present mode?
    if(!querySwapChainSupportedDetails(physicalDevice, m_vkSurface).isComplete()) return 0;

    switch(physicalDeviceProperties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:     rate += 1000; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:   rate += 500; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:              rate += 100; break;
    }
    rate += (physicalDeviceProperties.limits.maxImageDimension2D >> 8);
    return rate;
}

bool Resources::checkDeviceExtensionSupported(std::vector<const char*> extensionNames, const VkPhysicalDevice& physicalDevice) const
{
    /* 
    To check if the device extension is supported.The device extensions is specified when creating logical device
    `VkDevice`, but checking process is happend when picking a physical device.Only if a physical device surrports
    the decice extensions we need can we pick it.Thus the logical device created from current physcial device supportes
    these extenstions.
    */
    uint32_t deviceExtensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, deviceExtensionProperties.data());

    for(const char*& extensionName: extensionNames)
    {
        bool extensionSupproted = false;
        for(const VkExtensionProperties& extensionProperties: deviceExtensionProperties)
        {
            if(std::string(extensionName) == std::string(extensionProperties.extensionName))
            {
                extensionSupproted = true;
                break;
            }
        }
        if(!extensionSupproted) return false;
    }
    return true;
}

Resources::RequiredQueueFamilyIndices Resources::queryRequiredQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const
{
    RequiredQueueFamilyIndices requiredQueueFamilyIndices;

    uint32_t deviceQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &deviceQueueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &deviceQueueFamilyCount, deviceQueueFamilyProperties.data());
    for(uint32_t i = 0; i < deviceQueueFamilyProperties.size(); ++i)
    {   
        // Is this queue family supported by current physical device supports presenting to surface?
        VkBool32 surfaceSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surfaceSupported);
        if(surfaceSupported)
        {
            // std::cout << "Present queue family index:" << i << std::endl;
            requiredQueueFamilyIndices.presentFamily = i;
        }
        
        // Is this queue family supported by current physical device supports graphic operation?
        // A mention: If a queue family supports graphic operation or compute operation, it implicitly supports transfer operation
        if(deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            // std::cout << "Graphic queue family index:" << i << std::endl;
            requiredQueueFamilyIndices.graphicFamily = i;
        }
        if(deviceQueueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            // std::cout << "Graphic and compute queue family index:" << i << std::endl;
            requiredQueueFamilyIndices.graphicComputeFamily = i;
        }
        
        // If current physical already provides the queue families we need.If it's enough, we end the loop.
        if(requiredQueueFamilyIndices.isComplete()) break;
    }
    return requiredQueueFamilyIndices;
}

Resources::SwapChainSupportDetails Resources::querySwapChainSupportedDetails(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const
{
    SwapChainSupportDetails swapChainSupportedDetail = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupportedDetail.surfaceCapabilities);
    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, VK_NULL_HANDLE);
    swapChainSupportedDetail.surfaceFormats.resize(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, swapChainSupportedDetail.surfaceFormats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);
    swapChainSupportedDetail.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,  swapChainSupportedDetail.presentModes.data());
    return swapChainSupportedDetail;
}

VkSurfaceFormatKHR Resources::chooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> avaliableSurfaceFormats) const
{
    for(const VkSurfaceFormatKHR& surfaceFormat: avaliableSurfaceFormats)
    {
        if(VK_FORMAT_B8G8R8A8_SRGB == surfaceFormat.format && 
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == surfaceFormat.colorSpace)
            return surfaceFormat;
    }
    return avaliableSurfaceFormats[0];
}

VkPresentModeKHR Resources::chooseSwapChainPresentMode(std::vector<VkPresentModeKHR> avalibalePresentModes) const
{
    for(const VkPresentModeKHR& presentMode: avalibalePresentModes)
    {
        // Our top prefered present mode is VK_PRESENT_MODE_MAILBOX_KHR
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;
        // Else we choose VK_PRESENT_MODE_FIFO_KHR
        else if(presentMode == VK_PRESENT_MODE_FIFO_KHR)
            return presentMode;
    }
    return avalibalePresentModes[0];
}

VkExtent2D Resources::chooseSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    if(surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max() ||
        surfaceCapabilities.currentExtent.height == std::numeric_limits<uint32_t>::max())
    {
        VkExtent2D actualExtent;
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
        actualExtent.width = std::clamp((uint32_t)windowWidth,
            surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp((uint32_t)windowHeight,
            surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        return actualExtent;
    }
    else  // In our application, surfaceCapabilities.currentExtent equals (1920, 1080), which is exactly the window size
        return surfaceCapabilities.currentExtent;
}

VkFormat Resources::findSupportedFormat(std::vector<VkFormat> formatCandidates, 
    VkImageTiling tiling, 
    VkFormatFeatureFlags desiredFeatures) const
{
    for(const VkFormat& format: formatCandidates)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, format, &formatProperties);
        if(tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & desiredFeatures == desiredFeatures))
            return format;
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & desiredFeatures == desiredFeatures))
            return format;
    }
    throw std::runtime_error("VK ERROR: Failed to find a supported VkFormat.");
}

std::vector<char> Resources::readShaderFile(const std::string filePath) const
{
    std::vector<char> buffer;
    std::ifstream ifs(filePath, std::ifstream::ate | std::ifstream::binary);
    if(!ifs.is_open())
        throw std::runtime_error("VK ERROR: Failed to read " + filePath);

    size_t fileSize = (size_t)ifs.tellg();
    buffer.resize(fileSize);
    ifs.seekg(0, std::ios::beg);
    ifs.read(buffer.data(), fileSize);
    ifs.close();  // Don't forget to close filestream
    return buffer;
}

VkShaderModule Resources::createShaderModule(std::vector<char> shaderBytes) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderBytes.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
    
    VkShaderModule shaderModule;
    if(vkCreateShaderModule(m_vkDevice, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkShaderModule.");
    return shaderModule;
}

void Resources::updateUniformBuffers(uint32_t frameIndex) const
{
    float currentTime = (float)glfwGetTime();  // unit: seconds
   
    UBOMatrices uboMatrices;
    uboMatrices.model = glm::rotate(glm::mat4(1.f), glm::radians(90.f) * currentTime, glm::vec3(0.f, 0.f, 1.f));
    uboMatrices.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
    uboMatrices.projection = glm::perspective(glm::radians(45.f), 
        float(m_swapChainImageExtent.width) / m_swapChainImageExtent.height,
        0.1f, 10.f
    );
    
    memcpy(m_uniformBuffersMapped[frameIndex], &uboMatrices, sizeof(UBOMatrices));
    memcpy(m_uniformBuffersMapped[frameIndex], &uboMatrices, sizeof(UBOMatrices));
}

void Resources::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_drawDescriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
    if(vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &m_vkPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkPipelineLayout.");
}

void Resources::recordDrawCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // First, begin recording commandbuffer
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;  // No flags for recording command buffer
    commandBufferBeginInfo.pInheritanceInfo = VK_NULL_HANDLE;  // Only used for secondary command buffer

    if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to begin recording the VkCommandBuffer.");

    // Record `begin renderpass` command
    VkRenderPassBeginInfo renderpassBeginInfo = {};
    renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBeginInfo.renderPass = m_vkRenderPass;
    renderpassBeginInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];
    renderpassBeginInfo.renderArea.offset = {0, 0};
    renderpassBeginInfo.renderArea.extent = m_swapChainImageExtent;
    VkClearValue colorAttachmentClearValue = {.color = {.float32 = {0.f, 0.f, 0.f, 1.f}}};
    VkClearValue depthAttachmentClearValue = {.depthStencil = {.depth = 1.f, .stencil = 0}};
    VkClearValue clearValues[2] = {colorAttachmentClearValue, depthAttachmentClearValue};
    renderpassBeginInfo.clearValueCount = 2;
    renderpassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Record `bind to pipeline` command, then the command buffer will use the renderpass specified in that pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
    
    // Record `bind vertex&index buffer` command
    m_model->cmdBindBuffers(commandBuffer);

    // Record `bind descriptor set` commmand
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, 
        &m_drawDescriptorSets[m_currentFrame], 0, VK_NULL_HANDLE);

    // Record `set dynamic state` command(In our case, viewport state and scissor state).
    VkViewport viewPort = {};
    viewPort.x = 0.f;
    viewPort.y = 0.f;
    viewPort.width = m_swapChainImageExtent.width;
    viewPort.height = m_swapChainImageExtent.height;
    viewPort.minDepth = 0.f;
    viewPort.maxDepth = 1.f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainImageExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Record `draw` command
    m_model->cmdDrawIndexed(commandBuffer);
    
    // Record `end renderpass` command
    vkCmdEndRenderPass(commandBuffer);

    // Finally, end recording command buffer
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to record command buffer.");
}

void Resources::cleanUpSwapChain()
{
    for(VkFramebuffer framebuffer: m_swapChainFrameBuffers)
        vkDestroyFramebuffer(m_vkDevice, framebuffer, VK_NULL_HANDLE);
    for(VkImageView imageView: m_swapChainImageViews)
        vkDestroyImageView(m_vkDevice, imageView, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, VK_NULL_HANDLE);
    
    vkDestroyImageView(m_vkDevice, m_depthStencilImageView, VK_NULL_HANDLE);
    vkDestroyImage(m_vkDevice, m_depthStencilImage, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, m_depthStencilImageMemory, VK_NULL_HANDLE);

    vkDestroyImageView(m_vkDevice, m_colorMSAAImageView, VK_NULL_HANDLE);
    vkDestroyImage(m_vkDevice, m_colorMSAAImage, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, m_colorMSAAImageMemory, VK_NULL_HANDLE);
}

void Resources::recreateSwapChain()
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    while(width == 0 || height == 0)
    {
        glfwWaitEvents();
        glfwGetFramebufferSize(m_window, &width, &height);
    }

    vkDeviceWaitIdle(m_vkDevice);
    cleanUpSwapChain();

    createSwapChain();
    createSwapChainImageViews();
    createColorResources();
    createDepthResources();
    createSwapChainFrameBuffers();
}

void Resources::createVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) const
{
    VkDeviceSize veretexBufferSize = vertices.size() * sizeof(Vertex);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(veretexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, veretexBufferSize, 0, &data);
    memcpy(data, vertices.data(), veretexBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(veretexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    copyBuffer2Buffer(stagingBuffer, vertexBuffer, veretexBufferSize);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void Resources::createIndexBuffer(std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) const
{
    VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, indexBufferSize, 0, &data);
    memcpy(data, indices.data(), indexBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    
    copyBuffer2Buffer(stagingBuffer, indexBuffer, indexBufferSize);
    
    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void Resources::cleanUp()
{
    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, VK_NULL_HANDLE);
    
    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        vkDestroyFence(m_vkDevice, m_inFlightFences[i], VK_NULL_HANDLE);
        vkDestroySemaphore(m_vkDevice, m_acquireImageSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(m_vkDevice, m_drawSemaphores[i], VK_NULL_HANDLE);

        vkDestroyBuffer(m_vkDevice, m_uniformBuffers[i], VK_NULL_HANDLE);
        vkFreeMemory(m_vkDevice, m_uniformBufferMemories[i], VK_NULL_HANDLE);
    }
    m_model->cleanUp(m_vkDevice);

    vkDestroyCommandPool(m_vkDevice, m_graphicCommandPool, VK_NULL_HANDLE);
    vkDestroyPipeline(m_vkDevice, m_vkPipeline, VK_NULL_HANDLE);
    vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(m_vkDevice, m_drawDescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, VK_NULL_HANDLE);
    cleanUpSwapChain();
    vkDestroyDevice(m_vkDevice, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, VK_NULL_HANDLE);
    if(m_enableValidationLayer)
        vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkMessenger, VK_NULL_HANDLE);
    vkDestroyInstance(m_vkInstance, VK_NULL_HANDLE);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Resources::loadModel()
{
    m_model = new Model("./model/viking_room.obj");
}

void Resources::createTexture(const char* filename, Texture& texture) const
{
    int imageWidth, imageHeight, imageChannels;
    stbi_uc* data = stbi_load(filename, &imageWidth, &imageHeight, &imageChannels, 
        STBI_rgb_alpha);
    if(!data)
        throw std::runtime_error("STBI ERROR: Failed to load image.");
    texture.path = filename;

    VkDeviceSize imageBufferSize = 4 * imageWidth * imageHeight;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* bufferData;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, imageBufferSize, 0, &bufferData);
    memcpy(bufferData, data, imageBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);
    stbi_image_free(data);

    texture.mipLevels = glm::floor(glm::log2(static_cast<float_t>(glm::max(imageWidth, imageHeight)))) + 1;
    createImage(imageWidth, imageHeight, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
        texture.mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.imageMemory);
        
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.image, texture.mipLevels);
    
    copyBuffer2Image(stagingBuffer, texture.image, static_cast<uint32_t>(imageWidth), 
        static_cast<uint32_t>(imageHeight));
    
    generateMipmaps(texture.image, VK_FORMAT_R8G8B8A8_SRGB, imageWidth, imageHeight, texture.mipLevels);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);

    createImageView(texture.imageView, texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    createSampler(texture.sampler, texture.mipLevels);
}

void Resources::createSampler(VkSampler& sampler, uint32_t mipLevel) const
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;  // 如果启用了mipmap，那么在一个mimap level中是线性插值
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.f;
    samplerCreateInfo.anisotropyEnable = m_vkPhysicalDeviceFeature.samplerAnisotropy;
    samplerCreateInfo.maxAnisotropy = m_vkPhysicalDeviceProperties.limits.maxSamplerAnisotropy;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.f;
    samplerCreateInfo.maxLod = static_cast<float>(mipLevel - 1);
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    if(vkCreateSampler(m_vkDevice, &samplerCreateInfo, VK_NULL_HANDLE, &sampler) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkSampler.");
}

void Resources::mainLoop()
{
    if(!m_complete)
        throw std::runtime_error("VK ERROR: Vulkan resources have not been intialized yet.");
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(m_vkDevice);
}

void Resources::generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels) const
{
    // Assert that the image format supports linear
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, format, &formatProperties);
    if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("VK ERROR: Image format does not support mipmap linear filtering, generating mipmaps is rejected.");

    VkCommandBuffer genMipCommandBuffer = beginSingleTimeCommandBuffer();
    VkImageMemoryBarrier imageMemoryBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = VK_NULL_HANDLE,
        // .srcAccessMask = 0,
        // .dstAccessMask = 0,
        // .oldLayout = VK_IMAGE_LAYOUT_MAX_ENUM,
        // .newLayout = VK_IMAGE_LAYOUT_MAX_ENUM,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            // .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    uint32_t imageWidth = width, imageHeight = height; 
    for(uint32_t i = 1; i < mipLevels; ++i)
    {
        // transition mipmap level i - 1 from VK_IMAGE_LAYOUT_TRANSFER_DST to VK_IMAGE_LAYOUT_TRANSFER_SRC
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;  // vkCmdBlitImage & vkCopyBufferToImage are transfer operations
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
        vkCmdPipelineBarrier(genMipCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
            0,
            0, VK_NULL_HANDLE,
            0, VK_NULL_HANDLE,
            1, &imageMemoryBarrier);

        // blit image data from mip level i - 1 to i
        VkImageBlit imageBlit = {
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets = {
                {0, 0, 0},
                {static_cast<int32_t>(imageWidth), static_cast<int32_t>(imageHeight), 1}
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets = {
                {0, 0, 0},
                {imageWidth > 1 ? static_cast<int32_t>(imageWidth / 2) : 1, imageHeight > 1 ? static_cast<int32_t>(imageHeight / 2) : 1, 1}
            }
        };
        vkCmdBlitImage(genMipCommandBuffer, 
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageBlit, VK_FILTER_LINEAR);
        
        // transition miplevel i - 1 from VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
        vkCmdPipelineBarrier(genMipCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 
            0, VK_NULL_HANDLE,
            0, VK_NULL_HANDLE, 
            1, &imageMemoryBarrier);

        if(imageWidth > 1) imageWidth /= 2;
        if(imageHeight > 1) imageHeight /= 2;
    }

    // transition miplevel - 1 from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
    vkCmdPipelineBarrier(genMipCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 
        0, VK_NULL_HANDLE,
        0, VK_NULL_HANDLE, 
        1, &imageMemoryBarrier);
    
    endSingleTimeCommandBuffer(genMipCommandBuffer);
}

Resources* Resources::instance = VK_NULL_HANDLE;

Resources* Resources::get()
{
    if(!instance)
        instance = new Resources();
    return instance;
}

void Resources::cleanUpTexture(const Texture& texture) const
{
    vkDestroySampler(m_vkDevice, texture.sampler, VK_NULL_HANDLE);
    vkDestroyImageView(m_vkDevice, texture.imageView, VK_NULL_HANDLE);
    vkDestroyImage(m_vkDevice, texture.image, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, texture.imageMemory, VK_NULL_HANDLE);
}

VkSampleCountFlagBits Resources::getMSAASampleCount() const
{
    VkSampleCountFlags sampleCountFlags = m_vkPhysicalDeviceProperties.limits.framebufferColorSampleCounts 
        & m_vkPhysicalDeviceProperties.limits.framebufferDepthSampleCounts;
    for(VkSampleCountFlagBits sampleCountFlagBit : std::vector<VkSampleCountFlagBits>({VK_SAMPLE_COUNT_64_BIT, 
        VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT,
        VK_SAMPLE_COUNT_1_BIT}))
    {
        if(sampleCountFlags & sampleCountFlagBit)
            return sampleCountFlagBit;
    }
    throw std::runtime_error("VK ERROR: Failed to get a valid MSAA sample count");
}

void Resources::createColorResources()
{
    createImage(m_swapChainImageExtent.width, m_swapChainImageExtent.height, m_swapChainImageFormat, 
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 1, m_MSAASampleCount, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorMSAAImage, m_colorMSAAImageMemory);
    createImageView(m_colorMSAAImageView, m_colorMSAAImage, m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Resources::createParticleSSBOs(std::vector<VkBuffer>& particleSSBOs, std::vector<VkDeviceMemory>& particleSSBOMemories,
    const std::vector<Particle>& particles) const
{
    uint32_t bufferSize = sizeof(Particle) * particles.size();
    particleSSBOs.resize(m_maxInflightFrames);
    particleSSBOMemories.resize(m_maxInflightFrames);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, 
        stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), bufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particleSSBOs[i], particleSSBOMemories[i]);

        copyBuffer2Buffer(stagingBuffer, particleSSBOs[i], bufferSize);
    }

    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void Resources::allocateParticleDescriptorSets(std::vector<VkDescriptorSet> particleDescriptorSets) const
{
    
}