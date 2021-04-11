#include "string_util.h"
#include <cstring>

void generate_mesh_from_string(const char *s, MeshData *data, Color color)
{
    u32 length = strlen(s);
    data->vertexCount = 4 * length;
    data->triangleCount = 2 * length;

    data->position = new glm::vec3[data->vertexCount];
    data->texcoord0 = new glm::vec2[data->vertexCount];
    data->normal = new glm::vec3[data->vertexCount];
    data->tangent = new glm::vec4[data->vertexCount];
    data->color = new glm::vec4[data->vertexCount];
    data->triangles = new Triangle[data->triangleCount];

    for (int i = 0; i < length; i++)
    {
        data->position[4*i] = {i,0,0};
        data->position[4*i+1] = {i,1,0};
        data->position[4*i+2] = {i+1,1,0};
        data->position[4*i+3] = {i+1,0,0};

        char letter = s[i];
        u32 xLetter = letter % 16;
        u32 yLetter = letter / 16;
        r32 letterWidth = 1.0 / 16.0;
        r32 letterHeight = 1.0 / 16.0;

        data->texcoord0[4*i] = {letterWidth*xLetter,letterHeight*(yLetter+1)};
        data->texcoord0[4*i+1] = {letterWidth*xLetter,letterHeight*yLetter};
        data->texcoord0[4*i+2] = {letterWidth*(xLetter+1),letterHeight*yLetter};
        data->texcoord0[4*i+3] = {letterWidth*(xLetter+1),letterHeight*(yLetter+1)};

        data->normal[4*i] = {0,0,1};
        data->normal[4*i+1] = {0,0,1};
        data->normal[4*i+2] = {0,0,1};
        data->normal[4*i+3] = {0,0,1};

        data->tangent[4*i] = {1,0,0,1};
        data->tangent[4*i+1] = {1,0,0,1};
        data->tangent[4*i+2] = {1,0,0,1};
        data->tangent[4*i+3] = {1,0,0,1};

        glm::vec4 colorWithAlpha = {color.x, color.y, color.z, 1.0f};
        data->color[4*i] = colorWithAlpha;
        data->color[4*i+1] = colorWithAlpha;
        data->color[4*i+2] = colorWithAlpha;
        data->color[4*i+3] = colorWithAlpha;

        data->triangles[2*i].index[0] = 4*i;
        data->triangles[2*i].index[1] = 4*i+2;
        data->triangles[2*i].index[2] = 4*i+1;
        data->triangles[2*i+1].index[0] = 4*i;
        data->triangles[2*i+1].index[1] = 4*i+3;
        data->triangles[2*i+1].index[2] = 4*i+2;
    }
}
