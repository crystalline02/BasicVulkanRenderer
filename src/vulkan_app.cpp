#include "vulkan_app.h"

#include <stdexcept>
#include <iostream>

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
    create_instance();
    if(m_enableValidationLayer)
        create_messager_callback();
}

void VulkanApp::create_instance()
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
    if(m_enableValidationLayer)
    {
        if(!checkValidationLayersSurpported(m_validationLayerNames))
            throw std::runtime_error("VK ERROR: Unsurpported vulkan instance validation layer.");
        createInfo.enabledLayerCount = (uint32_t)m_validationLayerNames.size();
        createInfo.ppEnabledLayerNames = m_validationLayerNames.data();
    }
    else
        createInfo.enabledLayerCount = 0;

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

void VulkanApp::create_messager_callback()
{
    if(!m_enableValidationLayer) return;  // Release
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VulkanApp::debugMessageCallback;
    createInfo.pUserData = nullptr;
    if(VulkanApp::createDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create VkDebugUtilMessenger");
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
        destoryDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
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
    std::cerr << "Validation error: " << pCallbackData->pMessage << std::endl;
    std::cerr << "SEVERITY:";
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: std::cerr << "VERBOSE\n"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    std::cerr << "INFO\n"; break;
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

VkResult VulkanApp::createDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func_ptr = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if(!func_ptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    else
        return func_ptr(instance, pCreateInfo, pAllocator, pMessenger);
}

void VulkanApp::destoryDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func_ptr = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if(func_ptr)
        return func_ptr(instance, messenger, pAllocator);
}