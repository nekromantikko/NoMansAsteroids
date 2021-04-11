#ifndef MATERIAL_H
#define MATERIAL_H

enum RenderLayer //2 bits
{
    RENDER_LAYER_OPAQUE = 0,
    RENDER_LAYER_TRANSPARENT = 1,
    RENDER_LAYER_OVERLAY = 2,
    RENDER_LAYER_SKYBOX = 3
};

enum ShaderPropertyType
{
    SHADER_PROPERTY_FLOAT,
    SHADER_PROPERTY_VEC2,
    SHADER_PROPERTY_VEC4,
    SHADER_PROPERTY_INT,
    SHADER_PROPERTY_IVEC2,
    SHADER_PROPERTY_IVEC4,
    SHADER_PROPERTY_UINT,
    SHADER_PROPERTY_UVEC2,
    SHADER_PROPERTY_UVEC4,
    SHADER_PROPERTY_MAT2,
    SHADER_PROPERTY_MAT4
};

struct ShaderPropertyInfo
{
    const char* name;
    ShaderPropertyType type;
    u32 count; //if it's an array
    u32 offset; //offset to beginning of the uniform block in bytes
};

struct ShaderDataLayout
{
    u32 dataSize; //size of block in bytes
    u32 propertyCount; //number of properties
    ShaderPropertyInfo *properties;
};

struct Shader
{
    RenderLayer layer;
    VertexAttribFlags vertexInputs;
    ShaderDataLayout dataLayout;
    u32 samplerCount;
};

#define MAX_SHADER_DATA_BLOCK_SIZE 256
struct ShaderDataBlock
{
    u32 dataSize; //size of block in bytes
    char data[MAX_SHADER_DATA_BLOCK_SIZE];
};

struct Material
{
    ShaderHandle shader;
    ShaderDataBlock dataBlock;

    TextureHandle textures[8];

    bool castShadows;
};

#endif // MATERIAL_H
