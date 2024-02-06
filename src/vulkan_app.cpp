#include "vulkan_app.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <map>
#include <limits>
#include <algorithm>

#include "vulkan_fn.h"
#include "mesh.h"

void VulkanApp::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}

void VulkanApp::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan Renderer", VK_NULL_HANDLE, VK_NULL_HANDLE);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetWindowUserPointer(m_window, this);
}

void VulkanApp::initVulkan()
{
    createInstance();
    if(load_vkInstanceFunctions(m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to load vulkan instance functions.");
    if(m_enableValidationLayer)
        createDebugMessenger();
    createWindowSurface();
    pickPhysicalDevice();
    createLogicalDevices();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFrameBuffers();
    createDescriptorSetLayout();
    createGraphicPipeline();
    createGraphicCommandPool();
    allocateCommandBuffers();
    createSyncObjects();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void VulkanApp::createInstance()
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
}

void VulkanApp::createDebugMessenger()
{
    if(!m_enableValidationLayer) return;
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    populateMessengerCreateInfo(messengerCreateInfo);
    if(vkCreateDebugUtilsMessengerEXT(m_vkInstance, &messengerCreateInfo, VK_NULL_HANDLE, &m_vkMessenger) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create vkDebugUtilsMessengerEXT.");
}

void VulkanApp::createWindowSurface()
{
    if(glfwCreateWindowSurface(m_vkInstance, m_window, VK_NULL_HANDLE, &m_vkSurface) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkSurfaceKHR");
}

void VulkanApp::pickPhysicalDevice()
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
        
        // Log selected GPU
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &deviceProperties);
        std::cout << "VK INFO: " << deviceProperties.deviceName << " is selected as a vulkan physical device.Score: " 
            << score_devices.rbegin()->first <<  ".\n";
    }
    else throw std::runtime_error("VK ERROR: No suitable physical device for current application.");
}

void VulkanApp::createLogicalDevices()
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
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndices.graphicFamily.value(), 0, &m_vkGraphicQueue);
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndices.presentFamily.value(), 0, &m_vkPresentQueue);
}

void VulkanApp::createSwapChain()
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

void VulkanApp::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for(uint32_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_swapChainImageFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 指定这个image view将要使用这个image的那个aspect
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if(vkCreateImageView(m_vkDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &m_swapChainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("VK ERROR: Failed to create VkImageView for swap chain");
    }
}

void VulkanApp::createRenderPass()
{
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = m_swapChainImageFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // What to do before the renderpass for color&depth attachment
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // What to do after the renderpass for color&depth attachment
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // What to do before the renderpass for stencil attachment
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;   // What to do after the renderpass for stencil attachment
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // Image layout before the renderpass
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Image layout after the renderpass: present to surface

    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // Image layout during the render pass

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;

    VkSubpassDependency subpassDependency;
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if(vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, VK_NULL_HANDLE, &m_vkRenderPass) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkRenderPass");
}

void VulkanApp::createDescriptorSetLayout()
{
    // Specify bindings
    // Specify Matrices binding
    VkDescriptorSetLayoutBinding matricesLayoutBinding;
    matricesLayoutBinding.binding = 0;
    matricesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matricesLayoutBinding.descriptorCount = 1;
    matricesLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    matricesLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &matricesLayoutBinding;
    if(vkCreateDescriptorSetLayout(m_vkDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_vkDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDescriptorSetLayout.");
}

void VulkanApp::createGraphicPipeline()
{
    // Create info for Shader stage
    std::vector<char> vertexShaderBytes = readShaderFile("./shaders/triangle_vert.spv");
    std::vector<char> fragmentShaderBytes = readShaderFile("./shaders/triangle_frag.spv");
    
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderBytes);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderBytes);
    
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";  // Entry point
    vertexShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;  // This member is useful in some cases

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";
    fragmentShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    // Create info for vertex input state
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(float) * 5;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2];
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;  // vec2
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset = 0;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].offset = 2 * sizeof(float);

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
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
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.f;  // optional as depthBiasEnable is set to false
    rasterizationStateCreateInfo.depthBiasClamp = 0.f;  // optional
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.f;  // optional
    rasterizationStateCreateInfo.lineWidth = 1.f;

    // Create info for multisample state(Disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // 1 sample means multisample disabled
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;  // as we only specify one sample, no need to enable sampleShading
    multisampleStateCreateInfo.minSampleShading = 1.f;  // optional
    multisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE; // VK_NULL_HANDLE means do not mask any sample
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;  // optional
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;  // optional

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

    // Create pipeline layout
    m_vkPipelineLayout = createPipelineLayout();

    // Create info for graphic pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pTessellationState = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = m_vkRenderPass;
    graphicsPipelineCreateInfo.subpass = 0; // subpass index
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, 
        &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &m_vkPipeline) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkGraphicPipeline.");
    
    vkDestroyShaderModule(m_vkDevice, vertexShaderModule, VK_NULL_HANDLE);
    vkDestroyShaderModule(m_vkDevice, fragmentShaderModule, VK_NULL_HANDLE);
}

void VulkanApp::createFrameBuffers()
{
    m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());
    for(uint32_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = m_vkRenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &m_swapChainImageViews[i];
        framebufferCreateInfo.width = m_swapChainImageExtent.width;
        framebufferCreateInfo.height =  m_swapChainImageExtent.height;
        framebufferCreateInfo.layers = 1;
    
        if(vkCreateFramebuffer(m_vkDevice, &framebufferCreateInfo, VK_NULL_HANDLE, &m_swapChainFrameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("VK ERROR: Failed to create VkFrameBuffer.");
    }
}

void VulkanApp::createVertexBuffer()
{
    VkDeviceSize veretexBufferSize = m_vertices.size() * sizeof(m_vertices[0]);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(veretexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, veretexBufferSize, 0, &data);
    memcpy(data, m_vertices.data(), veretexBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(veretexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
    copyBuffer(stagingBuffer, m_vertexBuffer, veretexBufferSize);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void VulkanApp::createIndexBuffer()
{
    VkDeviceSize indexBufferSize = m_indices.size() * sizeof(m_indices[0]);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, indexBufferSize, 0, &data);
    memcpy(data, m_indices.data(), indexBufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);
    
    copyBuffer(stagingBuffer, m_indexBuffer, indexBufferSize);
    
    vkDestroyBuffer(m_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void VulkanApp::createUniformBuffers()
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

void VulkanApp::createDescriptorSets()
{
    m_descriptorSets.resize(m_maxInflightFrames);
    VkDescriptorSetLayout descriptorSetLayout[] = {VkDescriptorSetLayout(m_vkDescriptorSetLayout), 
        VkDescriptorSetLayout(m_vkDescriptorSetLayout)};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = m_maxInflightFrames;
    descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayout;
    if(vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, m_descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to allocate VkDescriptorSet.");
    
    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = m_uniformBuffers[i];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(UBOMatrices);

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = m_descriptorSets[i];
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pImageInfo = VK_NULL_HANDLE;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
        writeDescriptorSet.pTexelBufferView = VK_NULL_HANDLE;
        vkUpdateDescriptorSets(m_vkDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
    }
}

void VulkanApp::createDescriptorPool()
{
    VkDescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = m_maxInflightFrames;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = m_maxInflightFrames;
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
    if(vkCreateDescriptorPool(m_vkDevice, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &m_vkDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDescriptorPool.");
}

void VulkanApp::createSyncObjects()
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

void VulkanApp::createGraphicCommandPool()
{
    RequiredQueueFamilyIndices requriedQueueFamilyIndices = queryRequiredQueueFamilies(m_vkPhysicalDevice, m_vkSurface);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // Reset only the relevant command buffer(No influence for other command buffers).
    commandPoolCreateInfo.queueFamilyIndex = requriedQueueFamilyIndices.graphicFamily.value();

    if(vkCreateCommandPool(m_vkDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &m_graphicCommandPool) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkCommandPool.");
}

void VulkanApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredProperties, 
    VkBuffer& buffer, VkDeviceMemory& memory)
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
        throw std::runtime_error("VK ERROR: Failed to allocate memory.");
    
    // Bind memory for that buffer
    vkBindBufferMemory(m_vkDevice, buffer, memory, 0);
}

void VulkanApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    // Allocate command buffer for copying staging buffer to vertex buffer
    VkCommandBuffer transferCommandBuffer;
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_graphicCommandPool;  // command pool created for graphic queue also supports memory transfer operation
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_vkDevice, &allocateInfo, &transferCommandBuffer);

    // Record this command buffer(Transfer memory)
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if(vkBeginCommandBuffer(transferCommandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("VK ERORR: Failed to begin recording VkCommandBuffer.");

    VkBufferCopy bufferCopy = {};
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = size;
    vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    if(vkEndCommandBuffer(transferCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to end recoding VkCommandBuffer");
    
    // Submit the recorded command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer;
    vkQueueSubmit(m_vkGraphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vkGraphicQueue);  // Wait until copying completed

    vkFreeCommandBuffers(m_vkDevice, m_graphicCommandPool, 1, &transferCommandBuffer);
}

void VulkanApp::allocateCommandBuffers()
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

void VulkanApp::drawFrame()
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

    if(vkQueueSubmit(m_vkGraphicQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to submit drawCommandBuffer to graphic queue.");

    // Present image to screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_drawSemaphores[m_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_vkSwapChain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = VK_NULL_HANDLE;
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

void VulkanApp::recordDrawCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
    VkClearValue clearColor = {.color = {.float32 = {0.f, 0.f, 0.f, 1.f}}};
    renderpassBeginInfo.clearValueCount = 1;
    renderpassBeginInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Record `bind to pipeline` commands, then the command buffer will use the renderpass specified in that pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
    
    // Record `bind vertex/index buffer` commands
    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

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
    vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
    
    // Record `end renderpass` command
    vkCmdEndRenderPass(commandBuffer);

    // Finally, end recording command buffer
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to record command buffer.");
}

void VulkanApp::cleanUpSwapChain()
{
    for(VkFramebuffer framebuffer: m_swapChainFrameBuffers)
        vkDestroyFramebuffer(m_vkDevice, framebuffer, VK_NULL_HANDLE);
    for(VkImageView imageView: m_swapChainImageViews)
        vkDestroyImageView(m_vkDevice, imageView, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, VK_NULL_HANDLE);
}

void VulkanApp::recreateSwapChain()
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
    createImageViews();
    createFrameBuffers();
}

uint32_t VulkanApp::findMemoryTypeIndex(uint32_t requiredMemoryTypeBit, 
    VkMemoryPropertyFlags requirdMemoryPropertyFlags) const
{
    VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &supportedMemoryProperties);
    for(uint32_t i = 0; i < supportedMemoryProperties.memoryTypeCount; ++i)
    {
        if((requiredMemoryTypeBit & (1 << i)) && 
            ((supportedMemoryProperties.memoryTypes[i].propertyFlags & requirdMemoryPropertyFlags) == requirdMemoryPropertyFlags))
            return i;
    }
    throw std::runtime_error("VK ERROR: Failed to find suitable memory type.");
}

void VulkanApp::populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) const
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
    messengerCreateInfo.pfnUserCallback = VulkanApp::debugMessageCallback;
    messengerCreateInfo.pUserData = VK_NULL_HANDLE;
}

uint64_t VulkanApp::ratePhysicalDevice(const VkPhysicalDevice& physicalDevice) const
{
    uint64_t rate = 0;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatrues;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatrues);
    
    // Does this physical device suppport gemoetry shader?
    if(!physicalDeviceFeatrues.geometryShader) return 0;
    // Does this physical device have required queue families for operations
    // (In our case: graphic operations and presenting operatings)?
    if(!queryRequiredQueueFamilies(physicalDevice, m_vkSurface).isComplete()) return 0;
    // Does this physical device have required logical device extensions?
    // (If a physical device supports presentation, it supports VK_KHR_swapchain extension.
    // (So this check is redundent.We specified here to make vulkan more exlicit.)
    // (In our case: VK_KHR_swapchain)
    if(!checkDeviceExtensionSupported(m_deviceExtensionNames, physicalDevice)) return 0;
    // Does this physical device have required swapchain details support, that is, is the avaliable swapchain compilable with our window surface?
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

void VulkanApp::mainLoop()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(m_vkDevice);
}

void VulkanApp::updateUniformBuffers(uint32_t frameIndex)
{
    float currentTime = (float)glfwGetTime();
   
    UBOMatrices uboMatrices;
    uboMatrices.model = glm::rotate(glm::mat4(1.f), glm::radians(90.f) * currentTime, glm::vec3(0.f, 0.f, 1.f));
    uboMatrices.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    uboMatrices.projection = glm::perspective(glm::radians(45.f), 
        float(m_swapChainImageExtent.width) / m_swapChainImageExtent.height,
        0.1f, 10.f
    );
    
    memcpy(m_uniformBuffersMapped[frameIndex], &uboMatrices, sizeof(UBOMatrices));
}

void VulkanApp::cleanUp()
{
    for(uint32_t i = 0; i < m_maxInflightFrames; ++i)
    {
        vkDestroyFence(m_vkDevice, m_inFlightFences[i], VK_NULL_HANDLE);
        vkDestroySemaphore(m_vkDevice, m_acquireImageSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(m_vkDevice, m_drawSemaphores[i], VK_NULL_HANDLE);

        vkDestroyBuffer(m_vkDevice, m_uniformBuffers[i], VK_NULL_HANDLE);
        vkFreeMemory(m_vkDevice, m_uniformBufferMemories[i], VK_NULL_HANDLE);
    }
    vkDestroyBuffer(m_vkDevice, m_indexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, m_indexBufferMemory, VK_NULL_HANDLE);
    vkDestroyBuffer(m_vkDevice, m_vertexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_vkDevice, m_vertexBufferMemory, VK_NULL_HANDLE);
    vkDestroyCommandPool(m_vkDevice, m_graphicCommandPool, VK_NULL_HANDLE);
    vkDestroyPipeline(m_vkDevice, m_vkPipeline, VK_NULL_HANDLE);
    vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, VK_NULL_HANDLE);
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

bool VulkanApp::checkInstanceExtensionsSupported(std::vector<const char*> extensionNames) const
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

bool VulkanApp::checkInstanceValidationLayersSupported(std::vector<const char*> validationLayerNames) const
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

bool VulkanApp::checkDeviceExtensionSupported(std::vector<const char*> extensionNames, const VkPhysicalDevice& physicalDevice) const
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

VulkanApp::RequiredQueueFamilyIndices VulkanApp::queryRequiredQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const
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
            requiredQueueFamilyIndices.presentFamily = i;
        
        // Is this queue family supported by current physical device supports graphic operation?
        // A mention: If a queue family supports graphic operation or compute operation, it implicitly supports transfer operation
        if(deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            requiredQueueFamilyIndices.graphicFamily = i;
        
        // If current physical already provides the queue families we need.If it's enough, we end the loop.
        if(requiredQueueFamilyIndices.isComplete()) break;
    }
    return requiredQueueFamilyIndices;
}

VulkanApp::SwapChainSupportDetails VulkanApp::querySwapChainSupportedDetails(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) const
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

VkSurfaceFormatKHR VulkanApp::chooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> avaliableSurfaceFormats)
{
    for(const VkSurfaceFormatKHR& surfaceFormat: avaliableSurfaceFormats)
    {
        if(VK_FORMAT_B8G8R8A8_SRGB == surfaceFormat.format && 
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == surfaceFormat.colorSpace)
            return surfaceFormat;
    }
    return avaliableSurfaceFormats[0];
}

VkPresentModeKHR VulkanApp::chooseSwapChainPresentMode(std::vector<VkPresentModeKHR> avalibalePresentModes)
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

VkExtent2D VulkanApp::chooseSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities)
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

std::vector<const char*> VulkanApp::getRequiredExtentions() const
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

std::vector<char> VulkanApp::readShaderFile(const std::string filePath) const
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

VkShaderModule VulkanApp::createShaderModule(std::vector<char> shaderBytes) const
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

VkPipelineLayout VulkanApp::createPipelineLayout() const
{
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_vkDescriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
    if(vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkPipelineLayout.");
    return pipelineLayout;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

void VulkanApp::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    VulkanApp* app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}
