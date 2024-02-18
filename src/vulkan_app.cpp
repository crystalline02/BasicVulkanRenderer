#include "vulkan_app.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <map>
#include <limits>
#include <algorithm>

#include "resources.h"

void VulkanApp::run()
{
    m_appResources = new Resources();
    
    m_appResources->initWindow();
    initVulkan();
    m_appResources->mainLoop();
    m_appResources->cleanUp();
}

void VulkanApp::initVulkan()
{
    m_appResources->createInstance();

    if(m_appResources->isValidationLayerEnbaled())
        m_appResources->createDebugMessenger();
    m_appResources->createWindowSurface();
    m_appResources->pickPhysicalDevice();
    m_appResources->createLogicalDevices();
    m_appResources->createSwapChain();
    m_appResources->createSwapChainImageViews();
    m_appResources->createDepthResources();

    m_appResources->createGraphicCommandPool();
    m_appResources->allocateDrawCommandBuffers();

    m_appResources->createDescriptorSetLayout();
    m_appResources->createRenderPass();
    m_appResources->createGraphicPipeline();
    m_appResources->createSwapChainFrameBuffers();

    m_appResources->createSyncObjects();

    m_appResources->createDescriptorPool();
    m_appResources->loadModel();
    m_appResources->createUniformBuffers();
    m_appResources->allocateDescriptorSets();

    m_appResources->m_complete = true;
}


