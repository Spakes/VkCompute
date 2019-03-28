#include "Application.h"
#include "Device.h"
#include "Error.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage,
    void*                                       pUserData)
{
    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

    return VK_FALSE;
}

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
#pragma region CreateInstance
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    mLayerProps.resize(count);
    vkEnumerateInstanceLayerProperties(&count, mLayerProps.data());

    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    mExtensionProps.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, mExtensionProps.data());

    if (mValidation)
    {
        bool foundLayer = false;
        for (int i = 0; i < mLayerProps.size(); i++)
        {
            if (strcmp("VK_LAYER_LUNARG_standard_validation", mLayerProps[i].layerName) == 0)
            {
                foundLayer = true;
                break;
            }
        }

        if (!foundLayer) {
            throw std::runtime_error("Layer VK_LAYER_LUNARG_standard_validation not supported\n");
        }
        mEnabledLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // Alright, we can use this layer.

        bool foundExtension = false;
        for (int i = 0; i < mExtensionProps.size(); i++)
        {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, mExtensionProps[i].extensionName) == 0)
            {
                foundExtension = true;
                break;
            }
        }

        if (!foundExtension) {
            throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
        }
        mEnabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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

    /*
        Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
        layer are actually printed.
        */
    if (mValidation) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFn;

        // We have to explicitly load this function.
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr) {
            throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        (vkCreateDebugReportCallbackEXT(mInstance, &createInfo, NULL, &mDebugReportCallback));
    }
#pragma endregion CreateInstance

#pragma region CreateDevice
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

    /**************************** Important Note ******************************
    * At this point we should check the limits of the physical device, like
    * memory limits or work group size limits etc. We have ignored this for
    * now. There are some checks in the constructor of Device class.
    **************************************************************************/
    mPhysicalDevice = physicalDevices[0];
    mpDevice = new Device(mPhysicalDevice, mEnabledExtensions, mEnabledLayers);
#pragma endregion CreateDevice

    Buffer* buffer = mpDevice->CreateBuffer(1000, true);

#pragma region CreateDescriptorSetLayout
    // The number of resources bound in each shader stage must match the number of
    // VkDescriptorSetLayoutBinding structs
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[] =
    {
        { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0 },
    };

    VkDescriptorSetLayoutBinding descriptorSetLayoutBind = {};
    descriptorSetLayoutBind.binding = 0;
    descriptorSetLayoutBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBind.descriptorCount = 1;
    descriptorSetLayoutBind.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    descriptorSetLayoutBind.pImmutableSamplers = 0;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout. 
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBind;

    // Create the descriptor set layout. 
    VK_CHECK_ERR(vkCreateDescriptorSetLayout(mpDevice->GetVkDevice(), &descriptorSetLayoutCreateInfo, NULL, &mDescriptorSetLayout));
#pragma endregion CreateDescriptorSetLayout

#pragma region CreateDescriptorSet
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    // create descriptor pool.
    VK_CHECK_ERR(vkCreateDescriptorPool(mpDevice->GetVkDevice(), &descriptorPoolCreateInfo, NULL, &mDescriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = mDescriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &mDescriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_ERR(vkAllocateDescriptorSets(mpDevice->GetVkDevice(), &descriptorSetAllocateInfo, &mDescriptorSet));
#pragma endregion CreateDescriptorSet
    
    // First study writing compute shaders and then come back to descriptors

}