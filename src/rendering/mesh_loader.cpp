#include "mesh_loader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include "vulkan.h"
#include <cjson/cJSON.h>

void MeshLoader::init()
{

}
void MeshLoader::deinit()
{

}

void MeshLoader::load_mesh(MeshHandle handle, const char *fname)
{
    MeshData temp{};

    glm::vec3 *expandedVertices = nullptr;
    glm::vec2 *expandedUVs = nullptr;
    Triangle *expandedTris = nullptr;

    std::cout << "///LOADING glTF ASSET " << fname << "\n";
    std::ifstream file(fname, std::ios::ate);

    u32 fileSize = file.tellg();
    char buffer[0x4000];

    file.seekg(0);
    file.read(buffer, fileSize);
    file.close();

    cJSON *json = cJSON_Parse(buffer);

    if (json != NULL)
    {
        //asset info
        /*cJSON *asset = cJSON_GetObjectItemCaseSensitive(json, "asset");
        cJSON *assetVersion = cJSON_GetObjectItemCaseSensitive(asset, "version");
        cJSON *assetGenerator = cJSON_GetObjectItemCaseSensitive(asset, "generator");

        if (assetVersion != NULL)
        {
            std::cout << "glTF asset version: " << assetVersion->valuestring;
        }
        if (assetGenerator != NULL)
        {
            std::cout << "\ngenerator: " << assetGenerator->valuestring << std::endl;
        }

        //get scenes
        cJSON *scenes = cJSON_GetObjectItemCaseSensitive(json, "scenes");
        u32 sceneCount = cJSON_GetArraySize(scenes);

        std::cout << "Asset has " << sceneCount << " scene(s)\n";

        cJSON *scene;
        u32 sceneIndex = 0;
        cJSON_ArrayForEach(scene, scenes)
        {
            cJSON *nodes = cJSON_GetObjectItemCaseSensitive(json, "scenes");
            u32 nodeCount = cJSON_GetArraySize(nodes);
            std::cout << "Scene #" << sceneIndex << " has " << nodeCount << " node(s)\n";
        }*/

        //meshes
        cJSON *meshes = cJSON_GetObjectItemCaseSensitive(json, "meshes");
        cJSON *mesh = meshes->child;

        cJSON *primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
        u32 primitiveCount = cJSON_GetArraySize(primitives);
        cJSON *meshName = cJSON_GetObjectItemCaseSensitive(mesh, "name");
        std::cout << "Mesh " << meshName->valuestring << " has " << primitiveCount << " submeshes\n";

        cJSON *primitive = cJSON_GetArrayItem(primitives, 0);
        cJSON *attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");
        cJSON *indices = cJSON_GetObjectItemCaseSensitive(primitive, "indices");
        cJSON *mode = cJSON_GetObjectItemCaseSensitive(primitive, "mode");

        u32 indicesIndex = indices->valueint;
        u32 positionIndex, uvIndex, normalIndex, tangentIndex, colorIndex;
        cJSON *attribute = attributes->child;
        while (attribute != NULL)
        {
            if (strncmp(attribute->string, "POSITION", 7) == 0)
            {
                positionIndex = attribute->valueint;
            }
            else if (strncmp(attribute->string, "TEXCOORD_0", 10) == 0)
            {
                uvIndex = attribute->valueint;
            }
            else if (strncmp(attribute->string, "NORMAL", 6) == 0)
            {
                normalIndex = attribute->valueint;
            }
            else if (strncmp(attribute->string, "TANGENT", 7) == 0)
            {
                tangentIndex = attribute->valueint;
            }
            else if (strncmp(attribute->string, "COLOR_0", 7) == 0)
            {
                colorIndex = attribute->valueint;
            }
            attribute = attribute->next;
        }

        //accessors
        cJSON *accessors = cJSON_GetObjectItemCaseSensitive(json, "accessors");
        u32 accessorCount = cJSON_GetArraySize(accessors);
        std::cout << "Accessor count = " << accessorCount << std::endl;

        //bufferviews
        cJSON *bufferViews = cJSON_GetObjectItemCaseSensitive(json, "bufferViews");
        u32 bufferViewCount = cJSON_GetArraySize(bufferViews);
        std::cout << "Buffer view count = " << bufferViewCount << std::endl;

        //buffers
        cJSON *buffers = cJSON_GetObjectItemCaseSensitive(json, "buffers");
        u32 bufferCount = cJSON_GetArraySize(buffers);
        std::cout << "Buffer count = " << bufferCount << std::endl;

        //load first buffer
        cJSON *buffer0 = cJSON_GetArrayItem(buffers, 0);
        cJSON *bufferByteLength = cJSON_GetObjectItemCaseSensitive(buffer0, "byteLength");
        u32 buffer0Size = bufferByteLength->valueint;
        cJSON *bufferURI = cJSON_GetObjectItemCaseSensitive(buffer0, "uri");

        std::stringstream bufferFnameStream;

        int fnameLength = strlen(fname);
        int lastSlashIndex = 0;
        for (int i = fnameLength - 1; i > 0; i--)
        {
            if (fname[i] == '/')
            {
                lastSlashIndex = i;
                break;
            }
        }

        //The worst thing I ever wrote possibly
        char* path = new char[lastSlashIndex + 2];
        memcpy(path, fname, lastSlashIndex+1);
        path[lastSlashIndex+1] = 0;

        bufferFnameStream << path << bufferURI->valuestring;

        std::cout << "Opening binary file " << bufferFnameStream.str() << std::endl;
        std::cout << "File size in bytes = " << buffer0Size << std::endl;

        char *buffer0Data = new char[buffer0Size]{};

        std::ifstream bufferFile(bufferFnameStream.str(), std::ios::binary);

        bufferFile.read(buffer0Data, buffer0Size);
        bufferFile.close();
        delete[] path;

        //get triangles
        cJSON *indexAccessor = cJSON_GetArrayItem(accessors, indicesIndex);
        cJSON *indexBufferViewIndex = cJSON_GetObjectItemCaseSensitive(indexAccessor, "bufferView");
        cJSON *indexCount = cJSON_GetObjectItemCaseSensitive(indexAccessor, "count");

        temp.triangleCount = indexCount->valueint / 3;

        temp.triangles = new Triangle[temp.triangleCount];

        cJSON *indexBufferView = cJSON_GetArrayItem(bufferViews, indexBufferViewIndex->valueint);
        cJSON *indexBufferIndex = cJSON_GetObjectItemCaseSensitive(indexBufferView, "buffer");

        if (indexBufferIndex->valueint == 0)
        {
            u32 accessorByteOffset = cJSON_GetObjectItemCaseSensitive(indexAccessor, "byteOffset")->valueint;
            u32 byteOffset = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteOffset")->valueint;
            u32 byteLength = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteLength")->valueint;

            //std::cout << "Index accessor byte offset: " << accessorByteOffset << ", buffer view byte offset: " << byteOffset << ", byte length: " << byteLength << std::endl;
            //std::cout << "First index value = " << *(u16*)&buffer0Data[accessorByteOffset + byteOffset] << std::endl;

            memcpy(temp.triangles, &buffer0Data[accessorByteOffset + byteOffset], indexCount->valueint * sizeof(u16));
        }

        //get positions
        cJSON *positionAccessor = cJSON_GetArrayItem(accessors, positionIndex);
        cJSON *positionBufferViewIndex = cJSON_GetObjectItemCaseSensitive(positionAccessor, "bufferView");
        cJSON *positionCount = cJSON_GetObjectItemCaseSensitive(positionAccessor, "count");

        temp.vertexCount = positionCount->valueint;

        temp.position = new glm::vec3[temp.vertexCount];

        cJSON *positionBufferView = cJSON_GetArrayItem(bufferViews, positionBufferViewIndex->valueint);
        cJSON *positionBufferIndex = cJSON_GetObjectItemCaseSensitive(positionBufferView, "buffer");

        if (positionBufferIndex->valueint == 0)
        {
            u32 accessorByteOffset = cJSON_GetObjectItemCaseSensitive(positionAccessor, "byteOffset")->valueint;
            u32 byteOffset = cJSON_GetObjectItemCaseSensitive(positionBufferView, "byteOffset")->valueint;
            u32 byteLength = cJSON_GetObjectItemCaseSensitive(positionBufferView, "byteLength")->valueint;

            memcpy(temp.position, &buffer0Data[accessorByteOffset + byteOffset], positionCount->valueint * sizeof(float) * 3);
        }

        //get uv
        cJSON *uvAccessor = cJSON_GetArrayItem(accessors, uvIndex);
        cJSON *uvBufferViewIndex = cJSON_GetObjectItemCaseSensitive(uvAccessor, "bufferView");
        cJSON *uvCount = cJSON_GetObjectItemCaseSensitive(uvAccessor, "count");

        temp.texcoord0 = new glm::vec2[temp.vertexCount];

        cJSON *uvBufferView = cJSON_GetArrayItem(bufferViews, uvBufferViewIndex->valueint);
        cJSON *uvBufferIndex = cJSON_GetObjectItemCaseSensitive(uvBufferView, "buffer");

        if (uvBufferIndex->valueint == 0)
        {
            u32 accessorByteOffset = cJSON_GetObjectItemCaseSensitive(uvAccessor, "byteOffset")->valueint;
            u32 byteOffset = cJSON_GetObjectItemCaseSensitive(uvBufferView, "byteOffset")->valueint;
            u32 byteLength = cJSON_GetObjectItemCaseSensitive(uvBufferView, "byteLength")->valueint;

            memcpy(temp.texcoord0, &buffer0Data[accessorByteOffset + byteOffset], uvCount->valueint * sizeof(float) * 2);
        }

        //get normal
        cJSON *normalAccessor = cJSON_GetArrayItem(accessors, normalIndex);
        cJSON *normalBufferViewIndex = cJSON_GetObjectItemCaseSensitive(normalAccessor, "bufferView");
        cJSON *normalCount = cJSON_GetObjectItemCaseSensitive(normalAccessor, "count");

        temp.normal = new glm::vec3[temp.vertexCount];

        cJSON *normalBufferView = cJSON_GetArrayItem(bufferViews, normalBufferViewIndex->valueint);
        cJSON *normalBufferIndex = cJSON_GetObjectItemCaseSensitive(normalBufferView, "buffer");

        if (normalBufferIndex->valueint == 0)
        {
            u32 accessorByteOffset = cJSON_GetObjectItemCaseSensitive(normalAccessor, "byteOffset")->valueint;
            u32 byteOffset = cJSON_GetObjectItemCaseSensitive(normalBufferView, "byteOffset")->valueint;
            u32 byteLength = cJSON_GetObjectItemCaseSensitive(normalBufferView, "byteLength")->valueint;

            memcpy(temp.normal, &buffer0Data[accessorByteOffset + byteOffset], normalCount->valueint * sizeof(float) * 3);
        }

        //get tan
        cJSON *tangentAccessor = cJSON_GetArrayItem(accessors, tangentIndex);
        cJSON *tangentBufferViewIndex = cJSON_GetObjectItemCaseSensitive(tangentAccessor, "bufferView");
        cJSON *tangentCount = cJSON_GetObjectItemCaseSensitive(tangentAccessor, "count");

        temp.tangent = new glm::vec4[temp.vertexCount];

        cJSON *tangentBufferView = cJSON_GetArrayItem(bufferViews, tangentBufferViewIndex->valueint);
        cJSON *tangentBufferIndex = cJSON_GetObjectItemCaseSensitive(tangentBufferView, "buffer");

        if (normalBufferIndex->valueint == 0)
        {
            u32 accessorByteOffset = cJSON_GetObjectItemCaseSensitive(tangentAccessor, "byteOffset")->valueint;
            u32 byteOffset = cJSON_GetObjectItemCaseSensitive(tangentBufferView, "byteOffset")->valueint;
            u32 byteLength = cJSON_GetObjectItemCaseSensitive(tangentBufferView, "byteLength")->valueint;

            memcpy(temp.tangent, &buffer0Data[accessorByteOffset + byteOffset], tangentCount->valueint * sizeof(float) * 4);
        }

        cJSON_Delete(json);
        delete[] buffer0Data;
    }

    /*temp.vertexCount = 4;
    temp.position[0] = {-1.0, -1.0, 0.0};
    temp.position[1] = {1.0, -1.0, 0.0};
    temp.position[2] = {-1.0, 1.0, 0.0};
    temp.position[3] = {1.0, 1.0, 0.0};

    temp.texcoord0[0] = {0.0,0.0};
    temp.texcoord0[1] = {1.0,0.0};
    temp.texcoord0[2] = {0.0,1.0};
    temp.texcoord0[3] = {1.0,1.0};

    temp.triangleCount = 2;
    temp.triangles[0].index[0] = 0;
    temp.triangles[0].index[1] = 1;
    temp.triangles[0].index[2] = 2;
    temp.triangles[1].index[0] = 2;
    temp.triangles[1].index[1] = 1;
    temp.triangles[1].index[2] = 3;*/

    /*std::cout << "Mesh has " << temp.triangleCount << " triangles and " << temp.vertexCount << " vertices\n";

    for (int i = 0; i < temp.triangleCount; i++)
    {
        std::cout << "Triangle #" << i << ": {" << temp.triangles[i].index[0] << ", " << temp.triangles[i].index[1] << ", " << temp.triangles[i].index[2] << "}\n";
    }

    /*for (int i = 0; i < temp.vertexCount; i++)
    {
        std::cout << "Vert #" << i << ": {" << temp.position[i].x << ", " << temp.position[i].y << ", " << temp.position[i].z << "}\n";
    }*/

    temp.color = new glm::vec4[temp.vertexCount]{};

    Vulkan::create_vertex_buffer(handle, &temp);

    delete[] temp.triangles;
    delete[] temp.position;
    delete[] temp.texcoord0;
    delete[] temp.normal;
    delete[] temp.tangent;
    delete[] temp.color;
}
