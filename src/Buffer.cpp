#include "Buffer.h"
#include "Error.h"
#include "Device.h"

Buffer::Buffer(Device* device, size_t size, bool isHostVisible) :
    mpDevice(device),
    mSize(size)
{
    VkDevice vkDevice = mpDevice->GetVkDevice();

    // Create the buffer
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VK_CHECK_ERR(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mBuffer));

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements(vkDevice, mBuffer, &memRequirements);

    // Allocate the memory according to memRequirements
    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequirements.size;
    VkMemoryPropertyFlags memPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (isHostVisible)
    {
        memPropertyFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    memAllocInfo.memoryTypeIndex = mpDevice->GetMemoryTypeIndex(memRequirements.memoryTypeBits, memPropertyFlags);

    VK_CHECK_ERR(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &mDeviceMemory));

    VK_CHECK_ERR(vkBindBufferMemory(vkDevice, mBuffer, mDeviceMemory, 0));
}