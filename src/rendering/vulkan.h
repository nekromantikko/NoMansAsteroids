#ifndef RENDERING_VULKAN_H
#define RENDERING_VULKAN_H
#include <vulkan/vulkan.h>
#include "../util/typedef.h"
#include "rendering_util.h"
#include "material.h"
#include <windows.h>

struct InternalTexture;
struct InternalMesh;
struct Image;

#define MAX_MATERIAL_COUNT 256
#define MAX_SHADER_COUNT 64
#define MAX_TEXTURE_COUNT 256
#define MAX_VERTEX_BUFFER_COUNT 256

#define MAX_BINDING_COUNT 32

namespace Vulkan
{
    ///FUNCTIONS///
    bool load_function_exported_from_loader_library();
    bool load_global_level_functions();

    ///INSTANCE///
    const VkInstance get_instance();

    ///DEVICE///
    void find_physical_device();

    void create_logical_device();
    void free_logical_device();

    enum DescriptorSetLayoutFlags
    {
        DSF_NONE = 0,
        DSF_CAMERADATA = 1,
        DSF_LIGHTINGDATA = 1 << 1,
        DSF_INSTANCEDATA = 1 << 2,
        DSF_SHADERDATA = 1 << 3,
        DSF_SHADOWMAP = 1 << 4,
        DSF_CUBEMAP = 1 << 5,
        DSF_COLOR_TEX = 1 << 6,
        DSF_DEPTH_TEX = 1 << 7
    };

    struct DescriptorSetLayoutInfo
    {
        DescriptorSetLayoutFlags flags;
        u8 samplerCount;
        u8 bindingCount;
    };

    ///DESCRIPTOR POOLS///
    void create_descriptor_pool(VkDescriptorPool *pool, DescriptorSetLayoutInfo info);
    void destroy_descriptor_pool(VkDescriptorPool *pool);

    ///DESCRIPTOR SET LAYOUTS///
    void create_descriptor_set_layout(VkDescriptorSetLayout *layout, DescriptorSetLayoutInfo info);
    void destroy_descriptor_set_layout(VkDescriptorSetLayout *layout);

    ///GLOBAL UNIFORM BUFFERS///
    void create_camera_data_buffer();
    void update_matrices(glm::vec3 camPos, Quaternion camRot); //placeholder
    void destroy_camera_data_buffer();

    ///LIGHTING DATA///
    void create_lighting_buffer();
    void update_lighting(glm::vec3 mainLightPos, glm::vec3 mainLightDir, glm::vec4 mainLightColor);
    void destroy_lighting_buffer();

    ///PER-INSTANCE DATA///
    void create_per_instance_buffer();
    void destroy_per_instance_buffer();
    void set_transform_data(glm::mat4x4 *matrices, u32 length);

    ///SHADER DATA///
    void create_shader_data_block(u32 materialIndex, ShaderDataBlock *dataBlock, u32 shaderIndex);
    void update_shader_data_block(u32 materialIndex, u32 shaderIndex, ShaderDataBlock dataBlock, u32 texCount, s32 *textures);
    void update_descriptor_set(VkDescriptorSet descriptorSet, DescriptorSetLayoutInfo info, s32 *textures = nullptr, u32 shaderDataOffset = 0);
    void free_shader_data_block(u32 materialIndex, u32 shaderIndex);
    void create_shader_data_buffer();
    void destroy_shader_data_buffer();

    ///SWAPCHAIN///
    void create_swapchain_framebuffers();
    void destroy_swapchain_framebuffers();
    void create_swapchain_image_views();
    void destroy_swapchain_image_views();
    void create_swapchain();

    ///Z_BUFFER///
    void create_depth_texture();
    void destroy_depth_texture();

    ///SHADOW MAPPING///
    void create_shadow_map();
    void destroy_shadow_map();
    void create_shadow_pipeline();
    void destroy_shadow_pipeline();

    ///ENV MAP///
    void create_placeholder_cubemap();
    void destroy_placeholder_cubemap();
    void set_env_map(u32 textureIndex);

    ///MULTISAMPLING///
    void create_multisampling_attachments();
    void destroy_multisampling_attachments();

    ///COLOR TEXTURE///
    void create_color_texture();
    void destroy_color_texture();

    ///POST PROCESSING///
    void create_pp_framebuffer();
    void destroy_pp_framebuffer();
    void create_grading_pipeline();
    void destroy_grading_pipeline();

    ///RENDER PASSES///
    void create_shadow_render_pass();
    void create_forward_render_pass();
    void create_grading_render_pass();
    void create_render_passes();
    void destroy_render_passes();

    ///SHADER MODULES///
    void create_shader_module(VkShaderModule *module, const char* fname);

    ///RENDER PIPELINES///
    void create_render_pipeline(u32 index, const char *vert, const char *frag);
    void create_shader(u32 shaderIndex, Shader *shader, const char *vert, const char *frag);
    void destroy_shader(u32 shaderIndex);

    ///COMMAND BUFFERS///
    void create_command_pool();
    void begin_rendering();
    void begin_shadow_pass();
    void set_shadow_instance_data(u32 callIndex);
    void begin_forward_render_pass();
    void end_render_pass();
    void bind_shader(u32 shaderIndex);
    void bind_shader_data_block(u32 shaderIndex, u32 materialIndex, u32 callIndex);
    void bind_vertex_buffer(u32 meshIndex, VertexAttribFlags attribs);
    void draw_elements(u32 count, u32 firstIndex = 0, s32 vertexOffset = 0);
    void update_post_process_descriptor_set();
    void update_shadow_descriptor_set();
    void render_post_process(u32 meshIndex);
    void stop_rendering();

    ///SEMAPHORES///
    void create_semaphores();
    void destroy_semaphores();

    ///TEXTURES///
    void allocate_texture_memory(u32 index);
    void free_texture_memory(u32 index);
    void create_texture_image(u32 index, TextureType type, int width, int height, VkFormat format, int mipCount);
    void create_texture_sampler(u32 index, int mipCount, TextureFilter filter);
    void copy_staging_buffer_to_texture(u32 index, VkBuffer stagingBuffer, u32 width, u32 height, int layerCount, int mipCount);
    void generate_mipmaps(u32 index, int width, int height, int layerCount, int mipCount);
    void create_texture(u32 textureIndex, Image **image, Texture *texture, TextureType type, TextureFilter filter = (TextureFilter)VK_FILTER_LINEAR, bool generateMips = true);
    void destroy_texture(u32 textureIndex);

    ///VERTEX BUFFERS///
    void create_index_buffer(u8 index, MeshData *meshData);
    void create_vertex_position_buffer(u8 index, u16 vertexCount, glm::vec3 *pos);
    void create_vertex_texcoord0_buffer(u8 index, u16 vertexCount, glm::vec2 *uv);
    void create_vertex_normal_buffer(u8 index, u16 vertexCount, glm::vec3 *nrm);
    void create_vertex_tangent_buffer(u8 index, u16 vertexCount, glm::vec4 *tan);
    void create_vertex_color_buffer(u8 index, u16 vertexCount, glm::vec4 *clr);
    void create_vertex_buffer(u32 meshIndex, MeshData *meshData);
    void destroy_vertex_buffer(u32 meshIndex);

    u32 get_vertex_count(u32 meshIndex);
    u32 get_index_count(u32 meshIndex);

    ///MAIN///
    void init(u32 extensionCount, const char** extensionNames, void (*surfaceCallback)(VkSurfaceKHR*));
    void draw_frame(glm::vec3 camPos, Quaternion camRot); //placeholder function for early testing
    void free();

    ///UTIL///
    void create_buffer(VkBuffer *pBuffer, VkDeviceSize size, VkBufferUsageFlags usage);
    void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void free_buffer(VkBuffer *pBuffer);
    void allocate_memory(VkDeviceMemory *pMemory, VkDeviceSize size, u32 memoryTypeIndex);
    void allocate_buffer_memory(VkDeviceMemory *pMemory, VkBuffer buffer, VkMemoryPropertyFlags propertyFlags);
    u32 get_device_memory_type_index(u32 typeFilter, VkMemoryPropertyFlags propertyFlags);
    void create_image(VkImage *image, u32 w, u32 h, VkFormat format, VkImageTiling tiling, int usage, TextureType type = TEXTURE_2D, int mipCount = 1, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT);
    void create_image_view(VkImageView *imageView, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, TextureType type = TEXTURE_2D, int mipCount = 1);
}

#endif // VULKAN_H
