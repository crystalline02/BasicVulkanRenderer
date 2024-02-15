#pragma once

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include <vector>
#include <set>
#include <string>
#include <optional> 

class Resources;

class VulkanApp
{
public:
    void run();
private:
    void initVulkan();
    
    Resources* m_appResources;
};