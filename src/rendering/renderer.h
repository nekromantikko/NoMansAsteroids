#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include "../util/typedef.h"
#include "rendering_util.h"
#include "material.h"

struct Shader;
struct ShaderPropertyInfo;

namespace Renderer
{
    struct DrawCallData
    {
        u16 instanceCount;
        u16 transformIndex;

        //temp
        u32 indexCount;
        u32 firstIndex;
        s32 vertexOffset;
    };

    struct DrawCall
    {
        u64 sortingID;
    };

    struct RendererState
    {
        #define MAX_DRAWCALLS 0x1000
        DrawCallData data[MAX_DRAWCALLS]; //65536 drawcalls simultaneously should be enough
        #define MAX_TRANSFORMS 0x10000
        Transform transform[MAX_TRANSFORMS];
        glm::mat4x4 matrices[MAX_TRANSFORMS];
    };

    //////////////////////////////////////

    void init();
    void deinit();

    void set_fullscreen(bool s);

    //drawcall stuff
    s16 render_mesh(MeshHandle mesh, MaterialHandle material, glm::vec3 pos, Quaternion rot, glm::vec3 scl, u32 c = 0, u32 i = 0, s32 v = 0);
    void sort_drawcalls();

    void set_camera_position(glm::vec3 pos);
    void set_camera_rotation(Quaternion rot);
    void set_light(glm::vec3 pos, glm::vec3 dir, glm::vec4 color);
    void set_env_map(TextureHandle texture);

    void calculate_matrices();

    void draw();

    void clear_queue();

    u8 drawcall_get_layer(DrawCall call);
    u32 drawcall_get_depth(DrawCall call);
    u16 drawcall_get_material(DrawCall call);
    u8 drawcall_get_mesh(DrawCall call);
    u16 drawcall_get_data_index(DrawCall call);

    ShaderPropertyInfo *find_shader_property(ShaderHandle handle, const char *name);

    //Should these be put somewhere else instead?
    TextureHandle get_texture(const char *name);
    TextureHandle create_texture(const char *name, const char *fname, ImageType type = IMAGE_SRGB, TextureFilter filter = TEXFILTER_LINEAR);
    TextureHandle create_cubemap_texture(const char *name, const char **fnames, TextureFilter filter = TEXFILTER_LINEAR);
    void destroy_texture(TextureHandle texture);

    MeshHandle get_mesh(const char *name);
    MeshHandle create_mesh(const char *name, const char *fname);
    MeshHandle create_mesh(const char *name, MeshData *data);
    void destroy_mesh(MeshHandle mesh);

    ShaderHandle get_shader(const char *name);
    ShaderHandle create_shader(const char *name, const char *vertFname, const char *fragFname, RenderLayer layer, VertexAttribFlags vertexInputs, ShaderDataLayout dataLayout, u32 samplerCount);
    void destroy_shader(ShaderHandle shader);

    MaterialHandle get_material(const char *name);
    MaterialHandle create_material(const char *name, ShaderHandle shaderHandle, void *shaderData, TextureHandle* texHandles, bool castShadows);
    void destroy_material(MaterialHandle material);
    bool set_material_float(MaterialHandle material, const char* propertyName, u32 count, r32 *value);
    bool set_material_vec2(MaterialHandle material, const char* propertyName, u32 count, glm::vec2 *value);
    bool set_material_vec4(MaterialHandle material, const char* propertyName, u32 count, glm::vec4 *value);
    bool set_material_int(MaterialHandle material, const char* propertyName, u32 count, s32 *value);
    bool set_material_ivec2(MaterialHandle material, const char* propertyName, u32 count, glm::ivec2 *value);
    bool set_material_ivec4(MaterialHandle material, const char* propertyName, u32 count, glm::ivec4 *value);
    bool set_material_uint(MaterialHandle material, const char* propertyName, u32 count, u32 *value);
    bool set_material_uvec2(MaterialHandle material, const char* propertyName, u32 count, glm::uvec2 *value);
    bool set_material_uvec4(MaterialHandle material, const char* propertyName, u32 count, glm::uvec4 *value);
    bool set_material_mat2(MaterialHandle material, const char* propertyName, u32 count, glm::mat2 *value);
    bool set_material_mat4(MaterialHandle material, const char* propertyName, u32 count, glm::mat4 *value);
    bool set_material_texture(MaterialHandle material, const char* propertyName, Texture *value);
    void apply_material_changes(MaterialHandle material);
}

#endif // RENDERER_H
