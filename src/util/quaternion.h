#ifndef QUATERNION_H
#define QUATERNION_H

#include <glm/glm.hpp>
#include "typedef.h"

struct Quaternion
{
    r32 x, y, z, w;

    Quaternion() {}
    Quaternion(r32 x, r32 y, r32 z, r32 w) : x(x), y(y), z(z), w(w) {}
    inline static Quaternion angle_axis(r32 angle, glm::vec3 axis)
    {
        Quaternion result;
        result.w = std::cos(angle/2);
        glm::vec3 temp = glm::normalize(axis) * std::sin(angle/2);
        result.x = temp.x;
        result.y = temp.y;
        result.z = temp.z;

        return result;
    }
    inline Quaternion conjugate() const
    {
        return Quaternion(-x, -y, -z, w);
    }

    inline static Quaternion identity()
    {
        return Quaternion(0,0,0,1);
    }
    inline static Quaternion euler(glm::vec3 angles);
    inline static Quaternion euler(r32 x, r32 y, r32 z)
    {
        return euler({x,y,z});
    }
};

inline bool operator==(const Quaternion& a, const Quaternion& b)
{
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}
inline bool operator!=(const Quaternion& a, const Quaternion& b)
{
    return !(a == b);
}
inline Quaternion operator*(const Quaternion& a, const Quaternion& b)
{
    //This is called a Hamilton product
    return Quaternion(a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
                        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
                        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
                        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
}
inline Quaternion& operator*=(Quaternion& a, const Quaternion& b)
{
    a = a * b;
    return a;
}
inline glm::vec3 operator*(const Quaternion& q, const glm::vec3& v)
{
    // A vector3 is a quaternion with no real component, so we can do this:
    Quaternion vQuat = {v.x, v.y, v.z, 0.0f};

    Quaternion qResult = (q * vQuat) * q.conjugate();
    return {qResult.x, qResult.y, qResult.z};
}

inline Quaternion Quaternion::euler(glm::vec3 angles)
{
    //rotation order ZYX
    return angle_axis(angles.x, {1,0,0}) * angle_axis(angles.y, {0,1,0}) * angle_axis(angles.z, {0,0,1});
}

#endif // QUATERNION_H
