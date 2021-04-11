#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "../util/typedef.h"
#include "../util/quaternion.h"
#include "../rendering/renderer.h"

namespace Asteroids
{
    struct GameState
    {
        u32 score;
    };

    struct Player
    {
        glm::vec3 position = {0.0, 0.0, 0.0};
        Quaternion rotation = {0.0, 0.0, 0.0, 1.0};

        r32 acceleration = 4.0f;
        r32 deceleration = 6.0f;
        r32 rotateSpeed = 300.0f;

        r32 radius = 0.5f;

        glm::vec3 velocity = {0.0, 0.0, 0.0};
    };

    struct Camera
    {
        glm::vec3 position = {0.0, 0.0, 0.0};
        Quaternion rotation = {0.0, 0.0, 0.0, 0.0};
    };

    struct Bullet
    {
        glm::vec3 position;
        glm::vec3 velocity;
        r32 radius;
        r32 lifetime;
    };

    struct Sector
    {
        #define MIN_ASTROIDS_PER_SECTOR 8
        #define MAX_ASTROIDS_PER_SECTOR 32

        s32 x, y;
    };

    struct Asteroid
    {
        Sector *sector;
        r32 seed;

        MaterialHandle mat;
        MeshHandle mesh;

        r32 volume;
        r32 scale;

        glm::vec3 rotationAxis;
        r32 angularVelocity;

        glm::vec3 position;
        Quaternion rotation;

        glm::vec3 velocity;

        //collision
        r32 radius;
        r32 mass;

        u32 health;
    };

    r32 volume_to_scale(r32 volume);
    void init_asteroid(Asteroid* a, r32 seed, glm::vec2 sectorOrigin);
    void generate_sector(Sector *sector);
    void shoot();
    void mark_bullet_for_deletion(Bullet *b);
    void destroy_bullet(Bullet* b);
    void initialize();
    void play_game(r32 deltaTime);
    void move_player(r32 deltaTime);
    void update_bullets(r32 deltaTime);
    void update_gold_chunks(r32 deltaTime);
    void update_asteroid(Asteroid *a, r32 deltaTime);
    void update_asteroids(r32 deltaTime);
    void split_asteroid(Asteroid *a);
    void mark_gold_chunk_for_deletion(Asteroid *a);
    void mark_asteroid_for_deletion(Asteroid *a);
    void mark_sector_for_deletion(Sector *s);
}

#endif // ASTEROIDS_H
