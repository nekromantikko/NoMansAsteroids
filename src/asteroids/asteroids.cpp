#include "asteroids.h"
#include "../input/input.h"
#include "../util/math.h"
#include "../util/random.h"
#include "../util/resource_pool.h"
#include "string_util.h"
#include <iostream>
#include <cstring>

namespace Asteroids
{
    GameState state;

    Player player;
    Camera camera;

    #define MAX_BULLET_COUNT 128
    ResourcePool<Bullet, MAX_BULLET_COUNT> bullets;

    #define SECTOR_WIDTH 16
    #define SECTOR_HEIGHT 16
    #define MAX_SECTOR_COUNT 1024
    Sector visitedSectors[MAX_SECTOR_COUNT];
    u32 visitedSectorCount = 0;

    ResourcePool<Sector, 32> activeSectors;

    #define MAX_ASTEROID_COUNT 1024
    ResourcePool<Asteroid, MAX_ASTEROID_COUNT> asteroids;
    Asteroid *collidingAsteroids[MAX_ASTEROID_COUNT];
    u32 collidingAsteroidCount = 0;

    ResourcePool<Asteroid, 1024> goldChunks;
    Asteroid *collidingGoldChunks[1024];
    u32 collidingGoldChunksCount = 0;

    const r32 asteroidDensity = 5320; //kg / m^3
    const r32 goldDensity = 19300;
    const r32 goldPricePerKiloUSD = 56410.22;

    MeshHandle asteroidMeshes[4];
    MaterialHandle asteroidMaterials[4];

    MeshHandle shipMesh;
    MaterialHandle shipMaterial;

    MaterialHandle goldMaterial;

    ShaderHandle flatColorShader;
    ShaderHandle skyShader;
    ShaderHandle pbrShader;
    ShaderHandle toonShader;
    ShaderHandle uiShader;

    TextureHandle asciiTexture;
    MaterialHandle asciiMaterial;
    MeshHandle stringMesh = -1;
}

r32 Asteroids::volume_to_scale(r32 volume)
{
    //simplifying asteroid as a cube, scale is side length
    r32 side = std::pow(volume, 1.f/3.f);
    return side;
}

// Generated asteroid deterministically based on seed
void Asteroids::init_asteroid(Asteroid* a, r32 seed, glm::vec2 sectorOrigin)
{
    a->seed = seed;

    u32 materialIndex = std::round(seed*3);
    a->mat = asteroidMaterials[materialIndex];
    u32 meshIndex = std::round(hash11(seed)*3);
    a->mesh = asteroidMeshes[meshIndex];

    const r32 minVolume = 0.05f;
    const r32 maxVolume = 4.f;
    a->volume = hash11(seed) * (maxVolume - minVolume) + minVolume;
    a->scale = volume_to_scale(a->volume);

    // This is poop, needs better random function for this
    a->rotationAxis = glm::normalize(hash31(seed) * 2.f - 1.f);
    a->angularVelocity = hash11(seed);

    glm::vec2 asteroidPos = hash23({sectorOrigin.x, seed, sectorOrigin.y}) * glm::vec2(SECTOR_WIDTH,SECTOR_HEIGHT) - glm::vec2(SECTOR_WIDTH/2,SECTOR_HEIGHT/2) + sectorOrigin;
    a->position = {asteroidPos.x, 0, asteroidPos.y};
    r32 angle = hash11(seed) * 360.f;
    a->rotation = Quaternion::angle_axis(glm::radians(angle), a->rotationAxis);

    a->velocity = hash31(seed) - 0.5f;
    a->velocity.y = 0.f;

    a->radius = a->scale * 0.4f;
    a->mass = a->volume * asteroidDensity;

    u32 baseHealth = 3;
    a->health = baseHealth * a->scale;
}

void Asteroids::generate_sector(Sector *sector)
{
    u32 c = hash12({sector->x,sector->y}) * (MAX_ASTROIDS_PER_SECTOR - MIN_ASTROIDS_PER_SECTOR) + MIN_ASTROIDS_PER_SECTOR;
    //std::cout << "Generating " << c << " asteroids\n";

    for (int i = 0; i < c; i++)
    {
        r32 seed = hash13({sector->x,sector->y,i});

        if (asteroids.get_count() < MAX_ASTEROID_COUNT)
        {
            Asteroid *a = asteroids.create();
            init_asteroid(a, seed, {sector->x*SECTOR_WIDTH,sector->y*SECTOR_HEIGHT});
            a->sector = sector;
        }
        else std::cout << "Can't create asteroid, no room!\n";
    }
}

void Asteroids::shoot()
{
    glm::vec3 shootDirection = player.rotation * glm::vec3(0,0,1);

    if (bullets.get_count() < MAX_BULLET_COUNT)
    {
        Bullet* newBullet = bullets.create();

        r32 speed = 10.f;
        newBullet->position = player.position;
        newBullet->velocity = speed * shootDirection;
        newBullet->velocity += player.velocity;
        newBullet->radius = 0.05f;
        newBullet->lifetime = 2.0f;
    }
    else std::cout << "Can't create bullet, no room!\n";
}

void Asteroids::initialize()
{
    // Load meshes
    asteroidMeshes[0] = Renderer::create_mesh("Asteroid1", "res/meshes/asteroids/asteroids/asteroid1.gltf");
    asteroidMeshes[1] = Renderer::create_mesh("Asteroid2", "res/meshes/asteroids/asteroids/asteroid2.gltf");
    asteroidMeshes[2] = Renderer::create_mesh("Asteroid3", "res/meshes/asteroids/asteroids/asteroid3.gltf");
    asteroidMeshes[3] = Renderer::create_mesh("Asteroid4", "res/meshes/asteroids/asteroids/asteroid4.gltf");

    shipMesh = Renderer::create_mesh("Ship", "res/meshes/asteroids/ship.gltf");

    // Load textures
    TextureHandle asteroidNormalTex = Renderer::create_texture("AsteroidNormal", "res/textures/asteroids/asteroids_normals_1k.png", IMAGE_NORMAL);
    TextureHandle asteroidAOTex = Renderer::create_texture("AsteroidAO", "res/textures/asteroids/asteroids_occlusion_1k.png");
    const char* cubemapFnames[6] = {"res/textures/asteroids/skybox_right1.png",
                                    "res/textures/asteroids/skybox_left2.png",
                                    "res/textures/asteroids/skybox_top3.png",
                                    "res/textures/asteroids/skybox_bottom4.png",
                                    "res/textures/asteroids/skybox_front5.png",
                                    "res/textures/asteroids/skybox_back6.png"};
    TextureHandle envMap = Renderer::create_cubemap_texture("EnvSpace", cubemapFnames);
    Renderer::set_env_map(envMap);

    asciiTexture = Renderer::create_texture("Ascii", "res/textures/asteroids/tex_ascii.png", IMAGE_SRGB, TEXFILTER_NEAREST);

    // Load shaders
    VertexAttribFlags defaultVertexAttribs = (VertexAttribFlags)(VERTEX_POSITION_BIT | VERTEX_TEXCOORD_0_BIT | VERTEX_NORMAL_BIT | VERTEX_TANGENT_BIT | VERTEX_COLOR_BIT);

    ShaderDataLayout flatColorLayout;
    flatColorLayout.dataSize = sizeof(glm::vec4) * 2;
    flatColorLayout.propertyCount = 1;
    ShaderPropertyInfo colorInfo;
    colorInfo.name = "color";
    colorInfo.type = SHADER_PROPERTY_VEC4;
    colorInfo.count = 2;
    colorInfo.offset = 0;
    flatColorLayout.properties = &colorInfo;
    flatColorShader = Renderer::create_shader("flat_color", "shaders/vert.spv", "shaders/flat_color_frag.spv", RENDER_LAYER_OPAQUE, defaultVertexAttribs, flatColorLayout, 0);

    ShaderDataLayout skyLayout;
    skyLayout.dataSize = sizeof(glm::vec4) * 2;
    skyLayout.propertyCount = 1;
    ShaderPropertyInfo skyColorInfo;
    skyColorInfo.name = "color";
    skyColorInfo.type = SHADER_PROPERTY_VEC4;
    skyColorInfo.count = 2;
    skyColorInfo.offset = 0;
    skyLayout.properties = &skyColorInfo;
    skyShader = Renderer::create_shader("sky", "shaders/sky_vert.spv", "shaders/sky_frag.spv", RENDER_LAYER_SKYBOX, defaultVertexAttribs, skyLayout, 0);

    struct PbrData
    {
        glm::vec4 albedo;
        glm::vec2 tiling;
        r32 metallic;
        r32 roughness;
        r32 ao;
    };

    ShaderDataLayout pbrLayout;
    pbrLayout.dataSize = sizeof(PbrData);
    pbrLayout.propertyCount = 5;
    ShaderPropertyInfo pbrProps[5];
    pbrProps[0].name = "color";
    pbrProps[0].type = SHADER_PROPERTY_VEC4;
    pbrProps[0].count = 1;
    pbrProps[0].offset = 0;
    pbrProps[1].name = "tiling";
    pbrProps[1].type = SHADER_PROPERTY_VEC2;
    pbrProps[1].count = 1;
    pbrProps[1].offset = sizeof(glm::vec4);
    pbrProps[2].name = "metallic";
    pbrProps[2].type = SHADER_PROPERTY_FLOAT;
    pbrProps[2].count = 1;
    pbrProps[2].offset = sizeof(glm::vec4) + sizeof(glm::vec2);
    pbrProps[3].name = "roughness";
    pbrProps[3].type = SHADER_PROPERTY_FLOAT;
    pbrProps[3].count = 1;
    pbrProps[3].offset = sizeof(glm::vec4) + sizeof(glm::vec2) + sizeof(r32);
    pbrProps[4].name = "ao";
    pbrProps[4].type = SHADER_PROPERTY_FLOAT;
    pbrProps[4].count = 1;
    pbrProps[4].offset = sizeof(glm::vec4) + sizeof(glm::vec2) + sizeof(r32) * 2;
    pbrLayout.properties = pbrProps;
    pbrShader = Renderer::create_shader("pbr", "shaders/vert.spv", "shaders/pbr_lit_frag.spv", RENDER_LAYER_OPAQUE, defaultVertexAttribs, pbrLayout, 3);

    struct ToonData
    {
        glm::vec4 color;
        glm::vec4 shadowTint;
        glm::vec4 envColor;
        r32 shadowThreshold;
        r32 shadowSmoothness;
        alignas(16) glm::vec4 specularColor;
        r32 specularThreshold;
        r32 specularSmoothness;
        r32 anisotropy;
    };

    ShaderDataLayout toonLayout;
    toonLayout.dataSize = sizeof(ToonData);
    toonLayout.propertyCount = 9;
    ShaderPropertyInfo toonProps[9];
    toonProps[0].name = "color";
    toonProps[0].type = SHADER_PROPERTY_VEC4;
    toonProps[0].count = 1;
    toonProps[0].offset = 0;
    toonProps[1].name = "shadowTint";
    toonProps[1].type = SHADER_PROPERTY_VEC4;
    toonProps[1].count = 1;
    toonProps[1].offset = sizeof(glm::vec4);
    toonProps[2].name = "envColor";
    toonProps[2].type = SHADER_PROPERTY_VEC4;
    toonProps[2].count = 1;
    toonProps[2].offset = sizeof(glm::vec4) * 2;
    toonProps[3].name = "shadowThreshold";
    toonProps[3].type = SHADER_PROPERTY_FLOAT;
    toonProps[3].count = 1;
    toonProps[3].offset = sizeof(glm::vec4) * 3;
    toonProps[4].name = "shadowSmoothness";
    toonProps[4].type = SHADER_PROPERTY_FLOAT;
    toonProps[4].count = 1;
    toonProps[4].offset = sizeof(glm::vec4) * 3 + sizeof(r32);
    toonProps[5].name = "specularColor";
    toonProps[5].type = SHADER_PROPERTY_VEC4;
    toonProps[5].count = 1;
    toonProps[5].offset = sizeof(glm::vec4) * 3 + sizeof(r32) * 2;
    toonProps[6].name = "specularThreshold";
    toonProps[6].type = SHADER_PROPERTY_FLOAT;
    toonProps[6].count = 1;
    toonProps[6].offset = sizeof(glm::vec4) * 4 + sizeof(r32) * 2;
    toonProps[7].name = "specularSmoothness";
    toonProps[7].type = SHADER_PROPERTY_FLOAT;
    toonProps[7].count = 1;
    toonProps[7].offset = sizeof(glm::vec4) * 4 + sizeof(r32) * 3;
    toonProps[8].name = "anisotropy";
    toonProps[8].type = SHADER_PROPERTY_FLOAT;
    toonProps[8].count = 1;
    toonProps[8].offset = sizeof(glm::vec4) * 4 + sizeof(r32) * 4;
    toonLayout.properties = toonProps;
    toonShader = Renderer::create_shader("toon", "shaders/toon_vert.spv", "shaders/toon_frag.spv", RENDER_LAYER_OPAQUE, defaultVertexAttribs, toonLayout, 2);

    // Load materials
    ToonData whiteToonData;
    whiteToonData.color = {1, 1, 1, 1.0};
    whiteToonData.shadowTint = {0.471, 0.259, 0.219, 1.0};
    whiteToonData.envColor = {0.02, 0.05, 0.07, 1.0};
    whiteToonData.shadowThreshold = 0.0;
    whiteToonData.shadowSmoothness = 0.25;
    whiteToonData.specularColor = {0.48, 0.219, 0.167, 1.0};
    whiteToonData.specularThreshold = 0.17;
    whiteToonData.specularSmoothness = 0.003;
    whiteToonData.anisotropy = 0.044;

    TextureHandle whiteTex = Renderer::get_texture("White");
    TextureHandle whiteToonTextures[8] = {whiteTex, whiteTex, -1, -1, -1, -1, -1, -1};
    shipMaterial = Renderer::create_material("whiteToonMat", toonShader, (void*)&whiteToonData, whiteToonTextures, false);

    PbrData asteroidData;
    asteroidData.albedo = {0.2959238, 0.3019332, 0.3584906, 1.0};
    asteroidData.metallic = 0.0;
    asteroidData.roughness = 0.75;
    asteroidData.ao = 1.0;
    asteroidData.tiling = {1.0, 1.0};

    TextureHandle asteroidTextures[8] = {whiteTex, asteroidNormalTex, asteroidAOTex, -1, -1, -1, -1, -1};
    asteroidMaterials[0] = Renderer::create_material("asteroidMat1", pbrShader, (void*)&asteroidData, asteroidTextures, true);

    asteroidData.albedo = {0.2745098, 0.2470588, 0.2723069, 1.0};
    asteroidMaterials[1] = Renderer::create_material("asteroidMat2", pbrShader, (void*)&asteroidData, asteroidTextures, true);

    asteroidData.albedo = {0.2815503, 0.3012396, 0.3490566, 1.0};
    asteroidMaterials[2] = Renderer::create_material("asteroidMat3", pbrShader, (void*)&asteroidData, asteroidTextures, true);

    asteroidData.albedo = {0.3867925, 0.3594251, 0.3854237, 1.0};
    asteroidMaterials[3] = Renderer::create_material("asteroidMat4", pbrShader, (void*)&asteroidData, asteroidTextures, true);

    PbrData goldData;
    goldData.albedo = {1.0, 0.782, 0.344, 1.0};
    goldData.metallic = 1.0;
    goldData.roughness = 0.3;
    goldData.ao = 1.0;
    goldData.tiling = {1.0, 1.0};

    goldMaterial = Renderer::create_material("goldMat", pbrShader, (void*)&goldData, asteroidTextures, true);

    ShaderDataLayout uiLayout;
    uiLayout.dataSize = 0;
    uiLayout.propertyCount = 0;
    uiLayout.properties = nullptr;
    uiShader = Renderer::create_shader("ui", "shaders/ui_vert.spv", "shaders/ui_frag.spv", RENDER_LAYER_OVERLAY, defaultVertexAttribs, uiLayout, 1);

    TextureHandle asciiTextures[8] = {asciiTexture, -1, -1, -1, -1, -1, -1, -1};
    asciiMaterial = Renderer::create_material("asciiMat", uiShader, nullptr, asciiTextures, false);
}

void Asteroids::play_game(r32 deltaTime)
{
    if (Input::button_down(Input::BUTTON_JUMP))
    {
        shoot();
    }

    move_player(deltaTime);
    update_asteroids(deltaTime);
    update_bullets(deltaTime);
    update_gold_chunks(deltaTime);

    Sector currentSector{(s32)player.position.x/SECTOR_WIDTH, (s32)player.position.z/SECTOR_HEIGHT};

    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            Sector sector {currentSector.x + (x-1), currentSector.y + (y-1)};
            bool found = false;

            u32 sectorCount = activeSectors.get_count();
            for (int i = 0; i < sectorCount; i++)
            {
                u32 handle = activeSectors.get_handle(i);
                Sector activeSector = activeSectors[handle];

                if (activeSector.x == sector.x && activeSector.y == sector.y)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                Sector *newSector = activeSectors.create();
                if (newSector != nullptr)
                {
                    *newSector = sector;
                    generate_sector(newSector);
                }
            }
        }
    }

    for (int i = 0; i < activeSectors.get_count(); i++)
    {
        u32 handle = activeSectors.get_handle(i);
        Sector &activeSector = activeSectors[handle];

        glm::vec3 sectorOrigin = {activeSector.x*SECTOR_WIDTH,0,activeSector.y*SECTOR_HEIGHT};
        if (glm::distance(sectorOrigin, player.position) > 64.f)
        {
            activeSectors.mark_for_destruction(&activeSector);
        }
    }

    //this is a bit stupid having to set light position manually
    glm::vec3 lightDir = Quaternion::euler(glm::radians(glm::vec3(-40.f, 135.f, 0.f))) * glm::vec3(0.0,0.0,1.0);
    glm::vec4 lightColor = {1.0*1.5,1.0*1.5,0.9658*1.5,1.0};
    Renderer::set_light(player.position, lightDir, lightColor);

    camera.position = player.position + glm::vec3(0,10,0);
    camera.rotation = Quaternion::angle_axis(glm::radians(-90.0f), {1,0,0});
    Renderer::set_camera_position(camera.position);
    Renderer::set_camera_rotation(camera.rotation);

    //render ship
    Renderer::render_mesh(shipMesh, shipMaterial, player.position, player.rotation, {1.0,1.0,1.0});

    asteroids.destroy_objs();
    goldChunks.destroy_objs();
    bullets.destroy_objs();
    activeSectors.destroy_objs();

    //Draw score
    if (stringMesh >= 0)
        Renderer::destroy_mesh(stringMesh);

    char scoreString[100];
    strcpy(scoreString, "Score: ");
    itoa(state.score, scoreString + strlen(scoreString), 10);

    MeshData meshData;
    generate_mesh_from_string(scoreString, &meshData, {1,0.75,0});
    stringMesh = Renderer::create_mesh("", &meshData);

    delete[] meshData.position;
    delete[] meshData.texcoord0;
    delete[] meshData.normal;
    delete[] meshData.tangent;
    delete[] meshData.color;
    delete[] meshData.triangles;

    // TODO: Implement proper orthographic UI rendering
    // Now it's rendering in world space which makes it difficult to position / scale
    glm::vec3 scorePos = player.position + glm::vec3(-3.25, 5, -1.7);
    Renderer::render_mesh(stringMesh, asciiMaterial, scorePos, Quaternion::angle_axis(glm::radians(-90.0f), {1,0,0}), {.1,.1,.1});
}

void Asteroids::move_player(r32 deltaTime)
{
    bool moveForward = (Input::axis(Input::AXIS_LEFT_VERTICAL) < -0.001f) || Input::button(Input::BUTTON_UP);
    r32 xAxis = Input::axis(Input::AXIS_LEFT_HORIZONTAL);
    xAxis += (r32)Input::button(Input::BUTTON_RIGHT) - (r32)Input::button(Input::BUTTON_LEFT);
    xAxis = clamp(xAxis, -1.0f, 1.0f);

    r32 angle = player.rotateSpeed * deltaTime * xAxis;
    Quaternion deltaRotation = Quaternion::angle_axis(glm::radians(angle), {0.0,-1.0,0.0});
    player.rotation = deltaRotation * player.rotation;

    glm::vec3 facingDirection = player.rotation * glm::vec3(0,0,1);

    if (moveForward)
    {
        glm::vec3 moveDirection = facingDirection;
        player.velocity += moveDirection * player.acceleration * deltaTime;
    }
    else
    {
        glm::vec3 moveDirection = glm::normalize(player.velocity);
        if (std::abs(glm::length(player.velocity)) >= player.deceleration * deltaTime)
            player.velocity -= moveDirection * player.deceleration * deltaTime;
        else player.velocity = glm::vec3(0.0f);
    }

    player.position += player.velocity * deltaTime;
}

void Asteroids::update_bullets(r32 deltaTime)
{
    for (u32 i = 0; i < bullets.get_count(); i++)
    {
        u32 handle = bullets.get_handle(i);
        Bullet &b = bullets[handle];

        b.position += b.velocity * deltaTime;
        b.lifetime -= deltaTime;

        Renderer::render_mesh(Renderer::get_mesh("Sphere"), shipMaterial, b.position, Quaternion::identity(), {b.radius,b.radius,b.radius});

        if (b.lifetime <= 0)
        {
            bullets.mark_for_destruction(&b);
        }
    }
}

void Asteroids::update_asteroid(Asteroid* a, r32 deltaTime)
{
    Quaternion deltaRotation = Quaternion::angle_axis(a->angularVelocity * deltaTime, a->rotationAxis);
    a->rotation = deltaRotation * a->rotation;

    a->position += a->velocity * deltaTime;
}

void Asteroids::update_gold_chunks(r32 deltaTime)
{
    collidingGoldChunksCount = 0;
    for (u32 i = 0; i < goldChunks.get_count(); i++)
    {
        u32 handle = goldChunks.get_handle(i);
        Asteroid &a = goldChunks[handle];

        if (glm::distance(a.position, player.position) > 32.f)
        {
            goldChunks.mark_for_destruction(&a);
            continue;
        }

        glm::vec3 playerDirection = glm::normalize(player.position - a.position);
        r32 baseAcceleration = 2.f;
        glm::vec3 acceleration = baseAcceleration * playerDirection;
        a.velocity += acceleration * deltaTime;

        update_asteroid(&a, deltaTime);

        if (glm::distance(a.position, player.position) >= 10.f)
            continue;

        Renderer::render_mesh(a.mesh, a.mat, a.position, a.rotation, {a.scale,a.scale,a.scale});
        collidingGoldChunks[collidingGoldChunksCount++] = &a;
    }

    for (u32 i = 0; i < collidingGoldChunksCount; i++)
    {
        Asteroid &a = *collidingGoldChunks[i];

        //Collision with player:
        r32 distance = glm::distance(a.position, player.position);
        distance = MAX(distance, 0.0001f); // Prevent distance from being 0

        r32 combinedRadii = a.radius + player.radius;
        if (distance >= combinedRadii)
            continue;

        state.score += a.mass;

        goldChunks.mark_for_destruction(&a);
    }
}

void Asteroids::update_asteroids(r32 deltaTime)
{
    collidingAsteroidCount = 0;
    for (u32 i = 0; i < asteroids.get_count(); i++)
    {
        u32 handle = asteroids.get_handle(i);
        Asteroid &a = asteroids[handle];

        if (glm::distance(a.position, player.position) > 32.f)
        {
            asteroids.mark_for_destruction(&a);
            continue;
        }

        update_asteroid(&a, deltaTime);

        if (glm::distance(a.position, player.position) >= 10.f)
            continue;

        Renderer::render_mesh(a.mesh, a.mat, a.position, a.rotation, {a.scale,a.scale,a.scale});
        collidingAsteroids[collidingAsteroidCount++] = &a;
    }

    for (u32 i = 0; i < collidingAsteroidCount; i++)
    {
        Asteroid &a = *collidingAsteroids[i];

        //Collision with bullets:
        for (u32 j = 0; j < bullets.get_count(); j++)
        {
            u32 handle = bullets.get_handle(j);
            Bullet &b = bullets[handle];

            r32 distance = glm::distance(a.position, b.position);
            distance = MAX(distance, 0.0001f); // Prevent distance from being 0

            r32 combinedRadii = a.radius + b.radius;
            if (distance >= combinedRadii)
                continue;

            bullets.mark_for_destruction(&b);
            a.health--;

            if (a.health <= 0)
            {
                split_asteroid(&a);
                asteroids.mark_for_destruction(&a);
                return;
            }
        }

        //Collision with other asteroids:
        for (u32 j = i+1; j < collidingAsteroidCount; j++)
        {
            Asteroid &b = *collidingAsteroids[j];

            r32 distance = glm::distance(a.position, b.position);
            distance = MAX(distance, 0.0001f); // Prevent distance from being 0

            r32 combinedRadii = a.radius + b.radius;
            if (distance >= combinedRadii)
                continue;

            glm::vec3 normal = (b.position - a.position) / distance;

            // Resolve overlap
            r32 overlap = (combinedRadii - distance) * 0.5f;
            a.position -= overlap * normal;
            b.position += overlap * normal;

            glm::vec3 tangent = {-normal.z, 0, normal.x};

            r32 dotTanA = glm::dot(a.velocity,tangent);
            r32 dotTanB = glm::dot(b.velocity,tangent);

            r32 dotNormalA = glm::dot(a.velocity,normal);
            r32 dotNormalB = glm::dot(b.velocity,normal);

            r32 m1 = (dotNormalA * (a.mass - b.mass) + 2.f * b.mass * dotNormalB) / (a.mass + b.mass);
            r32 m2 = (dotNormalB * (b.mass - a.mass) + 2.f * a.mass * dotNormalA) / (a.mass + b.mass);

            a.velocity = tangent * dotTanA + normal * m1;
            b.velocity = tangent * dotTanB + normal * m2;
        }
    }
}

void Asteroids::split_asteroid(Asteroid* a)
{
    u32 chunkCount = 2 + a->seed * 4;
    if (a->mass < 1000)
        chunkCount = 0;
    r32 goldPercentage = hash11(a->seed) * 0.25;
    r32 rockPercentage = 1.f - goldPercentage;

    r32 totalMass = a->mass;
    r32 goldMass = totalMass * goldPercentage;
    r32 rockMass = totalMass * rockPercentage;

    u32 goldChunkCount = 2 + hash11(a->seed) * 3;
    if (goldMass < 200)
        goldChunkCount = 0;
    u32 totalChunkCount = chunkCount + goldChunkCount;

    glm::vec3 momentum = a->velocity * a->mass;
    glm::vec3 chunkMomentum = (momentum * rockPercentage) / (r32)chunkCount;
    glm::vec3 goldChunkMomentum = (momentum * goldPercentage) / (r32)goldChunkCount;
    r32 chunkMass = rockMass / chunkCount;
    r32 goldChunkMass = goldMass / goldChunkCount;
    glm::vec3 chunkVelocity = chunkMomentum / chunkMass;
    glm::vec3 goldChunkVelocity = goldChunkMomentum / goldChunkMass;
    r32 explosionMomentum = 1000;
    r32 explosionVelocity = (explosionMomentum * rockPercentage) / chunkMass;
    r32 goldExplosionVelocity = (explosionMomentum * goldPercentage) / goldChunkMass;
    r32 chunkVolume = chunkMass / asteroidDensity;
    r32 goldVolume = goldChunkMass / goldDensity;
    r32 chunkScale = volume_to_scale(chunkVolume);
    r32 goldChunkScale = volume_to_scale(goldVolume);
    r32 chunkAngMomentum = a->angularVelocity * a->mass / totalChunkCount;
    r32 chunkAngVel = chunkAngMomentum / chunkMass;
    r32 goldChunkAngVel = chunkAngMomentum / goldChunkMass;
    r32 chunkRadius = chunkScale * 0.4f;
    r32 goldRadius = goldChunkScale * 0.4f;

    for(int i = 0; i < chunkCount; i++)
    {
        if (asteroids.get_count() >= MAX_ASTEROID_COUNT)
            break;

        Asteroid* createdChunk = asteroids.create();

        createdChunk->sector = a->sector;
        r32 chunkSeed = hash12({a->seed, i});
        createdChunk->seed = chunkSeed;

        u32 materialIndex = std::round(chunkSeed*3);
        createdChunk->mat = asteroidMaterials[materialIndex];
        u32 meshIndex = std::round(hash11(chunkSeed)*3);
        createdChunk->mesh = asteroidMeshes[meshIndex];

        createdChunk->volume = chunkVolume;
        createdChunk->scale = chunkScale;

        createdChunk->rotationAxis = glm::normalize(hash31(chunkSeed));
        createdChunk->angularVelocity = chunkAngVel;

        r32 angle = glm::radians((360.f / chunkCount) * i);
        Quaternion direction = Quaternion::angle_axis(angle, {0,1,0});
        glm::vec3 offset = (direction * glm::vec3(0,0,1)) * chunkRadius;

        createdChunk->position = a->position + offset;
        createdChunk->rotation = Quaternion::angle_axis(hash11(chunkSeed), hash31(chunkSeed));

        glm::vec3 velocity = chunkVelocity + (direction * glm::vec3(0,0,1)) * explosionVelocity;

        createdChunk->velocity = velocity;

        createdChunk->radius = chunkRadius;
        createdChunk->mass = chunkMass;

        u32 baseHealth = 3;
        createdChunk->health = baseHealth * createdChunk->scale;
    }

    for(int g = 0; g < goldChunkCount; g++)
    {
        if (goldChunks.get_count() >= MAX_ASTEROID_COUNT)
            break;

        Asteroid* createdChunk = goldChunks.create();

        createdChunk->sector = a->sector;
        r32 chunkSeed = hash12({a->seed, g});
        createdChunk->seed = chunkSeed;

        createdChunk->mat = goldMaterial;
        u32 meshIndex = std::round(hash11(chunkSeed)*3);
        createdChunk->mesh = asteroidMeshes[meshIndex];

        createdChunk->volume = goldVolume;
        createdChunk->scale = goldChunkScale;

        createdChunk->rotationAxis = glm::normalize(hash31(chunkSeed));
        createdChunk->angularVelocity = goldChunkAngVel;

        r32 angle = glm::radians((360.f / goldChunkCount) * g);
        Quaternion direction = Quaternion::angle_axis(angle, {0,1,0});
        glm::vec3 offset = (direction * glm::vec3(0,0,1)) * goldRadius;

        createdChunk->position = a->position + offset;
        createdChunk->rotation = Quaternion::angle_axis(hash11(chunkSeed), hash31(chunkSeed));

        glm::vec3 velocity = goldChunkVelocity + (direction * glm::vec3(0,0,1)) * goldExplosionVelocity;

        createdChunk->velocity = velocity;

        createdChunk->radius = goldRadius;
        createdChunk->mass = goldChunkMass;

        u32 baseHealth = 3;
        createdChunk->health = baseHealth * createdChunk->scale;
    }
}

