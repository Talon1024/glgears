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
#include <vector>

struct GearVertex {
    // Position
    vec3_t pos;
    // Normal
    vec3_t nrm;
    // Barycentric coordinate, used by the shaders to display the wireframe
    vec2_t bary;
};

// Ensures three indices are always given for each triangle, otherwise you
// should get a compile error.
class IndexTriangle {
    public:
    GLuint a, b, c;
    IndexTriangle() = delete;
    IndexTriangle(GLuint a) = delete;
    IndexTriangle(GLuint a, GLuint b) = delete;
    IndexTriangle(GLuint a, GLuint b, GLuint c) : a(a), b(b), c(c) {}
};

struct GearBlueprint {
    GLfloat inner_radius;
    GLfloat outer_radius;
    GLfloat width;
    GLint teeth;
    GLfloat tooth_depth;
};

struct GearBuffersSeparate {
    std::vector<vec3_t> pos;
    std::vector<vec3_t> nrm;
    std::vector<vec2_t> bary;
    std::vector<unsigned int> indices;
    std::size_t totalSize() {
        return
            sizeof(vec3_t) * pos.size() +
            sizeof(vec3_t) * nrm.size() +
            sizeof(vec2_t) * bary.size();
    }
};

GearBuffersSeparate gear(GearBlueprint bp);
