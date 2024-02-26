#pragma once

typedef union vec3_t
{
    float xyz[3];
    float& operator[](unsigned long index)
    {
        return xyz[index];
    }
    struct {
        float x;
        float y;
        float z;
    };
} vec3_t;

typedef union vec2_t
{
    float xy[2];
    float& operator[](unsigned long index)
    {
        return xy[index];
    }
    struct {
        float x;
        float y;
    };
} vec2_t;
