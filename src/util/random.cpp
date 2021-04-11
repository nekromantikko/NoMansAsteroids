#include "random.h"
#include <stdlib.h>

// Hash without Sine
// MIT License...
/* Copyright (c)2014 David Hoskins.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

//----------------------------------------------------------------------------------------
//  1 out, 1 in...
r32 hash11(r32 p)
{
    p = glm::fract(p * .1031f);
    p *= p + 33.33f;
    p *= p + p;
    return glm::fract(p);
}

//----------------------------------------------------------------------------------------
//  1 out, 2 in...
r32 hash12(glm::vec2 p)
{
	glm::vec3 p3 = glm::fract(glm::vec3(p.x,p.y,p.x) * .1031f);
    p3 += glm::dot(p3, glm::vec3(p3.y,p3.z,p3.x) + 33.33f);
    return glm::fract((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------------------
//  1 out, 3 in...
r32 hash13(glm::vec3 p3)
{
	p3 = glm::fract(p3 * .1031f);
    p3 += glm::dot(p3, glm::vec3(p3.z,p3.y,p3.x) + 31.32f);
    return glm::fract((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------------------
//  2 out, 1 in...
glm::vec2 hash21(r32 p)
{
	glm::vec3 p3 = glm::fract(glm::vec3(p) * glm::vec3(.1031f, .1030f, .0973f));
	p3 += glm::dot(p3, glm::vec3(p3.y,p3.z,p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x,p3.x)+glm::vec2(p3.y,p3.z))*glm::vec2(p3.z,p3.y));

}

//----------------------------------------------------------------------------------------
///  2 out, 2 in...
glm::vec2 hash22(glm::vec2 p)
{
	glm::vec3 p3 = glm::fract(glm::vec3(p.x,p.y,p.x) * glm::vec3(.1031f, .1030f, .0973f));
    p3 += glm::dot(p3, glm::vec3(p3.y,p3.z,p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x,p3.x)+glm::vec2(p3.y,p3.z))*glm::vec2(p3.z,p3.y));

}

//----------------------------------------------------------------------------------------
///  2 out, 3 in...
glm::vec2 hash23(glm::vec3 p3)
{
	p3 = glm::fract(p3 * glm::vec3(.1031f, .1030f, .0973f));
    p3 += glm::dot(p3, glm::vec3(p3.y,p3.z,p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x,p3.x)+glm::vec2(p3.y,p3.z))*glm::vec2(p3.z,p3.y));
}

//----------------------------------------------------------------------------------------
//  3 out, 1 in...
glm::vec3 hash31(r32 p)
{
   glm::vec3 p3 = glm::fract(glm::vec3(p) * glm::vec3(.1031f, .1030f, .0973f));
   p3 += glm::dot(p3, glm::vec3(p3.y,p3.z,p3.x) + 33.33f);
   return glm::fract((glm::vec3(p3.x,p3.x,p3.y)+glm::vec3(p3.y,p3.z,p3.z))*glm::vec3(p3.z,p3.y,p3.x));
}


//----------------------------------------------------------------------------------------
///  3 out, 2 in...
glm::vec3 hash32(glm::vec2 p)
{
	glm::vec3 p3 = glm::fract(glm::vec3(p.x,p.y,p.x) * glm::vec3(.1031f, .1030f, .0973f));
    p3 += glm::dot(p3, glm::vec3(p3.y,p3.x,p3.z) + 33.33f);
    return glm::fract((glm::vec3(p3.x,p3.x,p3.y)+glm::vec3(p3.y,p3.z,p3.z))*glm::vec3(p3.z,p3.y,p3.x));
}

//----------------------------------------------------------------------------------------
///  3 out, 3 in...
glm::vec3 hash33(glm::vec3 p3)
{
	p3 = glm::fract(p3 * glm::vec3(.1031f, .1030f, .0973f));
    p3 += glm::dot(p3, glm::vec3(p3.y,p3.x,p3.z) + 33.33f);
    return glm::fract((glm::vec3(p3.x,p3.x,p3.y)+glm::vec3(p3.y,p3.x,p3.x))*glm::vec3(p3.z,p3.y,p3.x));

}

//----------------------------------------------------------------------------------------
// 4 out, 1 in...
glm::vec4 hash41(r32 p)
{
	glm::vec4 p4 = glm::fract(glm::vec4(p) * glm::vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += glm::dot(p4, glm::vec4(p4.w,p4.z,p4.x,p4.y) + 33.33f);
    return glm::fract((glm::vec4(p4.x,p4.x,p4.y,p4.z)+glm::vec4(p4.y,p4.z,p4.z,p4.w))*glm::vec4(p4.z,p4.y,p4.w,p4.x));

}

//----------------------------------------------------------------------------------------
// 4 out, 2 in...
glm::vec4 hash42(glm::vec2 p)
{
	glm::vec4 p4 = glm::fract(glm::vec4(p.x,p.y,p.x,p.y) * glm::vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += glm::dot(p4, glm::vec4(p4.w,p4.z,p4.x,p4.y) + 33.33f);
    return glm::fract((glm::vec4(p4.x,p4.x,p4.y,p4.z)+glm::vec4(p4.y,p4.z,p4.z,p4.w))*glm::vec4(p4.z,p4.y,p4.w,p4.x));

}

//----------------------------------------------------------------------------------------
// 4 out, 3 in...
glm::vec4 hash43(glm::vec3 p)
{
	glm::vec4 p4 = glm::fract(glm::vec4(p.x,p.y,p.z,p.x)  * glm::vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += glm::dot(p4, glm::vec4(p4.w,p4.z,p4.x,p4.y) + 33.33f);
    return glm::fract((glm::vec4(p4.x,p4.x,p4.y,p4.z)+glm::vec4(p4.y,p4.z,p4.z,p4.w))*glm::vec4(p4.z,p4.y,p4.w,p4.x));
}

//----------------------------------------------------------------------------------------
// 4 out, 4 in...
glm::vec4 hash44(glm::vec4 p4)
{
	p4 = glm::fract(p4  * glm::vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += glm::dot(p4, glm::vec4(p4.w,p4.z,p4.x,p4.y) + 33.33f);
    return glm::fract((glm::vec4(p4.x,p4.x,p4.y,p4.z)+glm::vec4(p4.y,p4.z,p4.z,p4.w))*glm::vec4(p4.z,p4.y,p4.w,p4.x));
}

///////////////////////////////////////

r32 rand_range(r32 min, r32 max)
{
    return min + (r32)rand()/RAND_MAX * (max - min);
}
glm::vec2 rand_range2(r32 min, r32 max)
{
    return {rand_range(min,max),rand_range(min,max)};
}
glm::vec3 rand_range3(r32 min, r32 max)
{
    return {rand_range(min,max),rand_range(min,max),rand_range(min,max)};
}
glm::vec4 rand_range4(r32 min, r32 max)
{
    return {rand_range(min,max),rand_range(min,max),rand_range(min,max),rand_range(min,max)};
}
