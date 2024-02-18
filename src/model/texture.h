# pragma once

#include <vulkan/vulkan.h>

struct Texture
{
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;

    uint32_t mipLevels;
    const char* path;
};