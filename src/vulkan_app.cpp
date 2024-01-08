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
        if(!checkValidationLayersSurpported(m_validationLayers.data(), (uint32_t)m_validationLayers.size()))
            throw std::runtime_error("VK ERROR: Unsurpported vulkan instance validation layer.");
        createInfo.enabledLayerCount = (uint32_t)m_validationLayers.size();
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    // Add&check glfw extensions
    uint32_t glfw_enabledExtensionCount;
    const char** glfw_enabledExtensionNames;
    glfw_enabledExtensionNames = glfwGetRequiredInstanceExtensions(&glfw_enabledExtensionCount);
    if(!checkExtensionsSurpported(glfw_enabledExtensionNames, glfw_enabledExtensionCount))
        throw std::runtime_error("VK ERROR: Unsurpported vulkan instance extensions.");
    for(uint32_t i = 0; i < glfw_enabledExtensionCount; ++i)
        m_enabledExtensionNames.emplace_back(glfw_enabledExtensionNames[i]);
    createInfo.enabledExtensionCount = (uint32_t)m_enabledExtensionNames.size();
    createInfo.ppEnabledExtensionNames = m_enabledExtensionNames.data();

    // finally create intance
    if(vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("VK ERROR: Failed to create vulkan instance.");
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
    vkDestroyInstance(m_vkInstance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool VulkanApp::checkExtensionsSurpported(const char** enabledExtensionNames, uint32_t enabledExtensionCount)
{
    uint32_t vk_avaliableExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_avaliableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vk_avalibaleExtensions(vk_avaliableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_avaliableExtensionCount, &vk_avalibaleExtensions[0]);
    for(uint32_t i = 0; i < enabledExtensionCount; ++i)
    {
        std::string glfw_extensionName = enabledExtensionNames[i];
        bool extensionFound = false;
        for(const VkExtensionProperties& vk_extension : vk_avalibaleExtensions)
        {
            if(glfw_extensionName == std::string(vk_extension.extensionName)) 
            {
                extensionFound = true;
                break;
            }
        }
        if(!extensionFound) return extensionFound;
    }
    return true;
}

bool VulkanApp::checkValidationLayersSurpported(const char** validationLayers, uint32_t validaitonLayersCount)
{
    uint32_t vk_avalibleValidationLayerCount;
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, nullptr);
    std::vector<VkLayerProperties> vk_avaliableValidationLayers(vk_avalibleValidationLayerCount);
    vkEnumerateInstanceLayerProperties(&vk_avalibleValidationLayerCount, vk_avaliableValidationLayers.data());
    for(uint32_t i = 0; i < validaitonLayersCount; ++i)
    {
        std::string validationLayerName = validationLayers[i];
        bool foundLayer = false;
        for(const VkLayerProperties& vk_layerProperty : vk_avaliableValidationLayers)
        {
            if(std::string(vk_layerProperty.layerName) == validationLayerName)
            {
                foundLayer = true;
                break;
            }
        }
        if(!foundLayer) return false;
    }
    return true;
}