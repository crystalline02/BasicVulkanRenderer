#include "vulkan_fn.h"

#include <stdexcept>

PFN_vkCreateDebugUtilsMessengerEXT vulkan_createDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT vulkan_destroyDebugUtilsMessengerEXT = nullptr;

VkResult load_vkInstanceFunctions(const VkInstance& instance)
{
    vulkan_createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(!vulkan_createDebugUtilsMessengerEXT) return VK_ERROR_EXTENSION_NOT_PRESENT;
    vulkan_destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(!vulkan_destroyDebugUtilsMessengerEXT) return VK_ERROR_EXTENSION_NOT_PRESENT;
    return VK_SUCCESS;
}