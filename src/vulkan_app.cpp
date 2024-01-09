#include "vulkan_app.h"

#include <stdexcept>
#include <iostream>
#include <map>

#include "vulkan_fn.h"

void VulkanApp::run()
{
    init_window();
    init_vulkan();
    main_loop();
    clean_up();
}

void VulkanApp::init_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan Renderer", nullptr, nullptr);
}

void VulkanApp::init_vulkan()
{
    createInstance();
    if(load_vkInstanceFunctions(m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to load vulkan instance functions.");
    if(m_enableValidationLayer)
        createDebugMessenger();
    createPhysicalDevices();
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
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;  // createInfo use applicationInfo as a part of its struct members

    // Add&check validation layers
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    if(m_enableValidationLayer)
    {
        if(!checkValidationLayersSurpported(m_validationLayerNames))
            throw std::runtime_error("VK ERROR: Unsurpported vulkan instance validation layer.");
        createInfo.enabledLayerCount = (uint32_t)m_validationLayerNames.size();
        createInfo.ppEnabledLayerNames = m_validationLayerNames.data();
        // Populate messengerCreateInfo
        populateMessengerCreateInfo(messengerCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // Add&check extensions
    m_enabledExtensionNames = getRequiredExtentions();
    if(!checkExtensionsSurpported(m_enabledExtensionNames))
        throw std::runtime_error("VK ERROR: Unsurpported vulkan instance extensions.");
    createInfo.enabledExtensionCount = (uint32_t)m_enabledExtensionNames.size();
    createInfo.ppEnabledExtensionNames = m_enabledExtensionNames.data();

    // finally create intance
    if(vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkInstance.");
}

void VulkanApp::createDebugMessenger()
{
    if(!m_enableValidationLayer) return;
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    populateMessengerCreateInfo(messengerCreateInfo);
    if(vkCreateDebugUtilsMessengerEXT(m_vkInstance, &messengerCreateInfo, nullptr, &m_vkMessenger) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create vkDebugUtilsMessengerCreateInfoEXT.");
}

void VulkanApp::createPhysicalDevices()
{
    // Get all physical devices
    uint32_t vk_physicalDeviceCount;
    vkEnumeratePhysicalDevices(m_vkInstance, &vk_physicalDeviceCount, nullptr);
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
        m_vkPhysicalDevice = score_devices.rbegin()->second;
        
        // Log selected GPU
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &deviceProperties);
        std::cout << "VK INFO: " << deviceProperties.deviceName << " is selected as a vulkan physical device.Score: " 
            << score_devices.rbegin()->first <<  ".\n";
    }
    else throw std::runtime_error("VK ERROR: No suitable physical device for current application.");
}

void VulkanApp::populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo)
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
    messengerCreateInfo.pUserData = nullptr;
}

uint64_t VulkanApp::ratePhysicalDevice(const VkPhysicalDevice& physicalDevice)
{
    uint64_t rate = 0;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatrues;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatrues);
    if(!physicalDeviceFeatrues.geometryShader) return 0;
    switch(physicalDeviceProperties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:     rate += 1000; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:   rate += 500; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:              rate += 100; break;
    }
    rate += (physicalDeviceProperties.limits.maxImageDimension2D >> 8);
    return rate;
}

void VulkanApp::main_loop()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void VulkanApp::clean_up()
{
    if(m_enableValidationLayer)
        vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkMessenger, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool VulkanApp::checkExtensionsSurpported(std::vector<const char*> extensionNames)
{
    uint32_t vk_avaliableExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_avaliableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vk_avalibaleExtensions(vk_avaliableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_avaliableExtensionCount, &vk_avalibaleExtensions[0]);
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

bool VulkanApp::checkValidationLayersSurpported(std::vector<const char*> validationLayerNames)
{
    uint32_t vk_avalibleValidationLayerCount;
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, nullptr);
    std::vector<VkLayerProperties> vk_avaliableValidationLayers(vk_avalibleValidationLayerCount);
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, vk_avaliableValidationLayers.data());
    for(const char*& validationLayerName : validationLayerNames)
    {
        bool foundLayer = false;
        for(const VkLayerProperties& vk_layerProperty : vk_avaliableValidationLayers)
        {
            if(std::string(vk_layerProperty.layerName) == std::string(validationLayerName))
            {
                foundLayer = true;
                break;
            }
        }
        if(!foundLayer) return false;
    }
    return true;
}

std::vector<const char*> VulkanApp::getRequiredExtentions()
{
    uint32_t glfw_enabledExtentionCount;
    const char** glfw_enabledExtentionNames;
    glfw_enabledExtentionNames = glfwGetRequiredInstanceExtensions(&glfw_enabledExtentionCount);
    std::vector<const char*> enabledExtension(glfw_enabledExtentionNames, glfw_enabledExtentionNames + glfw_enabledExtentionCount);
    if(m_enableValidationLayer)
        enabledExtension.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return enabledExtension;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if(messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
    std::cerr << "Validation error: " << pCallbackData->pMessage << std::endl;
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
