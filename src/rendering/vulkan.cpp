#include "vulkan.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "image_loader.h"
#include "../util/math.h"

struct RenderPipeline
{
    VkDescriptorSetLayout descriptorSetLayout;

    VkPipelineLayout layout;
    VkPipeline pipeline;
};

namespace Vulkan
{
    ///INSTANCE///
    VkInstance instance;
    ///SURFACE///
    VkSurfaceKHR surface;

    ///DEVICE///
    VkPhysicalDevice physicalDevice;
    struct PhysicalDeviceInfo
    {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;

        std::vector<VkQueueFamilyProperties> queueFamilies;
    } physicalDeviceInfo;

    u32 defaultQueueFamilyIndex = 0;

    VkDevice device;
    VkQueue deviceQueue;

    ///CAMERA DATA///
    #define CAMERA_DATA_BINDING 0
    VkDescriptorBufferInfo cameraDataInfo;
    VkBuffer cameraDataBuffer;
    VkDeviceMemory cameraDataMemory;

    struct GlobalMatrices //should Renderer own this? Maybe
    {
        //glm::mat4 model; //transformation from model space to world space
        glm::mat4 view; //transformation from world space to view (camera) space
        glm::mat4 proj; //transformation from view space to screen space

        glm::vec3 camPos;
    } globalMatrices;

    ///LIGHTING DATA///
    #define LIGHTING_DATA_BINDING 1
    VkDescriptorBufferInfo lightingDataInfo;
    VkBuffer lightingDataBuffer;
    VkDeviceMemory lightingDataMemory;

    struct LightingData
    {
        glm::mat4 mainLightMat;
        glm::mat4 mainLightProjMat;
        glm::vec4 mainLightColor;
        glm::vec4 mainLightDirection;
        glm::vec4 ambientColor;
    } lightingData;

    ///PER-INSTANCE DATA///
    #define PER_INSTANCE_DATA_BINDING 2
    VkDescriptorBufferInfo perInstanceInfo;
    VkBuffer perInstanceBuffer;
    VkDeviceMemory perInstanceMemory;
    u32 perInstanceDynamicOffset;

    VkDeviceSize minUniformBufferOffsetAlignment;

    ///SHADER DATA///
    #define SHADER_DATA_BINDING 3
    VkDescriptorSet descriptorSets[MAX_MATERIAL_COUNT];
    VkBuffer shaderDataBuffer;
    VkDeviceMemory shaderDataMemory;

    ///SWAPCHAIN///
    VkSwapchainKHR swapChain;

    //have three images in the swapchain for possible triple buffering (yay)
    #define SWAPCHAIN_IMAGE_COUNT 3
    u32 currentSwapchainImageIndex = 0;
    VkFramebuffer swapChainFramebuffer[SWAPCHAIN_IMAGE_COUNT];
    VkImageView swapChainImageViews[SWAPCHAIN_IMAGE_COUNT];
    VkImage swapChainImages[SWAPCHAIN_IMAGE_COUNT];

    ///DEPTH TEXTURE///
    #define DEPTH_TEX_BINDING 15
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkSampler depthSampler;

    ///SHADOW MAPPING///
    #define SHAD0WMAP_BINDING 12
    #define SHADOW_RESOLUTION 4096
    #define SHADOW_AREA 25
    VkImage shadowImage;
    VkDeviceMemory shadowImageMemory;
    VkImageView shadowImageView;
    VkSampler shadowSampler;
    VkFramebuffer shadowFramebuffer;

    ///ENV MAP///
    #define CUBEMAP_DATA_BINDING 13
    VkImage noCubemapImage;
    VkDeviceMemory noCubemapImageMemory;
    VkImageView noCubemapImageView;
    VkSampler noCubemapSampler;

    VkImageView *cubemapImageViewPtr;
    VkSampler *cubemapSamplerPtr;

    ///MULTISAMPLING///
    VkImage msDepthImage;
    VkDeviceMemory msDepthImageMemory;
    VkImageView msDepthImageView;

    VkImage msImage;
    VkDeviceMemory msImageMemory;
    VkImageView msImageView;

    ///COLOR TEXTURE///
    #define COLOR_TEX_BINDING 14
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
    VkSampler colorSampler;

    ///POST PROCESSING///
    VkFramebuffer postProcessFramebuffer;

    VkImage noiseImage;
    VkDeviceMemory noiseImageMemory;
    VkImageView noiseImageView;
    VkSampler noiseSampler;

    ///RENDER PASSES///
    VkRenderPass shadowRenderPass;

    VkRenderPass forwardRenderPass;

    VkRenderPass bloomRenderPass;

    VkRenderPass gradingRenderPass;

    VkDescriptorSet shadowDescriptorSet;
    VkDescriptorSet gradingDescriptorSet;

    ///RENDER PIPELINES///
    VkPipelineLayout pipelineLayouts[MAX_SHADER_COUNT];
    VkPipeline pipelines[MAX_SHADER_COUNT];

    VkPipelineLayout shadowPipelineLayout;
    VkPipeline shadowPipeline;

    VkPipelineLayout colorGradingPipelineLayout;
    VkPipeline colorGradingPipeline;

    ///DESCRIPTORS///
    VkDescriptorPool descriptorPools[MAX_SHADER_COUNT];
    VkDescriptorSetLayout descriptorSetLayouts[MAX_SHADER_COUNT];
    DescriptorSetLayoutInfo descriptorSetLayoutInfos[MAX_SHADER_COUNT];

    VkDescriptorPool shadowPassDescriptorPool;
    VkDescriptorSetLayout shadowPassDescriptorSetLayout;
    DescriptorSetLayoutInfo shadowPassDescriptorSetLayoutInfo;

    VkDescriptorPool gradingDescriptorPool;
    VkDescriptorSetLayout gradingDescriptorSetLayout;
    DescriptorSetLayoutInfo gradingDescriptorSetLayoutInfo;

    ///COMMAND BUFFERS///
    VkCommandPool commandPool;
    VkCommandBuffer renderCommandBuffer;

    ///SEMAPHORES///
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    ///TEXTURES///
    VkImage textureImages[MAX_TEXTURE_COUNT];
    VkDeviceMemory textureMemory[MAX_TEXTURE_COUNT];
    VkImageView textureImageViews[MAX_TEXTURE_COUNT];
    VkSampler textureSamplers[MAX_TEXTURE_COUNT];

    #define SAMPLER_BINDING0 4
    #define SAMPLER_BINDING1 5
    #define SAMPLER_BINDING2 6
    #define SAMPLER_BINDING3 7
    #define SAMPLER_BINDING4 8
    #define SAMPLER_BINDING5 9
    #define SAMPLER_BINDING6 10
    #define SAMPLER_BINDING7 11

    ///VERTEX BUFFERS///
    u32 vertexCounts[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer vertexPositionBuffers[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer vertexTexcoord0Buffers[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer vertexNormalBuffers[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer vertexTangentBuffers[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer vertexColorBuffers[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory vertexPositionBufferMemory[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory vertexTexcoord0BufferMemory[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory vertexNormalBufferMemory[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory vertexTangentBufferMemory[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory vertexColorBufferMemory[MAX_VERTEX_BUFFER_COUNT];

    u32 indexCounts[MAX_VERTEX_BUFFER_COUNT];
    VkBuffer indexBuffers[MAX_VERTEX_BUFFER_COUNT];
    VkDeviceMemory indexBufferMemory[MAX_VERTEX_BUFFER_COUNT];

    ///DEBUG///
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif // NDEBUG
}

///INSTANCE///
const VkInstance Vulkan::get_instance()
{
    return instance;
}

///DEVICE///
void Vulkan::find_physical_device()
{
    //set up physical device to do the drawing
    //just get the first compatible device
    u32 deviceCount = 1;
    vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);

    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceInfo.properties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceInfo.memProperties);
    minUniformBufferOffsetAlignment = physicalDeviceInfo.properties.limits.minUniformBufferOffsetAlignment;

    //print out device name just for funs
    std::cout << "Found device: " << physicalDeviceInfo.properties.deviceName << std::endl;

    u32 version = physicalDeviceInfo.properties.apiVersion;
    u32 major = ((uint32_t)(version) >> 22);
    u32 minor = (((uint32_t)(version) >> 12) & 0x3ff);
    u32 patch = ((uint32_t)(version) & 0xfff);

    std::cout << "Device supports Vulkan v. " << major << "." << minor << "." << patch << std::endl;

    //queue families
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    physicalDeviceInfo.queueFamilies.reserve(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, physicalDeviceInfo.queueFamilies.data());

    //for now, only require graphics (and presentation)
    VkQueueFlags queueFamilyRequiredFeatures = VK_QUEUE_GRAPHICS_BIT;

    for (int i = 0; i < queueFamilyCount; i++)
    {
        VkQueueFamilyProperties q_family = physicalDeviceInfo.queueFamilies[i];
        if (q_family.queueFlags & queueFamilyRequiredFeatures == queueFamilyRequiredFeatures)
        {
            //check if presenting is supported
            VkBool32 presentingSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentingSupported);
            if (presentingSupported)
            {
                std::cout << "Using queue family #" << i << " (supports graphics and presenting, " << q_family.queueCount << " queues)\n\n";
                defaultQueueFamilyIndex = i;
                break;
            }

        }
    }
}

void Vulkan::create_logical_device()
{
    //create logical device with one queue
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.queueFamilyIndex = defaultQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    //empty feature struct because no special features are needed
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    //apparently some older implementations might need the validation layer to be specified also for the device
    //so I'm doing that just in case (although if that gets broken it's not too bad since it's just for debug purposes)
    if (enableValidationLayers)
    {
        const char *validationLayer = "VK_LAYER_KHRONOS_validation";
        deviceCreateInfo.enabledLayerCount = 1;
        deviceCreateInfo.ppEnabledLayerNames = &validationLayer;
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
    }
    deviceCreateInfo.enabledExtensionCount = 1;
    const char* swapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &swapchainExtension;

    vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    vkGetDeviceQueue(device, defaultQueueFamilyIndex, 0, &deviceQueue);
}
void Vulkan::free_logical_device()
{
    vkDestroyDevice(device, nullptr);
}

///DESCRIPTOR POOLS///
void Vulkan::create_descriptor_pool(VkDescriptorPool *pool, DescriptorSetLayoutInfo info)
{
    // Currently 3 built in uniform buffers (GlobalMatrices, LightingData, PerInstanceData)
    VkDescriptorPoolSize uniformBufferPoolSize;
    uniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    u32 uniformBufferCount = 0;
    uniformBufferCount += (info.flags & DSF_CAMERADATA) == DSF_CAMERADATA;
    uniformBufferCount += (info.flags & DSF_LIGHTINGDATA) == DSF_LIGHTINGDATA;
    uniformBufferCount += (info.flags & DSF_INSTANCEDATA) == DSF_INSTANCEDATA;
    uniformBufferPoolSize.descriptorCount = uniformBufferCount * MAX_MATERIAL_COUNT;

    // 0-8 samplers depending on the shader, plus built in shadowmap and env cubemap
    VkDescriptorPoolSize samplerPoolSize;
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    u32 textureCount = info.samplerCount;
    textureCount += (info.flags & DSF_SHADOWMAP) == DSF_SHADOWMAP;
    textureCount += (info.flags & DSF_CUBEMAP) == DSF_CUBEMAP;
    textureCount += (info.flags & DSF_COLOR_TEX) == DSF_COLOR_TEX;
    textureCount += (info.flags & DSF_DEPTH_TEX) == DSF_DEPTH_TEX;

    samplerPoolSize.descriptorCount = textureCount * MAX_MATERIAL_COUNT;

    // ShaderData is dynamic uniform buffer
    VkDescriptorPoolSize shaderDataPoolSize;
    shaderDataPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bool32 hasShaderData = (info.flags & DSF_SHADERDATA) == DSF_SHADERDATA;
    shaderDataPoolSize.descriptorCount = hasShaderData * MAX_MATERIAL_COUNT;

    // 3 types of descriptors
    VkDescriptorPoolSize poolSize[3];
    u32 poolSizeIndex = 0;

    if (uniformBufferCount > 0)
    {
        poolSize[poolSizeIndex] = uniformBufferPoolSize;
        poolSizeIndex++;
    }

    if (textureCount > 0)
    {
        poolSize[poolSizeIndex] = samplerPoolSize;
        poolSizeIndex++;
    }

    if (hasShaderData)
    {
        poolSize[poolSizeIndex] = shaderDataPoolSize;
        poolSizeIndex++;
    }

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = MAX_MATERIAL_COUNT;
    poolInfo.poolSizeCount = poolSizeIndex;
    poolInfo.pPoolSizes = poolSize;

    vkCreateDescriptorPool(device, &poolInfo, nullptr, pool);
}

void Vulkan::destroy_descriptor_pool(VkDescriptorPool *pool)
{
    vkDestroyDescriptorPool(device, *pool, nullptr);
}

void Vulkan::create_descriptor_set_layout(VkDescriptorSetLayout *layout, DescriptorSetLayoutInfo info)
{
    //descriptor set layout
    VkDescriptorSetLayoutBinding bindings[MAX_BINDING_COUNT]{};

    u32 bindingIndex = 0;

    //camera data
    if ((info.flags & DSF_CAMERADATA) == DSF_CAMERADATA)
    {
        bindings[bindingIndex].binding = CAMERA_DATA_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //lighting data
    if ((info.flags & DSF_LIGHTINGDATA) == DSF_LIGHTINGDATA)
    {
        bindings[bindingIndex].binding = LIGHTING_DATA_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //instance data
    if ((info.flags & DSF_INSTANCEDATA) == DSF_INSTANCEDATA)
    {
        bindings[bindingIndex].binding = PER_INSTANCE_DATA_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //material data
    if ((info.flags & DSF_SHADERDATA) == DSF_SHADERDATA)
    {
        bindings[bindingIndex].binding = SHADER_DATA_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    for (int i = 0; i < info.samplerCount; i++)
    {
        bindings[bindingIndex].binding = SAMPLER_BINDING0 + i;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //shadowmap
    if ((info.flags & DSF_SHADOWMAP) == DSF_SHADOWMAP)
    {
        bindings[bindingIndex].binding = SHAD0WMAP_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //env cubemap
    if ((info.flags & DSF_CUBEMAP) == DSF_CUBEMAP)
    {
        bindings[bindingIndex].binding = CUBEMAP_DATA_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //color texture
    if ((info.flags & DSF_COLOR_TEX) == DSF_COLOR_TEX)
    {
        bindings[bindingIndex].binding = COLOR_TEX_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    //depth texture
    if ((info.flags & DSF_DEPTH_TEX) == DSF_DEPTH_TEX)
    {
        bindings[bindingIndex].binding = DEPTH_TEX_BINDING;
        bindings[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingIndex].descriptorCount = 1;
        bindings[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[bindingIndex].pImmutableSamplers = nullptr;

        bindingIndex++;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = info.bindingCount;
    layoutInfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, layout);
}

void Vulkan::destroy_descriptor_set_layout(VkDescriptorSetLayout *layout)
{
    vkDestroyDescriptorSetLayout(device, *layout, nullptr);
}

///GLOBAL UNIFORM BUFFERS///
void Vulkan::create_camera_data_buffer()
{
    VkDeviceSize bufferSize = sizeof(GlobalMatrices);

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    vkCreateBuffer(device, &bufferInfo, nullptr, &cameraDataBuffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, cameraDataBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &cameraDataMemory);
    vkBindBufferMemory(device, cameraDataBuffer, cameraDataMemory, 0);

    // Store info
    cameraDataInfo.buffer = cameraDataBuffer;
    cameraDataInfo.offset = 0;
    cameraDataInfo.range = sizeof(GlobalMatrices);
}

void Vulkan::update_matrices(glm::vec3 camPos, Quaternion camRot) //placeholder
{
    //globalMatrices.model = glm::mat4(1.0f);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), camPos);

    glm::mat4 rotation;
    rotation[0][0] = 1 - 2 * camRot.y * camRot.y - 2 * camRot.z * camRot.z;
    rotation[0][1] = 2 * camRot.x * camRot.y + 2 * camRot.z * camRot.w;
    rotation[0][2] = 2 * camRot.x * camRot.z - 2 * camRot.y * camRot.w;
    rotation[0][3] = 0;
    rotation[1][0] = 2 * camRot.x * camRot.y - 2 * camRot.z * camRot.w;
    rotation[1][1] = 1 - 2 * camRot.x * camRot.x - 2 * camRot.z * camRot.z;
    rotation[1][2] = 2 * camRot.y * camRot.z + 2 * camRot.x * camRot.w;
    rotation[1][3] = 0;
    rotation[2][0] = 2 * camRot.x * camRot.z + 2 * camRot.y * camRot.w;
    rotation[2][1] = 2 * camRot.y * camRot.z - 2 * camRot.x * camRot.w;
    rotation[2][2] = 1 - 2 * camRot.x * camRot.x - 2 * camRot.y * camRot.y;
    rotation[2][3] = 0;
    rotation[3][0] = 0;
    rotation[3][1] = 0;
    rotation[3][2] = 0;
    rotation[3][3] = 1;

    globalMatrices.view = glm::inverse(translation * rotation);

    globalMatrices.proj = glm::perspective(glm::radians(41.12f), SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.01f, 100.0f);
    globalMatrices.proj[1][1] *= -1;
    globalMatrices.camPos = camPos;

    void* data;
    vkMapMemory(device, cameraDataMemory, 0, sizeof(GlobalMatrices), 0, &data);
        memcpy(data, &globalMatrices, sizeof(GlobalMatrices));
    vkUnmapMemory(device, cameraDataMemory);
}

void Vulkan::destroy_camera_data_buffer()
{
    vkDestroyBuffer(device, cameraDataBuffer, nullptr);
    vkFreeMemory(device, cameraDataMemory, nullptr);
}

///LIGHTING DATA///
void Vulkan::create_lighting_buffer()
{
    VkDeviceSize bufferSize = sizeof(LightingData);

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    vkCreateBuffer(device, &bufferInfo, nullptr, &lightingDataBuffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, lightingDataBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &lightingDataMemory);
    vkBindBufferMemory(device, lightingDataBuffer, lightingDataMemory, 0);

    //Store info
    lightingDataInfo.buffer = lightingDataBuffer;
    lightingDataInfo.offset = 0;
    lightingDataInfo.range = sizeof(LightingData);
}
void Vulkan::update_lighting(glm::vec3 mainLightPos, glm::vec3 mainLightDir, glm::vec4 mainLightColor)
{
    lightingData.mainLightMat = glm::lookAt(mainLightPos, mainLightPos + mainLightDir, glm::vec3(0.0f, 1.0f, 0.0f));
    lightingData.mainLightColor = mainLightColor;
    lightingData.mainLightProjMat = glm::ortho(-SHADOW_AREA/2.0f, SHADOW_AREA/2.0f, SHADOW_AREA/2.0f, -SHADOW_AREA/2.0f, -1024.0f, 1024.0f);
    lightingData.mainLightDirection = -glm::vec4(mainLightDir, 0.0);
    lightingData.ambientColor = {0.25,0.25,0.5,0.0};

    void* data;
    vkMapMemory(device, lightingDataMemory, 0, sizeof(LightingData), 0, &data);
        memcpy(data, &lightingData, sizeof(LightingData));
    vkUnmapMemory(device, lightingDataMemory);
}
void Vulkan::destroy_lighting_buffer()
{
    vkDestroyBuffer(device, lightingDataBuffer, nullptr);
    vkFreeMemory(device, lightingDataMemory, nullptr);
}

///PER-INSTANCE DATA///
void Vulkan::create_per_instance_buffer()
{
    u32 bufferSize = sizeof(glm::mat4) * 0x1000;
    create_buffer(&perInstanceBuffer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, perInstanceBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    r32 multiplier = std::ceil((r32)memRequirements.alignment / minUniformBufferOffsetAlignment);
    perInstanceDynamicOffset = multiplier * minUniformBufferOffsetAlignment;

    vkAllocateMemory(device, &allocInfo, nullptr, &perInstanceMemory);
    vkBindBufferMemory(device, perInstanceBuffer, perInstanceMemory, 0);

    // Store info
    perInstanceInfo.buffer = perInstanceBuffer;
    perInstanceInfo.offset = 0;
    perInstanceInfo.range = 256 * 4;
}
void Vulkan::destroy_per_instance_buffer()
{
    vkDestroyBuffer(device, perInstanceBuffer, nullptr);
    vkFreeMemory(device, perInstanceMemory, nullptr);
}

void Vulkan::set_transform_data(glm::mat4x4 *matrices, u32 length)
{
    for (int i = 0; i < length; i++)
    {
        void* data;
        vkMapMemory(device, perInstanceMemory, i * 256, sizeof(glm::mat4x4), 0, &data);
        memcpy(data, &matrices[i], sizeof(glm::mat4x4));
        vkUnmapMemory(device, perInstanceMemory);
    }
}

///SHADER DATA///
void Vulkan::create_shader_data_block(u32 materialIndex, ShaderDataBlock *dataBlock, u32 shaderIndex)
{
    //create descriptor set
    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = descriptorPools[shaderIndex];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayouts[shaderIndex];

    vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[materialIndex]);
}

void Vulkan::update_descriptor_set(VkDescriptorSet descriptorSet, DescriptorSetLayoutInfo info, s32 *textures, u32 shaderDataOffset)
{
    VkWriteDescriptorSet descriptorWrite[MAX_BINDING_COUNT];
    u32 bindingIndex = 0;

    if ((info.flags & DSF_CAMERADATA) == DSF_CAMERADATA)
    {
        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = CAMERA_DATA_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[bindingIndex].pBufferInfo = &cameraDataInfo;
        descriptorWrite[bindingIndex].pImageInfo = nullptr;
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
    }

    if ((info.flags & DSF_LIGHTINGDATA) == DSF_LIGHTINGDATA)
    {
        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = LIGHTING_DATA_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[bindingIndex].pBufferInfo = &lightingDataInfo;
        descriptorWrite[bindingIndex].pImageInfo = nullptr;
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
    }

    if ((info.flags & DSF_INSTANCEDATA) == DSF_INSTANCEDATA)
    {
        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = PER_INSTANCE_DATA_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite[bindingIndex].pBufferInfo = &perInstanceInfo;
        descriptorWrite[bindingIndex].pImageInfo = nullptr;
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
    }

    if ((info.flags & DSF_SHADERDATA) == DSF_SHADERDATA)
    {
        VkDescriptorBufferInfo shaderDataInfo;
        shaderDataInfo.buffer = shaderDataBuffer;
        shaderDataInfo.offset = shaderDataOffset;
        shaderDataInfo.range = MAX_SHADER_DATA_BLOCK_SIZE;

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = SHADER_DATA_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[bindingIndex].pBufferInfo = &shaderDataInfo;
        descriptorWrite[bindingIndex].pImageInfo = nullptr;
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
    }

    u32 textureCount = info.samplerCount;
    textureCount += (info.flags & DSF_SHADOWMAP) == DSF_SHADOWMAP;
    textureCount += (info.flags & DSF_CUBEMAP) == DSF_CUBEMAP;
    textureCount += (info.flags & DSF_COLOR_TEX) == DSF_COLOR_TEX;
    textureCount += (info.flags & DSF_DEPTH_TEX) == DSF_DEPTH_TEX;

    VkDescriptorImageInfo textureInfos[MAX_BINDING_COUNT];
    u32 textureIndex = 0;

    for (;textureIndex < info.samplerCount;)
    {
        textureInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfos[textureIndex].imageView = textureImageViews[textures[textureIndex]];
        textureInfos[textureIndex].sampler = textureSamplers[textures[textureIndex]];

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = SAMPLER_BINDING0 + textureIndex;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[bindingIndex].pBufferInfo = nullptr;
        descriptorWrite[bindingIndex].pImageInfo = &textureInfos[textureIndex];
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
        textureIndex++;
    }

    if ((info.flags & DSF_SHADOWMAP) == DSF_SHADOWMAP)
    {
        textureInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        textureInfos[textureIndex].imageView = shadowImageView;
        textureInfos[textureIndex].sampler = shadowSampler;

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = SHAD0WMAP_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[bindingIndex].pBufferInfo = nullptr;
        descriptorWrite[bindingIndex].pImageInfo = &textureInfos[textureIndex];
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
        textureIndex++;
    }

    if ((info.flags & DSF_CUBEMAP) == DSF_CUBEMAP)
    {
        textureInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfos[textureIndex].imageView = *cubemapImageViewPtr;
        textureInfos[textureIndex].sampler = *cubemapSamplerPtr;

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = CUBEMAP_DATA_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[bindingIndex].pBufferInfo = nullptr;
        descriptorWrite[bindingIndex].pImageInfo = &textureInfos[textureIndex];
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
        textureIndex++;
    }

    if ((info.flags & DSF_COLOR_TEX) == DSF_COLOR_TEX)
    {
        textureInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfos[textureIndex].imageView = colorImageView;
        textureInfos[textureIndex].sampler = colorSampler;

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = COLOR_TEX_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[bindingIndex].pBufferInfo = nullptr;
        descriptorWrite[bindingIndex].pImageInfo = &textureInfos[textureIndex];
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
        textureIndex++;
    }

    if ((info.flags & DSF_DEPTH_TEX) == DSF_DEPTH_TEX)
    {
        textureInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfos[textureIndex].imageView = depthImageView;
        textureInfos[textureIndex].sampler = depthSampler;

        descriptorWrite[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[bindingIndex].pNext = nullptr;
        descriptorWrite[bindingIndex].dstSet = descriptorSet;
        descriptorWrite[bindingIndex].dstBinding = DEPTH_TEX_BINDING;
        descriptorWrite[bindingIndex].dstArrayElement = 0;
        descriptorWrite[bindingIndex].descriptorCount = 1;
        descriptorWrite[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[bindingIndex].pBufferInfo = nullptr;
        descriptorWrite[bindingIndex].pImageInfo = &textureInfos[textureIndex];
        descriptorWrite[bindingIndex].pTexelBufferView = nullptr;

        bindingIndex++;
        textureIndex++;
    }

    vkUpdateDescriptorSets(device, info.bindingCount, descriptorWrite, 0, nullptr);
}
void Vulkan::update_shader_data_block(u32 materialIndex, u32 shaderIndex, ShaderDataBlock dataBlock, u32 texCount, s32 *textures)
{
    if (dataBlock.dataSize > 0 && dataBlock.data != nullptr)
    {
        void* temp;
        vkMapMemory(device, shaderDataMemory, materialIndex * MAX_SHADER_DATA_BLOCK_SIZE, dataBlock.dataSize, 0, &temp);
        memcpy(temp, dataBlock.data, dataBlock.dataSize);
        vkUnmapMemory(device, shaderDataMemory);
    }

    DescriptorSetLayoutInfo layoutInfo = descriptorSetLayoutInfos[shaderIndex];
    update_descriptor_set(descriptorSets[materialIndex], layoutInfo, textures, materialIndex * MAX_SHADER_DATA_BLOCK_SIZE);
}
void Vulkan::free_shader_data_block(u32 materialIndex, u32 shaderIndex)
{
    vkFreeDescriptorSets(device, descriptorPools[shaderIndex], 1, &descriptorSets[materialIndex]);
}
void Vulkan::create_shader_data_buffer()
{
    u32 bufferSize = MAX_SHADER_DATA_BLOCK_SIZE * MAX_MATERIAL_COUNT;
    create_buffer(&shaderDataBuffer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, shaderDataBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &shaderDataMemory);
    vkBindBufferMemory(device, shaderDataBuffer, shaderDataMemory, 0);
}
void Vulkan::destroy_shader_data_buffer()
{
    vkDestroyBuffer(device, shaderDataBuffer, nullptr);
    vkFreeMemory(device, shaderDataMemory, nullptr);
}

///SWAPCHAIN///
void Vulkan::create_swapchain_framebuffers()
{
    for (int i = 0; i < SWAPCHAIN_IMAGE_COUNT; i++)
    {
        VkFramebufferCreateInfo framebufferInfo;
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.flags = 0;
        framebufferInfo.renderPass = gradingRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &swapChainImageViews[i];
        framebufferInfo.width = SCREEN_WIDTH;
        framebufferInfo.height = SCREEN_HEIGHT;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffer[i]);
    }
}
void Vulkan::destroy_swapchain_framebuffers()
{
    for (int i = 0; i < SWAPCHAIN_IMAGE_COUNT; i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffer[i], nullptr);
    }
}
void Vulkan::create_swapchain_image_views()
{
    for (int i = 0; i < SWAPCHAIN_IMAGE_COUNT; i++)
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
    }
}
void Vulkan::destroy_swapchain_image_views()
{
    for (int i = 0; i < SWAPCHAIN_IMAGE_COUNT; i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }
}
void Vulkan::create_swapchain()
{
    //create swapchain
    //support for all of these things should technically be queried but I'm not doing that here
    //I'll return to this once I have a bigger picture or if problems happens
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = SWAPCHAIN_IMAGE_COUNT;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    VkExtent2D extent;
    extent.width = SCREEN_WIDTH;
    extent.height = SCREEN_HEIGHT;

    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
    swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //this is basically vsync and apparently guaranteed to be supported, so no need to query for this
    //might end up using triple buffering later (that needs to be queried)
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain);

    u32 swapChainImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
    std::cout << "swap chain image count " << swapChainImageCount << std::endl;
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages);
}

///Z_BUFFER///
void Vulkan::create_depth_texture()
{
    create_image(&depthImage, SCREEN_WIDTH, SCREEN_HEIGHT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, TEXTURE_2D, 1, VK_SAMPLE_COUNT_1_BIT);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&depthImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, depthImage, depthImageMemory, 0);

    create_image_view(&depthImageView, depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_TRUE;

    vkCreateSampler(device, &samplerInfo, nullptr, &depthSampler);
}
void Vulkan::destroy_depth_texture()
{
    vkDestroySampler(device, depthSampler, nullptr);
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
}

///SHADOW MAPPING///
void Vulkan::create_shadow_map()
{
    create_image(&shadowImage, SHADOW_RESOLUTION, SHADOW_RESOLUTION, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, shadowImage, &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&shadowImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, shadowImage, shadowImageMemory, 0);

    create_image_view(&shadowImageView, shadowImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(device, &samplerInfo, nullptr, &shadowSampler);

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = shadowRenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &shadowImageView;
    framebufferInfo.width = SHADOW_RESOLUTION;
    framebufferInfo.height = SHADOW_RESOLUTION;
    framebufferInfo.layers = 1;

    vkCreateFramebuffer(device, &framebufferInfo, nullptr, &shadowFramebuffer);
}
void Vulkan::destroy_shadow_map()
{
    vkDestroyFramebuffer(device, shadowFramebuffer, nullptr);
    vkDestroySampler(device, shadowSampler, nullptr);
    vkDestroyImageView(device, shadowImageView, nullptr);
    vkDestroyImage(device, shadowImage, nullptr);
    vkFreeMemory(device, shadowImageMemory, nullptr);
}

void Vulkan::create_shadow_pipeline()
{
    VkShaderModule vertShader;
    create_shader_module(&vertShader, "shaders/shadow.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    //////////////////////////////////////////////////////

    //vertex input
    VkVertexInputBindingDescription vertDescription;

    vertDescription.binding = 0;
    vertDescription.stride = sizeof(glm::vec3);
    vertDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription;

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    ////////////////////////////////////////////////////////

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SHADOW_RESOLUTION;
    viewport.height = (float)SHADOW_RESOLUTION;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {(s32)viewport.x, (s32)viewport.y};
    scissor.extent = {(u32)viewport.width, (u32)viewport.height};

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    ////////////////////////////////////////////////////////

    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;

    ////////////////////////////////////////////////////////

    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.pNext = nullptr;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = VkStencilOpState{};
    depthStencil.back = VkStencilOpState{};
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &shadowPassDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &shadowPipelineLayout);

    ////////////////////////////////////////////////////////

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = &vertShaderStageInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = nullptr;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = shadowPipelineLayout;
    pipelineInfo.renderPass = shadowRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shadowPipeline);

    vkDestroyShaderModule(device, vertShader, nullptr);
}
void Vulkan::destroy_shadow_pipeline()
{
    vkDestroyPipeline(device, shadowPipeline, nullptr);
    vkDestroyPipelineLayout(device, shadowPipelineLayout, nullptr);
    destroy_descriptor_set_layout(&shadowPassDescriptorSetLayout);
    destroy_descriptor_pool(&shadowPassDescriptorPool);
}

void Vulkan::create_placeholder_cubemap()
{
    // TODO: get rid of this horrible copypasta
    create_image(&noCubemapImage, 4, 4, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, TEXTURE_CUBEMAP, 1);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, noCubemapImage, &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&noCubemapImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, noCubemapImage, noCubemapImageMemory, 0);

    #define PLACEHOLDER_CUBEMAP_BYTES (4*4*4*6)
    VkDeviceSize imageBytes = PLACEHOLDER_CUBEMAP_BYTES;
    u8 pixelData[PLACEHOLDER_CUBEMAP_BYTES];
    for (int i = 0; i < imageBytes / 4; i++) // Generate pink / black 4x4 grid
    {
        bool evenCol = (i % 2);
        bool evenRow = (i / 4) % 2;
        pixelData[4*i] = (evenCol ^ evenRow) * 255;
        pixelData[4*i+1] = 0;
        pixelData[4*i+2] = (evenCol ^ evenRow) * 255;
        pixelData[4*i+3] = 255;
    }
    //copy pixels
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, imageBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkMemoryRequirements stagingMemRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &stagingMemRequirements);
    u32 stagingMemoryTypeIndex = get_device_memory_type_index(stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageBytes, 0, &data);
    memcpy(data, pixelData, imageBytes);
    vkUnmapMemory(device, stagingBufferMemory);

    //copy staging buffer to image
    {
        VkCommandBufferAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer tempCmds;
        vkAllocateCommandBuffers(device, &allocInfo, &tempCmds);

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(tempCmds, &beginInfo);

        //cmds
        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = noCubemapImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;

        vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 6;
        region.imageOffset = {0,0,0};
        region.imageExtent = {4,4,1};

        vkCmdCopyBufferToImage(tempCmds, stagingBuffer, noCubemapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        vkEndCommandBuffer(tempCmds);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &tempCmds;

        vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(deviceQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &tempCmds);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    //call this even if no mipmaps, because the texture needs to be converted to correct format
    //generate mipmaps
    {
        VkCommandBufferAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer tempCmds;
        vkAllocateCommandBuffers(device, &allocInfo, &tempCmds);

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(tempCmds, &beginInfo);

        //generate mipmaps
        VkImageMemoryBarrier mipBarrier;
        mipBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        mipBarrier.pNext = nullptr;
        //mipBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //mipBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        //mipBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //mipBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        mipBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        mipBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        mipBarrier.image = noCubemapImage;
        mipBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //mipBarrier.subresourceRange.baseMipLevel = 0;
        mipBarrier.subresourceRange.levelCount = 1;
        mipBarrier.subresourceRange.baseArrayLayer = 0;
        mipBarrier.subresourceRange.layerCount = 6;
        mipBarrier.subresourceRange.baseMipLevel = 0;
        mipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        mipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        mipBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipBarrier);

        vkEndCommandBuffer(tempCmds);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &tempCmds;

        vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(deviceQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &tempCmds);
    }

    create_image_view(&noCubemapImageView, noCubemapImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_CUBEMAP, 1);

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(device, &samplerInfo, nullptr, &noCubemapSampler);

    cubemapImageViewPtr = &noCubemapImageView;
    cubemapSamplerPtr = &noCubemapSampler;
}
void Vulkan::destroy_placeholder_cubemap()
{
    vkDestroySampler(device, noCubemapSampler, nullptr);
    vkDestroyImageView(device, noCubemapImageView, nullptr);
    vkDestroyImage(device, noCubemapImage, nullptr);
    vkFreeMemory(device, noCubemapImageMemory, nullptr);
}

void Vulkan::set_env_map(u32 textureIndex)
{
    cubemapImageViewPtr = &textureImageViews[textureIndex];
    cubemapSamplerPtr = &textureSamplers[textureIndex];
}

void Vulkan::create_multisampling_attachments()
{
    //color attachment
    create_image(&msImage, SCREEN_WIDTH, SCREEN_HEIGHT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, TEXTURE_2D, 1, VK_SAMPLE_COUNT_8_BIT);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, msImage, &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&msImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, msImage, msImageMemory, 0);

    create_image_view(&msImageView, msImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

    //depth attachment
    create_image(&msDepthImage, SCREEN_WIDTH, SCREEN_HEIGHT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, TEXTURE_2D, 1, VK_SAMPLE_COUNT_8_BIT);

    vkGetImageMemoryRequirements(device, msDepthImage, &memRequirements);

    memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&msDepthImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, msDepthImage, msDepthImageMemory, 0);

    create_image_view(&msDepthImageView, msDepthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
}
void Vulkan::destroy_multisampling_attachments()
{
    vkDestroyImageView(device, msImageView, nullptr);
    vkDestroyImage(device, msImage, nullptr);
    vkFreeMemory(device, msImageMemory, nullptr);

    vkDestroyImageView(device, msDepthImageView, nullptr);
    vkDestroyImage(device, msDepthImage, nullptr);
    vkFreeMemory(device, msDepthImageMemory, nullptr);
}

void Vulkan::create_color_texture()
{
    create_image(&colorImage, SCREEN_WIDTH, SCREEN_HEIGHT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, colorImage, &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&colorImageMemory, memRequirements.size, memoryTypeIndex);

    vkBindImageMemory(device, colorImage, colorImageMemory, 0);

    create_image_view(&colorImageView, colorImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_TRUE;

    vkCreateSampler(device, &samplerInfo, nullptr, &colorSampler);
}
void Vulkan::destroy_color_texture()
{
    vkDestroySampler(device, colorSampler, nullptr);
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
}

///POST PROCESSING///
void Vulkan::create_pp_framebuffer()
{
    VkImageView attachments[4] = {msImageView, msDepthImageView, colorImageView, depthImageView};

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = forwardRenderPass;
    framebufferInfo.attachmentCount = 4;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = SCREEN_WIDTH;
    framebufferInfo.height = SCREEN_HEIGHT;
    framebufferInfo.layers = 1;

    vkCreateFramebuffer(device, &framebufferInfo, nullptr, &postProcessFramebuffer);
}
void Vulkan::destroy_pp_framebuffer()
{
    vkDestroyFramebuffer(device, postProcessFramebuffer, nullptr);
}

void Vulkan::create_grading_pipeline()
{
    //create shader modules
    VkShaderModule vertShader;
    create_shader_module(&vertShader, "shaders/framebuffer_vert.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkShaderModule fragShader;
    create_shader_module(&fragShader, "shaders/grading_frag.spv");

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //////////////////////////////////////////////////////

    //vertex input
    VkVertexInputBindingDescription vertDescription;

    vertDescription.binding = 0;
    vertDescription.stride = sizeof(glm::vec3);
    vertDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription;

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    ////////////////////////////////////////////////////////

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SCREEN_WIDTH;
    viewport.height = (float)SCREEN_HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {(s32)viewport.x, (s32)viewport.y};
    scissor.extent = {(u32)viewport.width, (u32)viewport.height};

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    ////////////////////////////////////////////////////////

    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;

    ////////////////////////////////////////////////////////

    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &gradingDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &colorGradingPipelineLayout);

    ////////////////////////////////////////////////////////

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = colorGradingPipelineLayout;
    pipelineInfo.renderPass = gradingRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &colorGradingPipeline);

    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragShader, nullptr);
}
void Vulkan::destroy_grading_pipeline()
{
    vkDestroyPipeline(device, colorGradingPipeline, nullptr);
    vkDestroyPipelineLayout(device, colorGradingPipelineLayout, nullptr);
    destroy_descriptor_set_layout(&gradingDescriptorSetLayout);
    destroy_descriptor_pool(&gradingDescriptorPool);
}

///RENDER PASSES///
void Vulkan::create_shadow_render_pass()
{
    DescriptorSetLayoutInfo info;
    info.flags =(DescriptorSetLayoutFlags)(DSF_CAMERADATA | DSF_LIGHTINGDATA | DSF_INSTANCEDATA);
    info.samplerCount = 0;
    info.bindingCount = 3;
    shadowPassDescriptorSetLayoutInfo = info;

    create_descriptor_pool(&shadowPassDescriptorPool, info);
    create_descriptor_set_layout(&shadowPassDescriptorSetLayout, info);

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.attachmentCount = 1;

    VkAttachmentDescription depthDescription{};
    depthDescription.flags = 0;
    depthDescription.format = VK_FORMAT_D32_SFLOAT;
    depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    createInfo.pAttachments = &depthDescription;
    createInfo.subpassCount = 1;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 0;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.inputAttachmentCount = 0; //I don't think texture sampling counts as input attachments here. We'll see
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 0;
    subpassDescription.pColorAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;

    createInfo.pSubpasses = &subpassDescription;
    createInfo.dependencyCount = 2;

    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    createInfo.pDependencies = dependencies;

    vkCreateRenderPass(device, &createInfo, nullptr, &shadowRenderPass);

    //create descriptor set
    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = shadowPassDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &shadowPassDescriptorSetLayout;

    vkAllocateDescriptorSets(device, &allocInfo, &shadowDescriptorSet);
    update_shadow_descriptor_set();
}

void Vulkan::create_forward_render_pass()
{
    //the 2d render pass is used to draw 2d things like the background, tiles or sprites
    //to a framebuffer that will later be used for post processing operations
    //in other words, 2d things that are part of the game world.
    VkRenderPassCreateInfo2 createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    createInfo.pNext = nullptr;
    createInfo.attachmentCount = 4;

    VkAttachmentDescription2 attachmentDescription{};
    attachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    attachmentDescription.pNext = nullptr;
    attachmentDescription.flags = 0;
    attachmentDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachmentDescription.samples = VK_SAMPLE_COUNT_8_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //temporary: clear before drawing. Change later to VK_ATTACHMENT_LOAD_OP_LOAD
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //this is also temporary for testing purposes
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription2 depthDescription{};
    depthDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    depthDescription.pNext = nullptr;
    depthDescription.flags = 0;
    depthDescription.format = VK_FORMAT_D32_SFLOAT;
    depthDescription.samples = VK_SAMPLE_COUNT_8_BIT;
    depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription2 colorAttachmentResolve{};
    colorAttachmentResolve.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    colorAttachmentResolve.pNext = nullptr;
    colorAttachmentResolve.flags = 0;
    colorAttachmentResolve.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription2 depthAttachmentResolve{};
    depthAttachmentResolve.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    depthAttachmentResolve.pNext = nullptr;
    depthAttachmentResolve.flags = 0;
    depthAttachmentResolve.format = VK_FORMAT_D32_SFLOAT;
    depthAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription2 attachments[4] = {attachmentDescription, depthDescription, colorAttachmentResolve, depthAttachmentResolve};

    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;

    VkAttachmentReference2 colorAttachmentReference;
    colorAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    colorAttachmentReference.pNext = nullptr;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkAttachmentReference2 depthAttachmentReference;
    depthAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    depthAttachmentReference.pNext = nullptr;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    VkAttachmentReference2 colorAttachmentResolveRef;
    colorAttachmentResolveRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    colorAttachmentResolveRef.pNext = nullptr;
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentResolveRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkAttachmentReference2 depthAttachmentResolveRef;
    depthAttachmentResolveRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    depthAttachmentResolveRef.pNext = nullptr;
    depthAttachmentResolveRef.attachment = 3;
    depthAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentResolveRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubpassDescriptionDepthStencilResolve depthResolve;
    depthResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
    depthResolve.pNext = nullptr;
    depthResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    depthResolve.stencilResolveMode = VK_RESOLVE_MODE_NONE;
    depthResolve.pDepthStencilResolveAttachment = &depthAttachmentResolveRef;

    VkSubpassDescription2 subpassDescription{};
    subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    subpassDescription.pNext = &depthResolve;
    subpassDescription.flags = 0;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.viewMask = 0;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pResolveAttachments = &colorAttachmentResolveRef;
    subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;

    createInfo.pSubpasses = &subpassDescription;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies = nullptr;
    createInfo.correlatedViewMaskCount = 0;
    createInfo.pCorrelatedViewMasks = nullptr;

    vkCreateRenderPass2(device, &createInfo, nullptr, &forwardRenderPass);
}
void Vulkan::create_grading_render_pass()
{
    DescriptorSetLayoutInfo info;
    info.flags =(DescriptorSetLayoutFlags)(DSF_CAMERADATA | DSF_COLOR_TEX | DSF_DEPTH_TEX);
    info.samplerCount = 0;
    info.bindingCount = 3;
    gradingDescriptorSetLayoutInfo = info;

    create_descriptor_pool(&gradingDescriptorPool, info);
    create_descriptor_set_layout(&gradingDescriptorSetLayout, info);

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.attachmentCount = 1;

    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.flags = 0;
    attachmentDescription.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    createInfo.pAttachments = &attachmentDescription;
    createInfo.subpassCount = 1;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pDepthStencilAttachment = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;

    createInfo.pSubpasses = &subpassDescription;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies = nullptr;

    vkCreateRenderPass(device, &createInfo, nullptr, &gradingRenderPass);

    //create descriptor set
    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = gradingDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &gradingDescriptorSetLayout;

    vkAllocateDescriptorSets(device, &allocInfo, &gradingDescriptorSet);
    //update_post_process_descriptor_set();
}
void Vulkan::create_render_passes()
{
    create_forward_render_pass();
    create_shadow_render_pass();
    create_grading_render_pass();
}
void Vulkan::destroy_render_passes()
{
    vkDestroyRenderPass(device, shadowRenderPass, nullptr);
    vkDestroyRenderPass(device, forwardRenderPass, nullptr);
    vkDestroyRenderPass(device, gradingRenderPass, nullptr);
}

///SHADER MODULES///
void Vulkan::create_shader_module(VkShaderModule *module, const char* fname)
{
    std::ifstream file(fname, std::ios::ate | std::ios::binary);
    u32 fileSize = file.tellg();
    char buffer[0x4000];
    file.seekg(0);
    file.read(buffer, fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = fileSize;
    createInfo.pCode = (u32*)buffer;

    vkCreateShaderModule(device, &createInfo, nullptr, module);
}

///RENDER PIPELINES///
void Vulkan::create_render_pipeline(u32 index, const char *vert, const char *frag)
{
    //create shader modules
    VkShaderModule vertShader;
    create_shader_module(&vertShader, vert);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkShaderModule fragShader;
    create_shader_module(&fragShader, frag);

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //////////////////////////////////////////////////////

    //vertex input
    VkVertexInputBindingDescription vertDescriptions[5];

    vertDescriptions[0].binding = 0;
    vertDescriptions[0].stride = sizeof(glm::vec3);
    vertDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertDescriptions[1].binding = 1;
    vertDescriptions[1].stride = sizeof(glm::vec2);
    vertDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertDescriptions[2].binding = 2;
    vertDescriptions[2].stride = sizeof(glm::vec3);
    vertDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertDescriptions[3].binding = 3;
    vertDescriptions[3].stride = sizeof(glm::vec4);
    vertDescriptions[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertDescriptions[4].binding = 4;
    vertDescriptions[4].stride = sizeof(glm::vec4);
    vertDescriptions[4].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[5];

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 1;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 0;

    attributeDescriptions[2].binding = 2;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = 0;

    attributeDescriptions[3].binding = 3;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = 0;

    attributeDescriptions[4].binding = 4;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[4].offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 5;
    vertexInputInfo.pVertexBindingDescriptions = vertDescriptions;
    vertexInputInfo.vertexAttributeDescriptionCount = 5;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    ////////////////////////////////////////////////////////

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SCREEN_WIDTH;
    viewport.height = (float)SCREEN_HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {(s32)viewport.x, (s32)viewport.y};
    scissor.extent = {(u32)viewport.width, (u32)viewport.height};

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    ////////////////////////////////////////////////////////

    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;

    ////////////////////////////////////////////////////////

    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.pNext = nullptr;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = VkStencilOpState{};
    depthStencil.back = VkStencilOpState{};
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    ////////////////////////////////////////////////////////

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[index];
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts[index]);

    ////////////////////////////////////////////////////////

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayouts[index];
    pipelineInfo.renderPass = forwardRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines[index]);

    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragShader, nullptr);
}

void Vulkan::create_shader(u32 shaderIndex, Shader *shader, const char *vert, const char *frag)
{
    DescriptorSetLayoutInfo info;
    info.flags = (DescriptorSetLayoutFlags)(DSF_CAMERADATA | DSF_LIGHTINGDATA | DSF_INSTANCEDATA | DSF_SHADERDATA | DSF_SHADOWMAP | DSF_CUBEMAP);
    info.samplerCount = shader->samplerCount;
    info.bindingCount = 6 + info.samplerCount;

    descriptorSetLayoutInfos[shaderIndex] = info;

    VkDescriptorPool *pool = &descriptorPools[shaderIndex];
    create_descriptor_pool(pool, info);
    VkDescriptorSetLayout *layout = &descriptorSetLayouts[shaderIndex];
    create_descriptor_set_layout(layout, info);
    create_render_pipeline(shaderIndex, vert, frag);
}
void Vulkan::destroy_shader(u32 shaderIndex)
{
    vkDestroyPipelineLayout(device, pipelineLayouts[shaderIndex], nullptr);
    vkDestroyPipeline(device, pipelines[shaderIndex], nullptr);
    destroy_descriptor_set_layout(&descriptorSetLayouts[shaderIndex]);
    destroy_descriptor_pool(&descriptorPools[shaderIndex]);
}

///COMMAND BUFFERS///
void Vulkan::create_command_pool()
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;
    poolInfo.queueFamilyIndex = 0;

    vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
}

void Vulkan::begin_rendering()
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocInfo, &renderCommandBuffer);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(renderCommandBuffer, &beginInfo);
}

void Vulkan::begin_shadow_pass()
{
    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = shadowRenderPass;
    renderPassInfo.framebuffer = shadowFramebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {SHADOW_RESOLUTION,SHADOW_RESOLUTION};

    VkClearValue clearColor = {1.0f, 0};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(renderCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline);

}

void Vulkan::set_shadow_instance_data(u32 callIndex)
{
    u32 dynamicOffset = perInstanceDynamicOffset * callIndex;
    vkCmdBindDescriptorSets(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 0, 1, &shadowDescriptorSet, 1, &dynamicOffset);
}
void Vulkan::begin_forward_render_pass()
{
    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = forwardRenderPass;
    renderPassInfo.framebuffer = postProcessFramebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {SCREEN_WIDTH,SCREEN_HEIGHT};

    VkClearValue clearColors[2] = {{0,0,0,1}, {1.0f, 0}};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearColors;

    //begin render pass! yeyeyey
    vkCmdBeginRenderPass(renderCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void Vulkan::end_render_pass()
{
    vkCmdEndRenderPass(renderCommandBuffer);
}

void Vulkan::bind_shader(u32 shaderIndex)
{
    vkCmdBindPipeline(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[shaderIndex]);
}
void Vulkan::bind_shader_data_block(u32 shaderIndex, u32 materialIndex, u32 callIndex)
{
    u32 dynamicOffset = perInstanceDynamicOffset * callIndex;

    vkCmdBindDescriptorSets(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[shaderIndex], 0, 1, &descriptorSets[materialIndex], 1, &dynamicOffset);
}

void Vulkan::bind_vertex_buffer(u32 meshIndex, VertexAttribFlags attribs)
{
    VkDeviceSize offset = 0;

    if (attribs & VERTEX_POSITION_BIT)
        vkCmdBindVertexBuffers(renderCommandBuffer, 0, 1, &vertexPositionBuffers[meshIndex], &offset);
    if (attribs & VERTEX_TEXCOORD_0_BIT)
        vkCmdBindVertexBuffers(renderCommandBuffer, 1, 1, &vertexTexcoord0Buffers[meshIndex], &offset);
    if (attribs & VERTEX_NORMAL_BIT)
        vkCmdBindVertexBuffers(renderCommandBuffer, 2, 1, &vertexNormalBuffers[meshIndex], &offset);
    if (attribs & VERTEX_TANGENT_BIT)
        vkCmdBindVertexBuffers(renderCommandBuffer, 3, 1, &vertexTangentBuffers[meshIndex], &offset);
    if (attribs & VERTEX_COLOR_BIT)
        vkCmdBindVertexBuffers(renderCommandBuffer, 4, 1, &vertexColorBuffers[meshIndex], &offset);

    vkCmdBindIndexBuffer(renderCommandBuffer, indexBuffers[meshIndex], 0, VK_INDEX_TYPE_UINT16);
}

void Vulkan::draw_elements(u32 count, u32 firstIndex, s32 vertexOffset)
{
    vkCmdDrawIndexed(renderCommandBuffer, count, 1, firstIndex, vertexOffset, 0);
}

void Vulkan::update_post_process_descriptor_set()
{
    update_descriptor_set(gradingDescriptorSet, gradingDescriptorSetLayoutInfo);
}

void Vulkan::update_shadow_descriptor_set()
{
    update_descriptor_set(shadowDescriptorSet, shadowPassDescriptorSetLayoutInfo);
}

void Vulkan::render_post_process(u32 meshIndex)
{
    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = gradingRenderPass;
    renderPassInfo.framebuffer = swapChainFramebuffer[currentSwapchainImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {SCREEN_WIDTH,SCREEN_HEIGHT};

    currentSwapchainImageIndex++;
    if (currentSwapchainImageIndex >= SWAPCHAIN_IMAGE_COUNT)
        currentSwapchainImageIndex = 0;

    VkClearValue clearColor = {0,0,0,1};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(renderCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorGradingPipeline);

    vkCmdBindDescriptorSets(renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorGradingPipelineLayout, 0, 1, &gradingDescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderCommandBuffer, 0, 1, &vertexPositionBuffers[meshIndex], &offset);
    vkCmdBindIndexBuffer(renderCommandBuffer, indexBuffers[meshIndex], 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(renderCommandBuffer, 6, 1, 0, 0, 0);
}

void Vulkan::stop_rendering()
{
    vkEndCommandBuffer(renderCommandBuffer);
}


///SEMAPHORES///
void Vulkan::create_semaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
}

void Vulkan::destroy_semaphores()
{
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
}

///TEXTURES///
void Vulkan::allocate_texture_memory(u32 index)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, textureImages[index], &memRequirements);

    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    allocate_memory(&textureMemory[index], memRequirements.size, memoryTypeIndex);
}
void Vulkan::free_texture_memory(u32 index)
{
    vkFreeMemory(device, textureMemory[index], nullptr);
}
void Vulkan::create_texture_image(u32 index, TextureType type, int width, int height, VkFormat format, int mipCount)
{
    create_image(&textureImages[index], width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, type, mipCount);

    allocate_texture_memory(index);
    vkBindImageMemory(device, textureImages[index], textureMemory[index], 0);

    std::cout << "Texture " << index << " created!\n";
}
void Vulkan::create_texture_sampler(u32 index, int mipCount, TextureFilter filter)
{
    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = (VkFilter)filter;
    samplerInfo.minFilter = (VkFilter)filter;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = mipCount;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(device, &samplerInfo, nullptr, &textureSamplers[index]);
}
void Vulkan::copy_staging_buffer_to_texture(u32 index, VkBuffer stagingBuffer, u32 width, u32 height, int layerCount, int mipCount)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer tempCmds;
    vkAllocateCommandBuffers(device, &allocInfo, &tempCmds);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(tempCmds, &beginInfo);

    //cmds
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImages[index];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;
    region.imageOffset = {0,0,0};
    region.imageExtent = {width,height,1};

    vkCmdCopyBufferToImage(tempCmds, stagingBuffer, textureImages[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkEndCommandBuffer(tempCmds);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tempCmds;

    vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &tempCmds);
}
void Vulkan::generate_mipmaps(u32 index, int width, int height, int layerCount, int mipCount)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer tempCmds;
    vkAllocateCommandBuffers(device, &allocInfo, &tempCmds);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(tempCmds, &beginInfo);

    //generate mipmaps
    VkImageMemoryBarrier mipBarrier;
    mipBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    mipBarrier.pNext = nullptr;
    //mipBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //mipBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //mipBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //mipBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mipBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mipBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mipBarrier.image = textureImages[index];
    mipBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //mipBarrier.subresourceRange.baseMipLevel = 0;
    mipBarrier.subresourceRange.levelCount = 1;
    mipBarrier.subresourceRange.baseArrayLayer = 0;
    mipBarrier.subresourceRange.layerCount = layerCount;

    s32 mipWidth = width;
    s32 mipHeight = height;

    for (int i = 1; i < mipCount; i++)
    {
        mipBarrier.subresourceRange.baseMipLevel = i - 1;
        mipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mipBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        mipBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipBarrier);

        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = layerCount;
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

        vkCmdBlitImage(tempCmds, textureImages[index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textureImages[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        mipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        mipBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        mipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipBarrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    mipBarrier.subresourceRange.baseMipLevel = mipCount - 1;
    mipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    mipBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(tempCmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipBarrier);

    vkEndCommandBuffer(tempCmds);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tempCmds;

    vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &tempCmds);
}
void Vulkan::create_texture(u32 textureIndex, Image **image, Texture *texture, TextureType type, TextureFilter filter, bool generateMips)
{
    int layerCount = 1;
    if (type == TEXTURE_CUBEMAP)
        layerCount = 6;

    VkFormat format;
    VkDeviceSize imageBytes;

    int width = image[0]->width;
    int height = image[0]->height;

    switch(image[0]->type)
    {
        case IMAGE_SRGB:
            format = VK_FORMAT_R8G8B8A8_SRGB;
            imageBytes = width * height * 4;
            break;
        case IMAGE_NORMAL:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            imageBytes = width * height * 4;
           break;
        default:
            format = VK_FORMAT_UNDEFINED;
            imageBytes = 0;
            break;
    }

    u32 mipCount = std::floor(std::log2(MAX(width, height))) + 1;
    if (!generateMips)
        mipCount = 1;

    create_texture_image(textureIndex, type, width, height, format, mipCount);

    //copy pixels
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, imageBytes * layerCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);
    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    allocate_memory(&stagingBufferMemory, imageBytes * layerCount, memoryTypeIndex);

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageBytes * layerCount, 0, &data);
    for (int i = 0; i < layerCount; i++)
    {
        memcpy((char*)data + (imageBytes * i), image[i]->pixels, imageBytes);
    }
    vkUnmapMemory(device, stagingBufferMemory);

    copy_staging_buffer_to_texture(textureIndex, stagingBuffer, width, height, layerCount, mipCount);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    //call this even if no mipmaps, because the texture needs to be converted to correct format
    generate_mipmaps(textureIndex, width, height, layerCount, mipCount);

    create_image_view(&textureImageViews[textureIndex], textureImages[textureIndex], format, VK_IMAGE_ASPECT_COLOR_BIT, type, mipCount);
    create_texture_sampler(textureIndex, mipCount, filter);

    texture->type = type;
}

void Vulkan::destroy_texture(u32 textureIndex)
{
    vkDestroyImage(device, textureImages[textureIndex], nullptr);
    vkDestroyImageView(device, textureImageViews[textureIndex], nullptr);
    vkDestroySampler(device, textureSamplers[textureIndex], nullptr);
    free_texture_memory(textureIndex);
}

///VERTEX BUFFERS///
void Vulkan::create_index_buffer(u8 index, MeshData *meshData)
{
    u32 bufferSize = sizeof(Triangle) * meshData->triangleCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, meshData->triangles, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&indexBuffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    allocate_buffer_memory(&indexBufferMemory[index], indexBuffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, indexBuffers[index], indexBufferMemory[index], 0);

    copy_buffer(stagingBuffer, indexBuffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    indexCounts[index] = meshData->triangleCount * 3;
}

void Vulkan::create_vertex_position_buffer(u8 index, u16 vertexCount, glm::vec3 *pos)
{
    u32 bufferSize = sizeof(glm::vec3) * vertexCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pos, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&vertexPositionBuffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocate_buffer_memory(&vertexPositionBufferMemory[index], vertexPositionBuffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, vertexPositionBuffers[index], vertexPositionBufferMemory[index], 0);

    copy_buffer(stagingBuffer, vertexPositionBuffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void Vulkan::create_vertex_texcoord0_buffer(u8 index, u16 vertexCount, glm::vec2 *uv)
{
    u32 bufferSize = sizeof(glm::vec2) * vertexCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, uv, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&vertexTexcoord0Buffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocate_buffer_memory(&vertexTexcoord0BufferMemory[index], vertexTexcoord0Buffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, vertexTexcoord0Buffers[index], vertexTexcoord0BufferMemory[index], 0);

    copy_buffer(stagingBuffer, vertexTexcoord0Buffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void Vulkan::create_vertex_normal_buffer(u8 index, u16 vertexCount, glm::vec3 *nrm)
{
    u32 bufferSize = sizeof(glm::vec3) * vertexCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, nrm, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&vertexNormalBuffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocate_buffer_memory(&vertexNormalBufferMemory[index], vertexNormalBuffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, vertexNormalBuffers[index], vertexNormalBufferMemory[index], 0);

    copy_buffer(stagingBuffer, vertexNormalBuffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void Vulkan::create_vertex_tangent_buffer(u8 index, u16 vertexCount, glm::vec4 *tan)
{
    u32 bufferSize = sizeof(glm::vec4) * vertexCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, tan, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&vertexTangentBuffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocate_buffer_memory(&vertexTangentBufferMemory[index], vertexTangentBuffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, vertexTangentBuffers[index],vertexTangentBufferMemory[index], 0);

    copy_buffer(stagingBuffer, vertexTangentBuffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
void Vulkan::create_vertex_color_buffer(u8 index, u16 vertexCount, glm::vec4 *clr)
{
    u32 bufferSize = sizeof(glm::vec4) * vertexCount;

    //staging bufffaaa
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    allocate_buffer_memory(&stagingBufferMemory, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, clr, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    create_buffer(&vertexColorBuffers[index], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    allocate_buffer_memory(&vertexColorBufferMemory[index], vertexColorBuffers[index], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, vertexColorBuffers[index],vertexColorBufferMemory[index], 0);

    copy_buffer(stagingBuffer, vertexColorBuffers[index], bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Vulkan::create_vertex_buffer(u32 meshIndex, MeshData *meshData)
{
    u32 vertexCount = meshData->vertexCount;
    create_vertex_position_buffer(meshIndex, vertexCount, meshData->position);
    create_vertex_texcoord0_buffer(meshIndex, vertexCount, meshData->texcoord0);
    create_vertex_normal_buffer(meshIndex, vertexCount, meshData->normal);
    create_vertex_tangent_buffer(meshIndex, vertexCount, meshData->tangent);
    create_vertex_color_buffer(meshIndex, vertexCount, meshData->color);

    create_index_buffer(meshIndex, meshData);

    vertexCounts[meshIndex] = vertexCount;
}
void Vulkan::destroy_vertex_buffer(u32 meshIndex)
{
    vkFreeMemory(device, indexBufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, indexBuffers[meshIndex], nullptr);
    vkFreeMemory(device, vertexPositionBufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, vertexPositionBuffers[meshIndex], nullptr);
    vkFreeMemory(device, vertexTexcoord0BufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, vertexTexcoord0Buffers[meshIndex], nullptr);
    vkFreeMemory(device, vertexNormalBufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, vertexNormalBuffers[meshIndex], nullptr);
    vkFreeMemory(device, vertexTangentBufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, vertexTangentBuffers[meshIndex], nullptr);
    vkFreeMemory(device, vertexColorBufferMemory[meshIndex], nullptr);
    vkDestroyBuffer(device, vertexColorBuffers[meshIndex], nullptr);
}

u32 Vulkan::get_vertex_count(u32 meshIndex)
{
    return vertexCounts[meshIndex];
}
u32 Vulkan::get_index_count(u32 meshIndex)
{
    return indexCounts[meshIndex];
}

///MAIN///
void Vulkan::init(u32 extensionCount, const char** extensionNames, void (*surfaceCallback)(VkSurfaceKHR*))
{
    //create vulkan instance:
    std::cout << "Extensions for vulkan instance required by the application:\n";
    for(int i = 0; i < extensionCount; i++)
    {
        std::cout << extensionNames[i] << std::endl;
    }
    std::cout << std::endl;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Nekro Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.pApplicationInfo = &appInfo;

    u32 supportedLayerCount;
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);

    VkLayerProperties layerProperties[32];
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, layerProperties);

    for (int i = 0; i < supportedLayerCount; i++)
    {
        u32 version = layerProperties[i].specVersion;
        u32 major = ((uint32_t)(version) >> 22);
        u32 minor = (((uint32_t)(version) >> 12) & 0x3ff);
        u32 patch = ((uint32_t)(version) & 0xfff);

        std::cout << layerProperties[i].layerName << ", specVersion = " << major << "." << minor << "." << patch << std::endl;
        std::cout << "     " << layerProperties[i].description << std::endl;
    }

    //debug: validation layers
    if (enableValidationLayers)
    {
        const char *validationLayer = "VK_LAYER_KHRONOS_validation";
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayer;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensionNames;

    vkCreateInstance(&createInfo, nullptr, &instance);

    std::cout << "Created instance with api version 1.2.0\n";

    //create surface
    (*surfaceCallback)(&surface);

    //device
    find_physical_device();
    create_logical_device();
    //uniform buffers
    create_camera_data_buffer();
    create_lighting_buffer();
    create_per_instance_buffer();
    create_shader_data_buffer();
    //swapchain
    create_swapchain();
    //image views
    create_swapchain_image_views();
    //z-buffer
    create_depth_texture();
    create_multisampling_attachments();
    //render passes
    create_render_passes();
    //color texture
    create_color_texture();
    //shadow map
    create_shadow_map();
    create_shadow_pipeline();
    //post processing
    create_grading_pipeline();
    //framebuffers
    create_pp_framebuffer();
    create_swapchain_framebuffers();
    //command pool
    create_command_pool();
    //cubemap
    create_placeholder_cubemap();

    //semaphores
    create_semaphores();
}
void Vulkan::draw_frame(glm::vec3 camPos, Quaternion camRot) //placeholder function
{
    update_matrices(camPos, camRot);

    u32 imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(deviceQueue, &presentInfo);
    vkQueueWaitIdle(deviceQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &renderCommandBuffer);
}

void Vulkan::free()
{
    //wait till all operations have completed
    vkDeviceWaitIdle(device);

    //semaphores
    destroy_semaphores();

    //command pool
    vkDestroyCommandPool(device, commandPool, nullptr);

    //render passes
    destroy_render_passes();

    //framebuffers
    destroy_pp_framebuffer();
    destroy_swapchain_framebuffers();

    //post processing
    destroy_grading_pipeline();

    //cubemap
    destroy_placeholder_cubemap();

    //shadow map
    destroy_shadow_pipeline();
    destroy_shadow_map();
    //color texture
    destroy_color_texture();
    //z-buffer
    destroy_depth_texture();
    destroy_multisampling_attachments();
    //image views
    destroy_swapchain_image_views();
    //swapchain
    vkDestroySwapchainKHR(device, swapChain, nullptr);

    //uniform buffers
    destroy_camera_data_buffer();
    destroy_lighting_buffer();
    destroy_per_instance_buffer();
    destroy_shader_data_buffer();

    free_logical_device();
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

///UTIL///
void Vulkan::create_buffer(VkBuffer *pBuffer, VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    vkCreateBuffer(device, &bufferInfo, nullptr, pBuffer);
}
void Vulkan::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer tempCmds;
    vkAllocateCommandBuffers(device, &allocInfo, &tempCmds);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(tempCmds, &beginInfo);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempCmds, src, dst, 1, &copyRegion);

    vkEndCommandBuffer(tempCmds);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tempCmds;

    vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &tempCmds);

}
void Vulkan::free_buffer(VkBuffer *pBuffer)
{
    vkDestroyBuffer(device, *pBuffer, nullptr);
}
void Vulkan::allocate_memory(VkDeviceMemory *pMemory, VkDeviceSize size, u32 memoryTypeIndex)
{
    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    vkAllocateMemory(device, &allocInfo, nullptr, pMemory);
}

void Vulkan::allocate_buffer_memory(VkDeviceMemory *pMemory, VkBuffer buffer, VkMemoryPropertyFlags propertyFlags)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    u32 memoryTypeIndex = get_device_memory_type_index(memRequirements.memoryTypeBits, propertyFlags);

    allocate_memory(pMemory, memRequirements.size, memoryTypeIndex);
}

u32 Vulkan::get_device_memory_type_index(u32 typeFilter, VkMemoryPropertyFlags propertyFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties = physicalDeviceInfo.memProperties;

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter  & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
        {
            return i;
        }
    }
}

void Vulkan::create_image(VkImage *image, u32 w, u32 h, VkFormat format, VkImageTiling tiling, int usage, TextureType type, int mipCount, VkSampleCountFlagBits numSamples)
{
    VkImageCreateFlags createFlags = 0;
    int layerCount = 1;

    if (type == TEXTURE_CUBEMAP)
    {
        createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        layerCount = 6;
    }


    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = createFlags;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = {w,h,1};
    imageInfo.mipLevels = mipCount;
    imageInfo.arrayLayers = layerCount;
    imageInfo.samples = numSamples;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(device, &imageInfo, nullptr, image);
}

void Vulkan::create_image_view(VkImageView *imageView, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, TextureType type, int mipCount)
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    int layerCount = 1;

    if (type == TEXTURE_CUBEMAP)
    {
        viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        layerCount = 6;
    }

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;

    vkCreateImageView(device, &viewInfo, nullptr, imageView);
}

