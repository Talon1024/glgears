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

#include <math.h>
#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <cmath>
#include "gear.h"
#include "vector.h"
#include <vector>

// Forward declarations for addQuad/Tri. These are only used in gear.cpp.

static void addQuad(
    std::vector<GearVertex>& buffer,
    vec3_t n,
    vec3_t v1, vec3_t v2,
    vec3_t v3, vec3_t v4
);
static void addIndexedQuad(
    std::vector<GearVertex>& geom,
    std::vector<IndexTriangle>& index,
    GLuint indexStart,
    vec3_t n,
    vec3_t v1, vec3_t v2,
    vec3_t v3, vec3_t v4
);
static void addTri(std::vector<GearVertex>& buffer,
                   vec3_t n,
                   vec3_t v1,
                   vec3_t v2,
                   vec3_t v3);

/**
    Generate geometry for a gear. Returns an interleaved vertex buffer with these
    attributes:

    position: XYZ position of vertex (3 floats)
    normal: XYZ vertex normal vector (3 floats)
    colour: RGB vertex colour (3 floats)

    The first argument, vertexCount, is passed by reference. It is set to the
    number of floats in the buffer.

    Input:  inner_radius - radius of hole at center
            outer_radius - radius at center of teeth
            width - width of gear
            teeth - number of teeth
            tooth_depth - depth of tooth
 **/

#define TRIS_PER_QUAD 2
#define VERTICES_PER_TRI 3
// Top face, top teeth, bottom face, bottom teeth, central hole
#define MODEL_PIECE_COUNT 5
// #define FIRST_TOOTH_ONLY

GearBuffers gear(GearBlueprint bp)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;

    GLfloat
        inner_radius = bp.inner_radius,
        outer_radius = bp.outer_radius,
        width = bp.width,
        tooth_depth = bp.tooth_depth;
    GLint teeth = bp.teeth;

    // Distance from the center to the hole
    r0 = inner_radius;
    // Distance from the center to the inside of the tooth
    r1 = outer_radius - tooth_depth / 2.f;
    // Distance from the center to the outside of the tooth
    r2 = outer_radius + tooth_depth / 2.f;

    da = M_PI / teeth / 2.;

    // One quad for each piece of the model, and four additional quads for the
    // outward faces of the teeth.
    // unsigned int quadCount = teeth * MODEL_PIECE_COUNT + teeth * 4;
    // Two pieces of the gear use "addTri"
    // unsigned int extraTriCount = teeth * 2;
    // Number of vertices in the buffer
    // size_t vertexCount = (quadCount * TRIS_PER_QUAD + extraTriCount) * VERTICES_PER_TRI;
    GearBuffers buff {
        std::vector<GearVertex> {},
        std::vector<IndexTriangle> {}
    };
    unsigned int currentIndexStart = 0;
    unsigned int circleIndexStart = 0;

    // Emulate old OpenGL glNormal3f/glMaterialfv calls
    vec3_t normal = {{0., 0., 1.}};

    bool inside_first = false;

    /* draw front face */
    #ifdef FIRST_TOOTH_ONLY
    bool first_tooth_added = false;
    #endif
    for (i = 0; i < teeth; i++) {
        bool last = i == (teeth - 1);
        angle = i * 2.f * (float) M_PI / teeth;
        currentIndexStart = buff.vertexCount();
        #ifdef FIRST_TOOTH_ONLY
        if (!first_tooth_added) {
        first_tooth_added = true;
        #endif
        addIndexedQuad(
            buff.vertexBuffer, buff.indexBuffer,
            currentIndexStart, normal,
            {{r0 * cosf(angle), r0 * sinf(angle), width * 0.5f}}, // 0
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), width * 0.5f}}, // 1
            {{r0 * cosf(angle + 4 * da), r0 * sinf(angle + 4 * da), width * 0.5f}}, // 2
            {{r1 * cosf(angle + 4 * da), r1 * sinf(angle + 4 * da), width * 0.5f}} // 3
        );
        buff.vertexBuffer.push_back({ // 4
            {{r1 * cosf(angle), r1 * sinf(angle), width * 0.5f}},
            normal,
            {{0., 0.}}
        });
        buff.indexBuffer.push_back({
            currentIndexStart + 0,
            currentIndexStart + 4,
            currentIndexStart + 1,
        });
        /* draw front sides of teeth */
        buff.vertexBuffer.push_back({ // 5
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), width * 0.5f}},
            normal,
            {{1., 1.}}
        });
        buff.vertexBuffer.push_back({ // 6
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), width * 0.5f}},
            normal,
            {{1., 0.}}
        });
        // Quad is 5, 6, 1, 4
        buff.indexBuffer.push_back({
            currentIndexStart + 5,
            currentIndexStart + 6,
            currentIndexStart + 4,
        });
        buff.indexBuffer.push_back({
            currentIndexStart + 1,
            currentIndexStart + 4,
            currentIndexStart + 6,
        });
        #ifdef FIRST_TOOTH_ONLY
        }
        #endif
    }
    #ifdef FIRST_TOOTH_ONLY
    first_tooth_added = false;
    #endif

    normal.z = -1.;

    /* draw back face */
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        #ifdef FIRST_TOOTH_ONLY
        if (!first_tooth_added) {
        first_tooth_added = true;
        #endif
        addTri(buff.vertexBuffer, normal,
            {{r1 * cosf(angle), r1 * sinf(angle), -width * 0.5f}},
            {{r0 * cosf(angle), r0 * sinf(angle), -width * 0.5f}},
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), -width * 0.5f}}
        );
        addQuad(buff.vertexBuffer, normal,
                {{r0 * cosf(angle + 4 * da), r0 * sinf(angle + 4 * da), -width * 0.5f}},
                {{r1 * cosf(angle + 4 * da), r1 * sinf(angle + 4 * da), -width * 0.5f}},
                {{r0 * cosf(angle), r0 * sinf(angle), -width * 0.5f}},
                {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), -width * 0.5f}}
        );
        /* draw back sides of teeth */
        addQuad(buff.vertexBuffer, normal,
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), -width * 0.5f}},
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), -width * 0.5f}},
            {{r1 * cosf(angle), r1 * sinf(angle), -width * 0.5f}},
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), -width * 0.5f}});
        #ifdef FIRST_TOOTH_ONLY
        }
        #endif
    }
    #ifdef FIRST_TOOTH_ONLY
    first_tooth_added = false;
    #endif

    /* draw outward faces of teeth */
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        GLfloat nextAngle = (i + 1) * 2.f * (float) M_PI / teeth;
        GLfloat u = r2 * cosf(angle + da) - r1 * cosf(angle);
        GLfloat v = r2 * sinf(angle + da) - r1 * sinf(angle);
        GLfloat len = (float) sqrt(u * u + v * v);
        u /= len;
        v /= len;

        #ifdef FIRST_TOOTH_ONLY
        if (!first_tooth_added) {
        first_tooth_added = true;
        #endif
        normal = {{v, -u, 0.0}};
        addIndexedQuad(
            buff.vertexBuffer, buff.indexBuffer,
            buff.vertexCount(), normal,
            {{r1 * cosf(angle), r1 * sinf(angle), width * 0.5f}},
            {{r1 * cosf(angle), r1 * sinf(angle), -width * 0.5f}},
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), width * 0.5f}},
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), -width * 0.5f}}
        );
        normal = {{cosf(angle), sinf(angle), 0.0}};
        addIndexedQuad(
            buff.vertexBuffer, buff.indexBuffer,
            buff.vertexCount(), normal,
            // The next line and the one after that are taken from the previous quad
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), width * 0.5f}},
            {{r2 * cosf(angle + da), r2 * sinf(angle + da), -width * 0.5f}},
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), width * 0.5f}},
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), -width * 0.5f}}
        );
        u = r1 * cosf(angle + 3 * da) - r2 * cosf(angle + 2 * da);
        v = r1 * sinf(angle + 3 * da) - r2 * sinf(angle + 2 * da);
        normal = {{v, -u, 0.0}};
        addIndexedQuad(
            buff.vertexBuffer, buff.indexBuffer,
            buff.vertexCount(), normal,
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), width * 0.5f}},
            {{r2 * cosf(angle + 2 * da), r2 * sinf(angle + 2 * da), -width * 0.5f}},
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), width * 0.5f}},
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), -width * 0.5f}});
        normal = {{cosf(angle), sinf(angle), 0.0}};
        addIndexedQuad(
            buff.vertexBuffer, buff.indexBuffer,
            buff.vertexCount(), normal,
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), width * 0.5f}},
            {{r1 * cosf(angle + 3 * da), r1 * sinf(angle + 3 * da), -width * 0.5f}},
            {{r1 * cosf(nextAngle), r1 * sinf(nextAngle), width * 0.5f}},
            {{r1 * cosf(nextAngle), r1 * sinf(nextAngle), -width * 0.5f}}
        );
        #ifdef FIRST_TOOTH_ONLY
        }
        #endif
    }

    #ifdef FIRST_TOOTH_ONLY
    first_tooth_added = false;
    #endif

    /* draw inside radius cylinder */
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        float nextAngle = (i + 1) * 2.f * (float) M_PI / teeth;

        #ifdef FIRST_TOOTH_ONLY
        if (!first_tooth_added) {
        first_tooth_added = true;
        #endif
        normal = {{-cosf(angle), -sinf(angle), 0.0}};
        if (!inside_first) {
            addIndexedQuad(
                buff.vertexBuffer, buff.indexBuffer,
                buff.vertexCount(), normal,
                {{r0 * cosf(angle), r0 * sinf(angle), -width * 0.5f}},
                {{r0 * cosf(angle), r0 * sinf(angle), width * 0.5f}},
                {{r0 * cosf(nextAngle), r0 * sinf(nextAngle), -width * 0.5f}},
                {{r0 * cosf(nextAngle), r0 * sinf(nextAngle), width * 0.5f}}
            );
            // Modify normal of next vertex so the inside looks smooth
            // I need to modify vertices 3 and 4.
            // Also, no need to modify normal Z coordinate, since it's already 0
            GearVertex* vtx = buff.vertexBuffer.data() + buff.vertexCount() - 2;
            vtx->nrm.x = -cosf(nextAngle);
            vtx->nrm.y = -sinf(nextAngle);
            vtx += 1;
            vtx->nrm.x = -cosf(nextAngle);
            vtx->nrm.y = -sinf(nextAngle);
            inside_first = true;
        } else {
            // Add two vertices and quad indices
            buff.vertexBuffer.push_back({
                {{r0 * cosf(nextAngle), r0 * sinf(nextAngle), -width * 0.5f}},
                normal,
                {{1., 0.}}
            });
            buff.vertexBuffer.push_back({
                {{r0 * cosf(nextAngle), r0 * sinf(nextAngle), width * 0.5f}},
                normal,
                {{0., 0.}}
            });
            buff.indexBuffer.push_back({
                buff.vertexCount() - 1,
                buff.vertexCount() - 2,
                buff.vertexCount() - 3,
            });
            buff.indexBuffer.push_back({
                buff.vertexCount() - 4,
                buff.vertexCount() - 3,
                buff.vertexCount() - 2,
            });
        }
        #ifdef FIRST_TOOTH_ONLY
        }
        #endif
        // glVertex3f(r0 * cosf(angle), r0 * sinf(angle), -width * 0.5f);
        // glVertex3f(r0 * cosf(angle), r0 * sinf(angle), width * 0.5f);
    }
    // glEnd();
    #ifdef FIRST_TOOTH_ONLY
    first_tooth_added = false;
    #endif

    // printf("VBOsize and VBOpos: %d %d\n", vertexCount, VBOpos);

    return buff;
}

static void addQuad(std::vector<GearVertex>& buffer,
                    vec3_t n,
                    vec3_t v1,
                    vec3_t v2,
                    vec3_t v3,
                    vec3_t v4)
{
    buffer.push_back({v1, n, {{1., 0.}}});
    buffer.push_back({v2, n, {{0., 1.}}});
    buffer.push_back({v4, n, {{0., 0.}}});
    buffer.push_back({v4, n, {{0., 0.}}});
    buffer.push_back({v3, n, {{0., 1.}}});
    buffer.push_back({v1, n, {{1., 0.}}});
}

static void addIndexedQuad(std::vector<GearVertex>& geom,
                    std::vector<IndexTriangle>& index,
                    unsigned int indexStart,
                    vec3_t n,
                    vec3_t v1,
                    vec3_t v2,
                    vec3_t v3,
                    vec3_t v4)
{
    geom.push_back({v1, n, {{1., 0.}}});
    geom.push_back({v2, n, {{0., 1.}}});
    geom.push_back({v3, n, {{0., 1.}}});
    geom.push_back({v4, n, {{0., 0.}}});
    index.push_back({indexStart + 0, indexStart + 1, indexStart + 3});
    index.push_back({indexStart + 3, indexStart + 2, indexStart + 0});
}

static void addTri(std::vector<GearVertex>& buffer,
                   vec3_t n,
                   vec3_t v1,
                   vec3_t v2,
                   vec3_t v3)
{
    buffer.push_back({v1, n, {{1., 0.}}});
    buffer.push_back({v2, n, {{0., 1.}}});
    buffer.push_back({v3, n, {{0., 0.}}});
}