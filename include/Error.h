#include "vulkan/vulkan.h"

#include <iostream>

#define ERR_STR "[VkCompute] ERROR: "
#define VK_CHECK_ERR(e)                                                                           \
{                                                                                                 \
    VkResult res = e;                                                                             \
    if (res)                                                                                      \
    {                                                                                             \
    std::cout << ERR_STR << vkResultToCString(res) << " at line " << __LINE__ << " in " <<        \
    __FILE__ << "in function - " << #e << "\n";                                                   \
    }                                                                                             \
}

void printfError(int line, const char* file, const char* fmtStr, ...);

const char* vkResultToCString(VkResult errorCode);