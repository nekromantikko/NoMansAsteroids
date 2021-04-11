#ifndef RANDOM_H
#define RANDOM_H

#include <glm/glm.hpp>
#include "typedef.h"

r32 hash11(r32 p);
r32 hash12(glm::vec2 p);
r32 hash13(glm::vec3 p3);
glm::vec2 hash21(r32 p);
glm::vec2 hash22(glm::vec2 p);
glm::vec2 hash23(glm::vec3 p3);
glm::vec3 hash31(r32 p);
glm::vec3 hash32(glm::vec2 p);
glm::vec3 hash33(glm::vec3 p3);
glm::vec4 hash41(r32 p);
glm::vec4 hash42(glm::vec2 p);
glm::vec4 hash43(glm::vec3 p);
glm::vec4 hash44(glm::vec4 p4);

// These are just my own
r32 rand_range(r32 min, r32 max);
glm::vec2 rand_range2(r32 min, r32 max);
glm::vec3 rand_range3(r32 min, r32 max);
glm::vec4 rand_range4(r32 min, r32 max);

#endif // RANDOM_H
