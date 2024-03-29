#ifndef MATH_H
#define MATH_H

#include <glm/glm.hpp>
#include "typedef.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

template <typename T> s32 sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
T clamp(T val, T min, T max)
{
    const T t = val < min ? min : val;
    return t > max ? max : t;
}

#endif // MATH_H
