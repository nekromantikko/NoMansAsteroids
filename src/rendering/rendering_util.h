#ifndef RENDERING_UTIL_H
#define RENDERING_UTIL_H
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../util/quaternion.h"

#define SCREEN_HEIGHT 576
#define SCREEN_WIDTH 1024

typedef glm::vec3 VertexPos;
typedef glm::vec2 VertexUV;
typedef glm::vec3 VertexNormal;
typedef glm::vec4 VertexTangent;
typedef glm::vec3 Color;

typedef s32 ShaderHandle;
typedef s32 TextureHandle;
typedef s32 MaterialHandle;
typedef s32 MeshHandle;

struct Triangle
{
    u16 index[3];

    Triangle() {}
    Triangle(u16 a, u16 b, u16 c)
    {
        index[0] = a;
        index[1] = b;
        index[2] = c;
    }
};

enum VertexAttribFlags
{
    VERTEX_POSITION_BIT = 1,
    VERTEX_TEXCOORD_0_BIT = 2,
    VERTEX_TEXCOORD_1_BIT = 4,
    VERTEX_TEXCOORD_2_BIT = 8,
    VERTEX_TEXCOORD_3_BIT = 16,
    VERTEX_NORMAL_BIT = 32,
    VERTEX_TANGENT_BIT = 64,
    VERTEX_COLOR_BIT = 128,
    VERTEX_JOINTS_BIT = 256,
    VERTEX_WEIGHTS_BIT = 512

};

struct MeshData
{
    u32 vertexCount;
    glm::vec3 *position;
    glm::vec2 *texcoord0;
    glm::vec3 *normal;
    glm::vec4 *tangent;
    glm::vec4 *color;
    u32 triangleCount;
    Triangle *triangles;
};

struct Mesh
{
    // Empty :c
};

///////////////////////////////////////

enum ImageType
{
    IMAGE_SRGB,
    IMAGE_NORMAL,
};

struct Image
{
    ImageType type;
    u32 width, height;
    u8 *pixels;
};

enum TextureFilter
{
    TEXFILTER_NEAREST = 0,
    TEXFILTER_LINEAR = 1
};

enum TextureType
{
    TEXTURE_2D,
    TEXTURE_CUBEMAP
};

struct Texture
{
    TextureType type;
};

////////////////////////////////////////

struct Transform
{
    glm::vec3 position;
    Quaternion rotation;
    glm::vec3 scale;
};

#endif // RENDERING_UTIL_H
