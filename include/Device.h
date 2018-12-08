#ifndef DEVICE_H_
#define DEVICE_H_ 1

#include "vulkan/vulkan.h"

#include <vector>

class Device
{
public:
    Device(VkPhysicalDevice physicalDevice, std::vector<const char*>& enabledExtensions, std::vector<const char*>& enabledLayers);
    ~Device();

private:
    VkPhysicalDevice mPhysicalDevice;

    VkPhysicalDeviceFeatures mDeviceFeatures;
    VkPhysicalDeviceProperties mDeviceProperties;
    VkPhysicalDeviceMemoryProperties mMemoryProps;
    std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;

    struct
    {
        uint32_t graphics = -1;
        uint32_t compute = -1;
        uint32_t transfer = -1;
    } mQueueFamilyIndices;

    struct
    {
        VkQueue graphics = VK_NULL_HANDLE;
        VkQueue compute = VK_NULL_HANDLE;
        VkQueue transfer = VK_NULL_HANDLE;
    } mQueues;

    VkDevice mDevice;
};

#endif // DEVICE_H_