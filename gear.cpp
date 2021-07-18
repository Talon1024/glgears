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

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <cmath>
#include "glad.h"
#include "gear.h"
#include <cstdio>

// Forward declarations for addVertex/Quad/Tri. These are only used in gear.cpp.

static void addVertex(GearVertex* buffer,
                      float vx, float vy, float vz,
                      float nx, float ny, float nz,
                      float r, float g);
static void addQuad(GearVertex* buffer,
                    float nx, float ny, float nz,
                    float v1x, float v1y, float v1z,
                    float v2x, float v2y, float v2z,
                    float v3x, float v3y, float v3z,
                    float v4x, float v4y, float v4z);
static void addTri(GearVertex* buffer,
                   float nx, float ny, float nz,
                   float v1x, float v1y, float v1z,
                   float v2x, float v2y, float v2z,
                   float v3x, float v3y, float v3z);

/**
    Generate geometry for a gear. Returns an interleaved vertex buffer with these
    attributes:

    position: XYZ position of vertex (3 floats)
    normal: XYZ vertex normal vector (3 floats)
    colour: RGB vertex colour (3 floats)

    The first argument, VBOsize, is passed by reference. It is set to the number
    of floats in the buffer.

    Input:  inner_radius - radius of hole at center
                    outer_radius - radius at center of teeth
                    width - width of gear teeth - number of teeth
                    tooth_depth - depth of tooth
 **/

#define TRIS_PER_QUAD 2
#define VERTICES_PER_TRI 3
// Front face, front teeth, back face, back teeth, central hole
#define MODEL_PIECE_COUNT 5

GearVertex* gear(GLuint& vertexCount, GLfloat inner_radius, GLfloat outer_radius,
    GLfloat width, GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;

    // Distance from the center to the hole
    r0 = inner_radius;
    // Distance from the center to the inside of the tooth
    r1 = outer_radius - tooth_depth / 2.f;
    // Distance from the center to the outside of the tooth
    r2 = outer_radius + tooth_depth / 2.f;

    da = (float) M_PI / teeth / 2.;

    // One quad for each piece of the model, and four additional quads for the
    // outward faces of the teeth.
    unsigned int quadCount = teeth * MODEL_PIECE_COUNT + teeth * 4;
    // Two pieces of the gear use "addTri"
    unsigned int extraTriCount = teeth * 2;
    // Number of bytes for each vertex
    // unsigned int VBOstride = sizeof(float);
    // Number of floats in the buffer
    vertexCount = (quadCount * TRIS_PER_QUAD + extraTriCount) * VERTICES_PER_TRI;
    GearVertex* VBOdata = new GearVertex[vertexCount];

    // glShadeModel(GL_FLAT); // flat or smooth shading depends on normals

    // glNormal3f(0.f, 0.f, 1.f);
    // Emulate old OpenGL glNormal3f/glMaterialfv calls
    float nx = 0.;
    float ny = 0.;
    float nz = 1.;

    /* draw front face */
    // glBegin(GL_QUAD_STRIP);
    unsigned int VBOpos = 0;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        addTri(VBOdata + VBOpos, nx, ny, nz,
            r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
            r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f
        );
        VBOpos += VERTICES_PER_TRI;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
                r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
                r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
                r0 * (float) cos(angle + 4 * da), r0 * (float) sin(angle + 4 * da), width * 0.5f,
                r1 * (float) cos(angle + 4 * da), r1 * (float) sin(angle + 4 * da), width * 0.5f
        );
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        /*
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        if (i < teeth) {
            glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
            glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
        }
        */
    }
    // glEnd();

    /* draw front sides of teeth */
    // glBegin(GL_QUADS);
    da = 2.f * (float) M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        /*
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
        */
    }
    // glEnd();

    // glNormal3f(0.0, 0.0, -1.0);
    nx = 0.;
    ny = 0.;
    nz = -1.;

    /* draw back face */
    // glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        addTri(VBOdata + VBOpos, nx, ny, nz,
            r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f,
            r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f
        );
        VBOpos += VERTICES_PER_TRI;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
                r0 * (float) cos(angle + 4 * da), r0 * (float) sin(angle + 4 * da), -width * 0.5f,
                r1 * (float) cos(angle + 4 * da), r1 * (float) sin(angle + 4 * da), -width * 0.5f,
                r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f,
                r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f
        );
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        /*
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
        glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        if (i < teeth) {
            glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
            glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        }
        */
    }
    // glEnd();

    /* draw back sides of teeth */
    // glBegin(GL_QUADS);
    da = 2.f * (float) M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;

        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f,
            r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f,
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        /*
        glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
        glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
        */
    }
    // glEnd();

    /* draw outward faces of teeth */
    // glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        float nextAngle = (i + 1) * 2.f * (float) M_PI / teeth;
        // glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
        // glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
        u = r2 * (float) cos(angle + da) - r1 * (float) cos(angle);
        v = r2 * (float) sin(angle + da) - r1 * (float) sin(angle);
        len = (float) sqrt(u * u + v * v);
        u /= len;
        v /= len;
        // glNormal3f(v, -u, 0.0);
        // glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f);
        // glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        nx = v;
        ny = -u;
        nz = 0.0;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
            r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f,
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f,
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        // glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
        nx = (float) cos(angle);
        ny = (float) sin(angle);
        nz = 0.0;
        // glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
        // glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f, // This line and the next are taken from the previous quad
            r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        u = r1 * (float) cos(angle + 3 * da) - r2 * (float) cos(angle + 2 * da);
        v = r1 * (float) sin(angle + 3 * da) - r2 * (float) sin(angle + 2 * da);
        // glNormal3f(v, -u, 0.f);
        nx = v;
        ny = -u;
        nz = 0.0;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f,
            r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        // glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
        // glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
        // glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
        nx = (float) cos(angle);
        ny = (float) sin(angle);
        nz = 0.0;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
            r1 * (float) cos(nextAngle), r1 * (float) sin(nextAngle), width * 0.5f,
            r1 * (float) cos(nextAngle), r1 * (float) sin(nextAngle), -width * 0.5f);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
    }

    // glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), width * 0.5f);
    // glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), -width * 0.5f);

    // glEnd();

    // glShadeModel(GL_SMOOTH);

    /* draw inside radius cylinder */
    // glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float) M_PI / teeth;
        float nextAngle = (i + 1) * 2.f * (float) M_PI / teeth;
        // glNormal3f(-(float) cos(angle), -(float) sin(angle), 0.f);
        nx = -(float) cos(angle);
        ny = -(float) sin(angle);
        nz = 0.f;
        addQuad(VBOdata + VBOpos, nx, ny, nz,
            r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f,
            r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
            r0 * (float) cos(nextAngle), r0 * (float) sin(nextAngle), -width * 0.5f,
            r0 * (float) cos(nextAngle), r0 * (float) sin(nextAngle), width * 0.5f);
        // Modify normal of next vertex so the inside looks smooth
        // Vertices 3 and 4 -> 2, 3, 4
        GearVertex* vtx = VBOdata + VBOpos + 2;
        vtx->nrm.x = -(float) cos(nextAngle);
        vtx->nrm.y = -(float) sin(nextAngle);
        vtx += 1;
        // No need to modify normal Z coordinate
        vtx->nrm.x = -(float) cos(nextAngle);
        vtx->nrm.y = -(float) sin(nextAngle);
        vtx += 1;
        vtx->nrm.x = -(float) cos(nextAngle);
        vtx->nrm.y = -(float) sin(nextAngle);
        VBOpos += VERTICES_PER_TRI * TRIS_PER_QUAD;
        // glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
        // glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
    }
    // glEnd();

    /*
    printf("%-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s",
        "pos x", "pos y", "pos z",
        "nrm x", "nrm y", "nrm z",
        "bary x", "bary y");
    for (unsigned i = 0; i < VBOsize; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        printf("% -8.3f ", VBOdata[i]);
    }
    */

    return VBOdata;
}


static void addVertex(GearVertex* buffer, float vx, float vy, float vz, float nx, float ny,
    float nz, float r, float g/*, float b*/)
{
    // Position
    buffer->pos.x = vx; buffer->pos.y = vy; buffer->pos.z = vz;
    // Normal
    buffer->nrm.x = nx; buffer->nrm.y = ny; buffer->nrm.z = nz;
    // Colour
    buffer->bary.x = r;  buffer->bary.y = g; //  buffer[8] = b;
}

static void addQuad(GearVertex* buffer,
                         float nx, float ny, float nz,
                         float v1x, float v1y, float v1z,
                         float v2x, float v2y, float v2z,
                         float v3x, float v3y, float v3z,
                         float v4x, float v4y, float v4z)
{
    unsigned int pos = 0;
    /*
    addVertex(buffer + pos, v1x, v1y, v1z, nx, ny, nz, r, g, b);
    pos += sizeof(GearVertex);
    addVertex(buffer + pos, v2x, v2y, v2z, nx, ny, nz, r, g, b);
    pos += sizeof(GearVertex);
    addVertex(buffer + pos, v3x, v3y, v3z, nx, ny, nz, r, g, b);
    pos += sizeof(GearVertex);
    addVertex(buffer + pos, v3x, v3y, v3z, nx, ny, nz, r, g, b);
    pos += sizeof(GearVertex);
    addVertex(buffer + pos, v2x, v2y, v2z, nx, ny, nz, r, g, b);
    pos += sizeof(GearVertex);
    addVertex(buffer + pos, v4x, v4y, v4z, nx, ny, nz, r, g, b);
    */
    addVertex(buffer + pos, v1x, v1y, v1z, nx, ny, nz, 1, 0); pos += 1;
    addVertex(buffer + pos, v2x, v2y, v2z, nx, ny, nz, 0, 1); pos += 1;
    addVertex(buffer + pos, v4x, v4y, v4z, nx, ny, nz, 0, 0); pos += 1;
    addVertex(buffer + pos, v4x, v4y, v4z, nx, ny, nz, 0, 0); pos += 1;
    addVertex(buffer + pos, v3x, v3y, v3z, nx, ny, nz, 0, 1); pos += 1;
    addVertex(buffer + pos, v1x, v1y, v1z, nx, ny, nz, 1, 0);
}

static void addTri(GearVertex* buffer,
                        float nx, float ny, float nz,
                         float v1x, float v1y, float v1z,
                         float v2x, float v2y, float v2z,
                         float v3x, float v3y, float v3z)
{
    unsigned int pos = 0;
    addVertex(buffer + pos, v1x, v1y, v1z, nx, ny, nz, 1, 0); pos += 1;
    addVertex(buffer + pos, v2x, v2y, v2z, nx, ny, nz, 0, 1); pos += 1;
    addVertex(buffer + pos, v3x, v3y, v3z, nx, ny, nz, 0, 0);
}