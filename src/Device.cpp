#include "Device.h"
#include "Error.h"
#include "Buffer.h"

Device::Device(
    VkPhysicalDevice physicalDevice,
    std::vector<const char*>& enabledExtensions,
    std::vector<const char*>& enabledLayers)
{
    mPhysicalDevice = physicalDevice;

    vkGetPhysicalDeviceFeatures(physicalDevice, &mDeviceFeatures);
    vkGetPhysicalDeviceProperties(physicalDevice, &mDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mMemoryProps);

    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
    mQueueFamilyProperties.resize(queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, mQueueFamilyProperties.data());

    // Grpahics queue index
    for (int i = 0; i < queueFamilyPropertiesCount; i++)
    {
        if (mQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            mQueueFamilyIndices.graphics = i;
            break;
        }
    }

    // Compute queue index
    for (int i = 0; i < queueFamilyPropertiesCount; i++)
    {
        if ((mQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(mQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            mQueueFamilyIndices.compute = i;
            break;
        }
    }
    if (mQueueFamilyIndices.compute = -1)
    {
        mQueueFamilyIndices.compute = mQueueFamilyIndices.graphics;
    }

    // Transfer queue index
    for (int i = 0; i < queueFamilyPropertiesCount; i++)
    {
        if (mQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            mQueueFamilyIndices.transfer = i;
            break;
        }
    }
    if (mQueueFamilyIndices.transfer = -1)
    {
        mQueueFamilyIndices.transfer = mQueueFamilyIndices.graphics;
    }

    // A queue is created along with the creation of VkDevice
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1; // For we create only one instance of compute queue
    const float queuePriority = 0.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfo.queueFamilyIndex = mQueueFamilyIndices.compute;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1; // For we create only one type of queue - compute
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    /* Apparently these are not the same extensions as the ones we used for VkInstance
    deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();*/
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();

    VK_CHECK_ERR(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &mDevice));
    vkGetDeviceQueue(mDevice, mQueueFamilyIndices.compute, 0, &(mQueues.compute));
}

Device::~Device()
{

}

Buffer* Device::CreateBuffer(size_t size, bool isHostVisible)
{
    Buffer* buffer = new Buffer(this, size, isHostVisible);
    return buffer;
}

uint32_t Device::GetMemoryTypeIndex(uint32_t memTypeBits, VkMemoryPropertyFlags memProps) const
{
    for (uint32_t i = 0; i < mMemoryProps.memoryTypeCount; i++)
    {
        if ((memTypeBits & (1 << i)) &&
            ((mMemoryProps.memoryTypes[i].propertyFlags & memProps) == memProps))
        {
            return i;
        }
    }

    printfError(__LINE__, __FILE__, "Required memory type not found");
    return -1;
}