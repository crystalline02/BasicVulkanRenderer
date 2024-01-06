#pragma once

class VulkanApp
{
public:
    void run();
private:
    void init_vulkan();
    void main_loop();
    void clean_up(); 
};