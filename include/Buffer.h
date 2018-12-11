#ifndef BUFFER_H_
#define BUFFER_H_ 1

#include "vulkan\vulkan.h"

class Device;

class Buffer
{
public:
    Buffer(Device* device, size_t size, bool isHostVisible);

private:

    VkBuffer mBuffer;
    VkDeviceMemory mDeviceMemory;
    Device* mpDevice;

    size_t mSize;
};

#endif // BUFFER_H_