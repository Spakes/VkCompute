#include "Application.h"
#include "Device.h"
#include "Error.h"

Application::Application() :
    mValidation(false),
    mpDevice(nullptr)
{

}

Application::~Application()
{
    if (nullptr != mpDevice)
    {
        delete(mpDevice);
    }
}

void Application::Run()
{
    Init();
}

/// Initializes the Vulkan instance, physical device, compute queue and a wrapping Device
/// object which gets stored in mpDevice.
void Application::Init()
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    mLayerProps.resize(count);
    vkEnumerateInstanceLayerProperties(&count, mLayerProps.data());

    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    mExtensionProps.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, mExtensionProps.data());

    if (mValidation)
    {
        for (int i = 0; i < mLayerProps.size(); i++)
        {
            if (strcmp("VK_LAYER_LUNARG_standard_validation", mLayerProps[i].layerName) == 0)
            {
                mEnabledLayers.push_back(mLayerProps[i].layerName);
                break;
            }
        }

        for (int i = 0; i < mExtensionProps.size(); i++)
        {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, mExtensionProps[i].extensionName) == 0)
            {
                mEnabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                break;
            }
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pApplicationName = APPLICATION_NAME;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = mEnabledLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = mEnabledLayers.data();
    instanceCreateInfo.enabledExtensionCount = mEnabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = mEnabledExtensions.data();

    VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);
    if (res != VK_SUCCESS)
    {
        std::cout << ERR_STR << "vkCreateInstance() failed: " << vkResultToCString(res) << ". Quitting...\n";
        exit(EXIT_FAILURE);
    }

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    res = vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());
    if (res != VK_SUCCESS)
    {
        std::cout << ERR_STR << "vkEnumeratePhysicalDevices() failed: " << vkResultToCString(res) << ". Quitting...\n";
        exit(EXIT_FAILURE);
    }

    if (physicalDevices.size() == 0)
    {
        std::cout << ERR_STR << "No Vulkan capable physical device found. Quitting...\n";
        exit(EXIT_FAILURE);
    }

    mPhysicalDevice = physicalDevices[0];
    mpDevice = new Device(mPhysicalDevice, mEnabledExtensions, mEnabledLayers);

    Buffer* buffer = mpDevice->CreateBuffer(1000, true);

    // The number of resources bound in each shader stage must match the number of
    // VkDescriptorSetLayoutBinding structs
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[] =
    {
        { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0 },
    };
    
    // First study writing compute shaders and then come back to descriptors

}