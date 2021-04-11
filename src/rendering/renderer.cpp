#include "renderer.h"
#include "vulkan.h"
#include "sdl_window.h"
#include <iostream>
#include <thread>
#include <algorithm>

#include "image_loader.h"
#include "mesh_loader.h"
#include "../util/math.h"
#include "../util/resource_pool.h"

struct InternalMesh;
struct InternalTexture;

namespace Renderer
{
    DrawCall renderQueue[0x1000];
    u16 queueLength = 0;

    u16 nextDataIndex = 0;
    u16 nextTransformIndex = 0;

    RendererState state;

    glm::vec3 camPos;
    Quaternion camRot;

    const char *textureNames[MAX_TEXTURE_COUNT];
    ResourcePool<Texture, MAX_TEXTURE_COUNT> textures;

    const char *meshNames[MAX_VERTEX_BUFFER_COUNT];
    ResourcePool<Mesh, MAX_VERTEX_BUFFER_COUNT> meshes;

    const char *shaderNames[MAX_SHADER_COUNT];
    ResourcePool<Shader, MAX_SHADER_COUNT> shaders;

    const char* materialNames[MAX_MATERIAL_COUNT];
    ResourcePool<Material, MAX_MATERIAL_COUNT> materials;
}

using namespace Renderer;

void Renderer::init()
{
    std::cout << "Initializing Renderer!\n";
    std::cout << "Memory footprint:\n";
    std::cout << "Renderer state: " << sizeof(RendererState) * 2 << " bytes\n";
    std::cout << std::endl;

    //////////////////////////////////////////////////////

    SDL::create_window();

    u32 extensionCount = SDL::get_vulkan_instance_extension_count();
    const char* extensionNames[32];
    SDL::get_vulkan_instance_extension_names(extensionCount, extensionNames);

    Vulkan::init(extensionCount, extensionNames, &SDL::create_vulkan_surface);

    // Some default resources
    create_texture("NormalEmpty", "res/textures/dev/normal_empty.png", IMAGE_NORMAL);
    create_texture("White", "res/textures/dev/white.png");
    create_texture("DevGrey", "res/textures/dev/dev_256_gr_128x.jpg");

    create_mesh("Plane", "res/meshes/dev/plane.gltf");
    create_mesh("Sphere", "res/meshes/dev/sphere.gltf");
}
void Renderer::deinit()
{
    for (int i = 0; i < materials.get_count(); i++)
    {
        destroy_material(materials.get_handle(i));
    }
    materials.destroy_objs();

    for (int i = 0; i < shaders.get_count(); i++)
    {
        destroy_shader(shaders.get_handle(i));
    }
    shaders.destroy_objs();

    for (int i = 0; i < textures.get_count(); i++)
    {
        destroy_texture(textures.get_handle(i));
    }
    textures.destroy_objs();

    for (int i = 0; i < meshes.get_count(); i++)
    {
        destroy_mesh(meshes.get_handle(i));
    }
    meshes.destroy_objs();

    Vulkan::free();

    SDL::destroy_window();
}

void Renderer::set_fullscreen(bool s)
{
    SDL::set_fullscreen(s);
}

///DRAWCALLS///
s16 Renderer::render_mesh(MeshHandle mesh, MaterialHandle material, glm::vec3 pos, Quaternion rot, glm::vec3 scl, u32 c, u32 i, s32 v)
{
    u16 transformIndex = nextTransformIndex++;
    u16 dataIndex = nextDataIndex++;

    DrawCallData data;
    data.instanceCount = 1;
    data.transformIndex = transformIndex;
    //temp
    data.indexCount = c;
    data.firstIndex = i;
    data.vertexOffset = v;

    state.data[dataIndex] = data;

    Transform transform;
    transform.position = pos;
    transform.rotation = rot;
    transform.scale = scl;

    state.transform[transformIndex] = transform;

    DrawCall call{};
    call.sortingID += (dataIndex % 0x1000);
    call.sortingID += (u64)mesh << 12;
    call.sortingID += (u64)material << 20;
    call.sortingID += (u64)shaders[materials[material].shader].layer << 56;

    renderQueue[queueLength++] = call;

    return dataIndex;
}

void Renderer::sort_drawcalls()
{
	auto comp = [](const DrawCall &a, const DrawCall &b)
	{
	    return a.sortingID < b.sortingID;
	};

	std::sort(&renderQueue[0], &renderQueue[queueLength],comp);
}

void Renderer::set_camera_position(glm::vec3 pos)
{
    camPos = pos;
}

void Renderer::set_camera_rotation(Quaternion rot)
{
    camRot = rot;
}

void Renderer::set_light(glm::vec3 pos, glm::vec3 dir, glm::vec4 color)
{
    Vulkan::update_lighting(pos, dir, color);
}

void Renderer::set_env_map(TextureHandle texture)
{
    Vulkan::set_env_map(texture);
}

void Renderer::calculate_matrices()
{
    for (int i = 0; i < queueLength; i++)
    {
        u32 dataIndex = drawcall_get_data_index(renderQueue[i]);

        DrawCallData data = state.data[dataIndex];

        Transform currentTransform = state.transform[data.transformIndex];

        //std::cout << "{" << currentTransform.position.x << ", " << currentTransform.position.y << ", " << currentTransform.position.z << "}\n";

        glm::vec3 finalPos = currentTransform.position;
        Quaternion finalRotation = currentTransform.rotation;

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), finalPos);

        glm::mat4 rotation;
        rotation[0][0] = 1 - 2 * finalRotation.y * finalRotation.y - 2 * finalRotation.z * finalRotation.z;
        rotation[0][1] = 2 * finalRotation.x * finalRotation.y + 2 * finalRotation.z * finalRotation.w;
        rotation[0][2] = 2 * finalRotation.x * finalRotation.z - 2 * finalRotation.y * finalRotation.w;
        rotation[0][3] = 0;
        rotation[1][0] = 2 * finalRotation.x * finalRotation.y - 2 * finalRotation.z * finalRotation.w;
        rotation[1][1] = 1 - 2 * finalRotation.x * finalRotation.x - 2 * finalRotation.z * finalRotation.z;
        rotation[1][2] = 2 * finalRotation.y * finalRotation.z + 2 * finalRotation.x * finalRotation.w;
        rotation[1][3] = 0;
        rotation[2][0] = 2 * finalRotation.x * finalRotation.z + 2 * finalRotation.y * finalRotation.w;
        rotation[2][1] = 2 * finalRotation.y * finalRotation.z - 2 * finalRotation.x * finalRotation.w;
        rotation[2][2] = 1 - 2 * finalRotation.x * finalRotation.x - 2 * finalRotation.y * finalRotation.y;
        rotation[2][3] = 0;
        rotation[3][0] = 0;
        rotation[3][1] = 0;
        rotation[3][2] = 0;
        rotation[3][3] = 1;

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), currentTransform.scale);

        state.matrices[i] = translation * rotation * scale;
    }

    if (queueLength > 0)
        Vulkan::set_transform_data(state.matrices, queueLength);
}

void Renderer::draw()
{
    calculate_matrices();

    //draw things
    Vulkan::begin_rendering();
    Vulkan::begin_shadow_pass();

    //shadowmap rendering
    for (int i = 0; i < queueLength; i++)
    {
        DrawCall call = renderQueue[i];

        MeshHandle meshHandle = drawcall_get_mesh(call);

        MaterialHandle matHandle = drawcall_get_material(call);
        Material mat = materials[matHandle];

        if (mat.castShadows == false)
            continue;

        Vulkan::set_shadow_instance_data(i);
        Vulkan::bind_vertex_buffer(meshHandle, VERTEX_POSITION_BIT);

        u32 dataIndex = drawcall_get_data_index(call);
        DrawCallData data = state.data[dataIndex];

        u32 drawCount;
        if (data.indexCount == 0)
            drawCount = Vulkan::get_index_count(meshHandle);
        else drawCount = data.indexCount;

        Vulkan::draw_elements(drawCount, data.firstIndex, data.vertexOffset);
    }

    Vulkan::end_render_pass();
    Vulkan::begin_forward_render_pass();

    //normal rendering
    for (int i = 0; i < queueLength; i++)
    {
        DrawCall call = renderQueue[i];

        MeshHandle meshHandle = drawcall_get_mesh(call);

        MaterialHandle matHandle = drawcall_get_material(call);
        Material mat = materials[matHandle];

        Vulkan::bind_shader(mat.shader);
        Vulkan::bind_shader_data_block(mat.shader, matHandle, i);
        Vulkan::bind_vertex_buffer(meshHandle, (VertexAttribFlags)(VERTEX_POSITION_BIT | VERTEX_TEXCOORD_0_BIT | VERTEX_NORMAL_BIT | VERTEX_TANGENT_BIT | VERTEX_COLOR_BIT));

        u32 dataIndex = drawcall_get_data_index(call);
        DrawCallData data = state.data[dataIndex];

        u32 drawCount;
        if (data.indexCount == 0)
            drawCount = Vulkan::get_index_count(meshHandle);
        else drawCount = data.indexCount;

        Vulkan::draw_elements(drawCount, data.firstIndex, data.vertexOffset);
    }

    Vulkan::end_render_pass();

    Vulkan::update_post_process_descriptor_set(); // TODO: Shouldn't need to update this more than once, but it fails for some reason. Make code better
    Vulkan::render_post_process(get_mesh("Plane"));
    Vulkan::end_render_pass();

    Vulkan::stop_rendering();

    Vulkan::draw_frame(camPos, camRot);

    //Clear temporary meshes
    meshes.destroy_objs();
    materials.destroy_objs();
    textures.destroy_objs();
    shaders.destroy_objs();
}

void Renderer::clear_queue()
{
    queueLength = 0;

    nextDataIndex = 0;
    nextTransformIndex = 0;
}

u8 Renderer::drawcall_get_layer(DrawCall call)
{
    return call.sortingID >> 56;
}
u32 Renderer::drawcall_get_depth(DrawCall call)
{
    return (call.sortingID >> 32) % 0x1000000;
}
u16 Renderer::drawcall_get_material(DrawCall call)
{
    return (call.sortingID >> 20) % 0x1000;
}
u8 Renderer::drawcall_get_mesh(DrawCall call)
{
    return (call.sortingID >> 12) % 0x100;
}
u16 Renderer::drawcall_get_data_index(DrawCall call)
{
    return call.sortingID % 0x1000;
}

//////////////////////

ShaderPropertyInfo *Renderer::find_shader_property(ShaderHandle handle, const char *name)
{
    ShaderPropertyInfo *result = nullptr;
    Shader *shader = &shaders[handle];
    if (shader)
    {
        for (int i = 0; i < shader->dataLayout.propertyCount; i++)
        {
            ShaderPropertyInfo *property = &shader->dataLayout.properties[i];
            if (strncmp(name, property->name, 1000) == 0)
            {
                result = property;
                break;
            }
        }
    }

    return result;
}

//////////////////////
TextureHandle Renderer::get_texture(const char *name)
{
    for (int i = 0; i < textures.get_count(); i++)
    {
        u32 handle = textures.get_handle(i);

        if (strncmp(name, textureNames[handle], 1000) == 0)
            return handle;
    }
    return -1;
}
TextureHandle Renderer::create_texture(const char *name, const char *fname, ImageType type, TextureFilter filter)
{
    TextureHandle handle;
    Texture *texture = textures.create(&handle);
    textureNames[handle] = name;

    Image image;
    Image *imagePtr = &image;

    ImageLoader::load_image(&image, fname, type);
    Vulkan::create_texture(handle, &imagePtr, texture, TEXTURE_2D, filter);
    ImageLoader::free_image(&image);

    return handle;
}
TextureHandle Renderer::create_cubemap_texture(const char *name, const char **fnames, TextureFilter filter)
{
    TextureHandle handle;
    Texture *texture = textures.create(&handle);
    textureNames[handle] = name;

    Image cubeImages[6];
    ImageLoader::load_image(&cubeImages[0], fnames[0]);
    ImageLoader::load_image(&cubeImages[1], fnames[1]);
    ImageLoader::load_image(&cubeImages[2], fnames[2]);
    ImageLoader::load_image(&cubeImages[3], fnames[3]);
    ImageLoader::load_image(&cubeImages[4], fnames[4]);
    ImageLoader::load_image(&cubeImages[5], fnames[5]);
    Image *cubeImagePtrs[6] = {&cubeImages[0], &cubeImages[1], &cubeImages[2], &cubeImages[3], &cubeImages[4], &cubeImages[5]};
    Vulkan::create_texture(handle, cubeImagePtrs, texture, TEXTURE_CUBEMAP, filter);
    ImageLoader::free_image(&cubeImages[0]);
    ImageLoader::free_image(&cubeImages[1]);
    ImageLoader::free_image(&cubeImages[2]);
    ImageLoader::free_image(&cubeImages[3]);
    ImageLoader::free_image(&cubeImages[4]);
    ImageLoader::free_image(&cubeImages[5]);

    return handle;
}
void Renderer::destroy_texture(TextureHandle handle)
{
    Texture &texture = textures[handle];
    textures.mark_for_destruction(&texture, [](Texture* t, u32 handle) {
        Vulkan::destroy_texture(handle);
        });
}

MeshHandle Renderer::get_mesh(const char *name)
{
    for (int i = 0; i < meshes.get_count(); i++)
    {
        MeshHandle handle = meshes.get_handle(i);

        if (strncmp(name, meshNames[handle], 1000) == 0)
            return handle;
    }
    return -1;
}
MeshHandle Renderer::create_mesh(const char *name, const char *fname)
{
    MeshHandle handle;
    Mesh *mesh = meshes.create(&handle);
    meshNames[handle] = name;

    MeshLoader::load_mesh(handle, fname);

    return handle;
}
MeshHandle Renderer::create_mesh(const char *name, MeshData *data)
{
    MeshHandle handle;
    Mesh *mesh = meshes.create(&handle);
    meshNames[handle] = name;

    Vulkan::create_vertex_buffer(handle, data);

    return handle;
}
void Renderer::destroy_mesh(MeshHandle handle)
{
    Mesh &mesh = meshes[handle];
    meshes.mark_for_destruction(&mesh, [](Mesh* m, u32 handle) {
        Vulkan::destroy_vertex_buffer(handle);
        });
}

ShaderHandle Renderer::get_shader(const char *name)
{
    for (int i = 0; i < shaders.get_count(); i++)
    {
        ShaderHandle handle = shaders.get_handle(i);

        if (strncmp(name, shaderNames[handle], 1000) == 0)
            return handle;
    }
    return -1;
}
ShaderHandle Renderer::create_shader(const char *name, const char *vertFname, const char *fragFname, RenderLayer layer, VertexAttribFlags vertexInputs, ShaderDataLayout dataLayout, u32 samplerCount)
{
    ShaderHandle handle;
    Shader *shader = shaders.create(&handle);
    shaderNames[handle] = name;

    shader->layer = layer;
    shader->vertexInputs = vertexInputs;
    shader->dataLayout.dataSize = dataLayout.dataSize;
    shader->dataLayout.propertyCount = dataLayout.propertyCount;
    shader->dataLayout.properties = new ShaderPropertyInfo[dataLayout.propertyCount];
    memcpy(shader->dataLayout.properties, dataLayout.properties, sizeof(ShaderPropertyInfo) * dataLayout.propertyCount);
    shader->samplerCount = samplerCount;

    Vulkan::create_shader(handle, shader, vertFname, fragFname);
    return handle;
}
void Renderer::destroy_shader(ShaderHandle handle)
{
    Shader &shader = shaders[handle];
    shaders.mark_for_destruction(&shader, [](Shader *s, u32 handle) {
                                 Vulkan::destroy_shader(handle);
                                 delete s->dataLayout.properties;});
}

MaterialHandle Renderer::get_material(const char *name)
{
    for (int i = 0; i < materials.get_count(); i++)
    {
        MaterialHandle handle = materials.get_handle(i);

        if (strncmp(name, materialNames[handle], 1000) == 0)
            return handle;
    }
    return -1;
}
MaterialHandle Renderer::create_material(const char *name, ShaderHandle shaderHandle, void *shaderData, TextureHandle* texHandles, bool castShadows)
{
    MaterialHandle handle;
    Material *material = materials.create(&handle);
    materialNames[handle] = name;

    TextureHandle tex[8] = {texHandles[0],texHandles[1],texHandles[2],texHandles[3],texHandles[4],texHandles[5],texHandles[6],texHandles[7]};

    Shader *shader = &shaders[shaderHandle];
    material->shader = shaderHandle;
    u32 dataSize = MIN(shader->dataLayout.dataSize, MAX_SHADER_DATA_BLOCK_SIZE);
    material->dataBlock.dataSize = dataSize;
    if (dataSize > 0)
        memcpy(material->dataBlock.data, shaderData, dataSize);
    memcpy(material->textures, tex, sizeof(Texture*) * 8);
    material->castShadows = castShadows;

    Vulkan::create_shader_data_block(handle, &material->dataBlock, shaderHandle);
    Vulkan::update_shader_data_block(handle, shaderHandle, material->dataBlock, shader->samplerCount, tex);

    return handle;
}
void Renderer::destroy_material(MaterialHandle handle)
{
    Material &material = materials[handle];
    materials.mark_for_destruction(&material, [](Material *m, u32 handle) {
                                   Vulkan::free_shader_data_block(handle, m->shader);});
}

bool Renderer::set_material_float(MaterialHandle handle, const char* propertyName, u32 count, r32 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_FLOAT)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(r32) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_vec2(MaterialHandle handle, const char* propertyName, u32 count, glm::vec2 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_VEC2)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::vec2) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_vec4(MaterialHandle handle, const char* propertyName, u32 count, glm::vec4 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_VEC4)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::vec4) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_int(MaterialHandle handle, const char* propertyName, u32 count, s32 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_INT)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(s32) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_ivec2(MaterialHandle handle, const char* propertyName, u32 count, glm::ivec2 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_IVEC2)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::ivec2) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_ivec4(MaterialHandle handle, const char* propertyName, u32 count, glm::ivec4 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_IVEC4)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::ivec4) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_uint(MaterialHandle handle, const char* propertyName, u32 count, u32 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_UINT)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(u32) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_uvec2(MaterialHandle handle, const char* propertyName, u32 count, glm::uvec2 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_UVEC2)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::uvec2) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_uvec4(MaterialHandle handle, const char* propertyName, u32 count, glm::uvec4 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_UVEC4)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::uvec4) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_mat2(MaterialHandle handle, const char* propertyName, u32 count, glm::mat2 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_MAT2)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::mat2) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_mat4(MaterialHandle handle, const char* propertyName, u32 count, glm::mat4 *value)
{
    bool result = false;

    if (handle > 0)
    {
        Material *material = &materials[handle];

        ShaderPropertyInfo *foundProp = find_shader_property(material->shader, propertyName);
        if (foundProp && foundProp->type == SHADER_PROPERTY_MAT4)
        {
            memcpy(material->dataBlock.data + foundProp->offset, value, sizeof(glm::mat4) * count);
            result = true;
        }
    }

    return result;
}
bool Renderer::set_material_texture(MaterialHandle handle, const char* propertyName, Texture *value)
{
    bool result = false;
    //TODO
    return result;
}
void Renderer::apply_material_changes(MaterialHandle handle)
{
    Material &material = materials[handle];
    Shader &shader = shaders[material.shader];
    Vulkan::update_shader_data_block(handle, material.shader, material.dataBlock, shader.samplerCount, material.textures);
}
