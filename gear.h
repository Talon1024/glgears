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
#include "vector.h"
#include <memory>

struct GearVertex {
    // Position
    vec3_t pos;
    // Normal
    vec3_t nrm;
    // Barycentric coordinate, used by the shaders to display the wireframe
    vec2_t bary;
};

struct GearBlueprint {
    GLfloat inner_radius;
    GLfloat outer_radius;
    GLfloat width;
    GLint teeth;
    GLfloat tooth_depth;
};

struct GearBuffers {
    std::unique_ptr<GearVertex[]> vertexBuffer;
    std::unique_ptr<GLuint[]> indexBuffer;
    GLuint indexCount;
    GLuint vertexCount;
};

GearBuffers gear(GearBlueprint bp);
