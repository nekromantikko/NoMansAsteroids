#ifndef MESH_LOADER_H
#define MESH_LOADER_H
#include "../util/typedef.h"
#include "rendering_util.h"

#define GLTF_BYTE 5120
#define GLTF_UNSIGNED_BYTE 5121
#define GLTF_SHORT 5122
#define GLTF_UNSIGNED_SHORT 5123
#define GLTF_UNSIGNED_INT 5125
#define GLTF_FLOAT 5126

namespace MeshLoader
{
    void init();
    void deinit();

    void load_mesh(MeshHandle handle, const char *fname);
}

#endif
