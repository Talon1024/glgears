/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * Command line options:
 *    -info      print GL implementation information
 *    -exit      automatically exit after 30 seconds
 *
 *
 * Brian Paul
 *
 *
 * Marcus Geelnard:
 *   - Conversion to GLFW
 *   - Time based rendering (frame rate independent)
 *   - Slightly modified camera that should work better for stereo viewing
 *
 *
 * Camilla Löwy:
 *   - Removed FPS counter (this is not a benchmark)
 *   - Added a few comments
 *   - Enabled vsync
 *
 * Kevin Caccamo:
 *   - Upgraded to modern OpenGL 3.3 Core profile
 */
#pragma once
#include "glad.h"

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

struct GearVertex {
    vec3_t pos;
    vec3_t nrm;
    vec2_t bary;
};

struct GearBlueprint {
    GLfloat inner_radius;
    GLfloat outer_radius;
    GLfloat width;
    GLint teeth;
    GLfloat tooth_depth;
};

GearVertex* gear(GLuint& vertexCount, GearBlueprint bp);
