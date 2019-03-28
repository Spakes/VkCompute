#ifndef APPLICATION_H_
#define APPLICATION_H_ 1

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

#define APPLICATION_NAME "VkCompute";

class Device;

class Application
{
public:
    Application();
    ~Application();
    // TODO: Eventually have the Init() functionality in the constructor of Application class
    // and Run() like functionality available to anyone that includes Application.h
    void Run();

    bool mValidation;

private:
    // Initializes the instance, device and queues
    void Init();

    std::vector<VkExtensionProperties> mExtensionProps;
    std::vector<VkLayerProperties> mLayerProps;
    std::vector<const char*> mEnabledExtensions;
    std::vector<const char*> mEnabledLayers;

    VkInstance mInstance;
    VkDebugReportCallbackEXT mDebugReportCallback;
    VkPhysicalDevice mPhysicalDevice;
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSet mDescriptorSet;
    Device* mpDevice;
};

#endif // APPLICATION_H_