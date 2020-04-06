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
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**

  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.

  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear teeth - number of teeth
          tooth_depth - depth of tooth

 **/

#define TRIS_PER_QUAD 2
#define VERTICES_PER_TRI 3
// Position, normal, and RGB colour as 3-component vectors
#define VERTEX_ATTRIBUTES 9
// Front face, front teeth, back face, back teeth, teeth, central hole
#define MODEL_PIECE_COUNT 6

void addVertex(float* buffer, float vx, float vy, float vz, float nx, float ny,
  float nz, float r, float g, float b)
{
  // Position
  buffer[0] = vx; buffer[1] = vy; buffer[2] = vz;
  // Normal
  buffer[3] = nx; buffer[4] = ny; buffer[5] = nz;
  // Colour
  buffer[6] = r;  buffer[7] = g;  buffer[8] = b;
}

void addQuad(float* buffer,
             float nx, float ny, float nz,
             float r, float g, float b,
             float v1x, float v1y, float v1z,
             float v2x, float v2y, float v2z,
             float v3x, float v3y, float v3z,
             float v4x, float v4y, float v4z)
{
  addVertex(buffer, v1x, v1y, v1z, nx, ny, nz, r, g, b);
  buffer += sizeof float * VERTEX_ATTRIBUTES;
  addVertex(buffer, v2x, v2y, v2z, nx, ny, nz, r, g, b);
  buffer += sizeof float * VERTEX_ATTRIBUTES;
  addVertex(buffer, v3x, v3y, v3z, nx, ny, nz, r, g, b);
  buffer += sizeof float * VERTEX_ATTRIBUTES;
  addVertex(buffer, v4x, v4y, v4z, nx, ny, nz, r, g, b);
  buffer += sizeof float * VERTEX_ATTRIBUTES;
  addVertex(buffer, v3x, v3y, v3z, nx, ny, nz, r, g, b);
  buffer += sizeof float * VERTEX_ATTRIBUTES;
  addVertex(buffer, v2x, v2y, v2z, nx, ny, nz, r, g, b);
}

static unsigned int
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
  GLint teeth, GLfloat tooth_depth, GLfloat* rgba, GLuint& VBO, GLuint& VAO,
  GLint& triCount)
{
  GLint i;
  GLfloat r0, r1, r2;
  GLfloat angle, da;
  GLfloat u, v, len;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.f;
  r2 = outer_radius + tooth_depth / 2.f;

  da = 2.f * (float) M_PI / teeth / 4.f;

  unsigned int VAO, VBO;
  unsigned int quadCount = teeth * MODEL_PIECE_COUNT + teeth * 4 + 1;
  unsigned int VBOstride = VERTEX_ATTRIBUTES * sizeof float;
  triCount = quadCount * TRIS_PER_QUAD;
  unsigned int VBOsize = VERTICES_PER_TRI * triCount * VBOstride;
  float* VBOdata = (float*) malloc(VBOsize);

  glGenBuffers(1, &VBO);
  // glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VBOstride, (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VBOstride, (void*)3 * sizeof float);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VBOstride, (void*)6 * sizeof float);
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  // glShadeModel(GL_FLAT); // flat or smooth shading depends on normals

  // glNormal3f(0.f, 0.f, 1.f);
  float nx = 0.;
  float ny = 0.;
  float nz = 1.;
  float r = rgba[0];
  float g = rgba[1];
  float b = rgba[2];

  /* draw front face */
  // glBegin(GL_QUAD_STRIP);
  unsigned int VBOpos = 0;
  for (i = 0; i < teeth; i++) {
    angle = i * 2.f * (float) M_PI / teeth;
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
            r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
            r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
            r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
            r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
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
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
    /*
    glVertex3f(r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f);
    glVertex3f(r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f);
    glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
    glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
    */
  }
  // glEnd();

  // glNormal3f(0.0, 0.0, -1.0);
  float nx = 0.;
  float ny = 0.;
  float nz = -1.;

  /* draw back face */
  // glBegin(GL_QUAD_STRIP);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.f * (float) M_PI / teeth;
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f,
      r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
      r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
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

    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f,
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f,
      r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
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
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r1 * (float) cos(angle), r1 * (float) sin(angle), width * 0.5f,
      r1 * (float) cos(angle), r1 * (float) sin(angle), -width * 0.5f,
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f,
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
    // glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
    nx = (float) cos(angle);
    ny = (float) sin(angle);
    nz = 0.0;
    // glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f);
    // glVertex3f(r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), width * 0.5f, // This line and the next are taken from the previous quad
      r2 * (float) cos(angle + da), r2 * (float) sin(angle + da), -width * 0.5f,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
    u = r1 * (float) cos(angle + 3 * da) - r2 * (float) cos(angle + 2 * da);
    v = r1 * (float) sin(angle + 3 * da) - r2 * (float) sin(angle + 2 * da);
    // glNormal3f(v, -u, 0.f);
    nx = v;
    ny = -u;
    nz = 0.0;
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), width * 0.5f,
      r2 * (float) cos(angle + 2 * da), r2 * (float) sin(angle + 2 * da), -width * 0.5f,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
    // glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f);
    // glVertex3f(r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f);
    // glNormal3f((float) cos(angle), (float) sin(angle), 0.f);
    nx = (float) cos(angle);
    ny = (float) sin(angle);
    nz = 0.0;
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
      r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
      r1 * (float) cos(nextAngle), r1 * (float) sin(nextAngle), width * 0.5f,
      r1 * (float) cos(nextAngle), r1 * (float) sin(nextAngle), -width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
  }

  addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
    r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), width * 0.5f,
    r1 * (float) cos(angle + 3 * da), r1 * (float) sin(angle + 3 * da), -width * 0.5f,
    r1 * (float) cos(0), r1 * (float) sin(0), width * 0.5f,
    r1 * (float) cos(0), r1 * (float) sin(0), -width * 0.5f);
  VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
  // glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), width * 0.5f);
  // glVertex3f(r1 * (float) cos(0), r1 * (float) sin(0), -width * 0.5f);

  // glEnd();

  // glShadeModel(GL_SMOOTH);

  /* draw inside radius cylinder */
  // glBegin(GL_QUAD_STRIP);
  for (i = 1; i < teeth; i++) {
    float nextAngle = (i + 1) * 2.f * (float) M_PI / teeth;
    angle = i * 2.f * (float) M_PI / teeth;
    // glNormal3f(-(float) cos(angle), -(float) sin(angle), 0.f);
    nx = -(float) cos(angle);
    ny = -(float) sin(angle);
    nz = 0.f;
    addQuad(VBOdata + VBOpos, nx, ny, nz, r, g, b,
      r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f,
      r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f,
      r0 * (float) cos(nextAngle), r0 * (float) sin(nextAngle), -width * 0.5f,
      r0 * (float) cos(nextAngle), r0 * (float) sin(nextAngle), width * 0.5f);
    VBOpos += VBOstride * VERTICES_PER_TRI * TRIS_PER_QUAD;
    // glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), -width * 0.5f);
    // glVertex3f(r0 * (float) cos(angle), r0 * (float) sin(angle), width * 0.5f);
  }
  // glEnd();

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, VBOsize, VBOdata, GL_STATIC_DRAW);
  return VAO;
}


static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLuint gear1A, gear1B, gear1S, gear2A, gear2B, gear2S, gear3A, gear3B, gear3S;
static GLint shaderProgram, vertexShader, fragmentShader;
static GLint uniformProjection, uniformModel, uniformView;
static GLfloat angle = 0.f;
static glm::mat4 projection(1.0f);

/* OpenGL draw function & timing */
static void draw(void)
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view(1.0f);
  glm::rotate(view, view_rotx, glm::vec3(1.0, 0.0, 0.0));
  glm::rotate(view, view_roty, glm::vec3(0.0, 1.0, 0.0));
  glm::rotate(view, view_rotz, glm::vec3(0.0, 0.0, 1.0));

  glUseProgram(shaderProgram);
  glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

  glm::mat4 model(1.0);
  glm::translate(model, 0.0, 0.0, -20.0);
  glm::translate(model, -3.0, -2.0, 0.0);
  glm::rotate(model, angle, glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear1B);
  glBindVertexArray(gear1A);
  glDrawArrays(GL_TRIANGLES, 0, gear1S);

  model = glm::mat4(1.0);
  glm::translate(model, 0.0, 0.0, -20.0);
  glm::translate(model, 3.1, -2.0, 0.0);
  glm::rotate(model, -2.f * angle - 9.f, glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear2B);
  glBindVertexArray(gear2A);
  glDrawArrays(GL_TRIANGLES, 0, gear2S);

  model = glm::mat4(1.0);
  glm::translate(model, 0.0, 0.0, -20.0);
  glm::translate(model, -3.1, 4.2, 0.0);
  glm::rotate(model, -2.f * angle - 25.f, glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear2B);
  glBindVertexArray(gear2A);
  glDrawArrays(GL_TRIANGLES, 0, gear2S);
  /*
  glPushMatrix();
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
      glTranslatef(-3.0, -2.0, 0.0);
      glRotatef(angle, 0.0, 0.0, 1.0);
      glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(3.1f, -2.f, 0.f);
      glRotatef(-2.f * angle - 9.f, 0.f, 0.f, 1.f);
      glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(-3.1f, 4.2f, 0.f);
      glRotatef(-2.f * angle - 25.f, 0.f, 0.f, 1.f);
      glCallList(gear3);
    glPopMatrix();

  glPopMatrix();
  */
}


/* update animation parameters */
static void animate(void)
{
  angle = 100.f * (float) glfwGetTime();
}


/* change view angle, exit upon ESC */
void key( GLFWwindow* window, int k, int s, int action, int mods )
{
  if( action != GLFW_PRESS ) return;

  switch (k) {
  case GLFW_KEY_Z:
    if( mods & GLFW_MOD_SHIFT )
      view_rotz -= 5.0;
    else
      view_rotz += 5.0;
    break;
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    break;
  case GLFW_KEY_UP:
    view_rotx += 5.0;
    break;
  case GLFW_KEY_DOWN:
    view_rotx -= 5.0;
    break;
  case GLFW_KEY_LEFT:
    view_roty += 5.0;
    break;
  case GLFW_KEY_RIGHT:
    view_roty -= 5.0;
    break;
  default:
    return;
  }
}


/* new window size */
void reshape( GLFWwindow* window, int width, int height )
{
  GLfloat aspect = (GLfloat) height / (GLfloat) width;
  GLfloat xmax, znear, zfar;

  znear = 5.0f;
  zfar  = 30.0f;
  xmax  = znear * 0.5f;

  glViewport( 0, 0, (GLint) width, (GLint) height );
  /*
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xmax, xmax, -xmax*h, xmax*h, znear, zfar );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -20.0 );
  */
  projection = glm::frustum(-xmax, xmax, -xmax * aspect, xmax * aspect, znear, zfar);
}


static bool initShaders(GLfloat* lightPos)
{
  char* vertexShaderSource;
  unsigned int vertexShaderLength;
  char* fragmentShaderSource;
  unsigned int fragmentShaderLength;
  char compileError[1024];
  unsigned int compileStatus = 0;
  memset(compileError, 1024, 0);

  // Read the shader source files
  FILE* vsSourceFile = fopen("default.vert", "r");
  if (!vsSourceFile)
  {
    fputs("default.vert cannot be opened!", stderr);
    return false;
  }

  fseek(vsSourceFile, 0, SEEK_END);
  vertexShaderLength = ftell(vsSourceFile);
  vertexShaderSource = new char[fileSize];
  fseek(vsSourceFile, 0, SEEK_SET);
  fread(vertexShaderSource, 1, fileSize, vsSourceFile);
  
  FILE* fsSourceFile = fopen("default.frag", "r");
  if (!fsSourceFile)
  {
    fputs("default.frag cannot be opened!", stderr);
    return false;
  }

  fseek(fsSourceFile, 0, SEEK_END);
  fragmentShaderLength = ftell(fsSourceFile);
  fragmentShaderSource = new char[fileSize];
  fseek(fsSourceFile, 0, SEEK_SET);
  fread(fragmentShaderSource, 1, fileSize, fsSourceFile);

  // Compile the shaders
  shaderProgram = glCreateProgram();
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, vertexShaderSource, &vertexShaderLength);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE)
  {
    glGetShaderInfoLog(vertexShader, 1024, nullptr, compileError);
    fputs(compileError, stderr);
    return false;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, fragmentShaderSource, &fragmentShaderLength);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE)
  {
    glGetShaderInfoLog(fragmentShader, 1024, nullptr, compileError);
    fputs(compileError, stderr);
    return false;
  }

  // Link the shader program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  GLint uniformLightPos = glGetUniformLocation(shaderProgram, "lightPos");
  glUniform3f(uniformLightPos, lightPos);
  uniformProjection = glGetUniformLocation(shaderProgram, "projection");
  uniformModel = glGetUniformLocation(shaderProgram, "model");
  uniformView = glGetUniformLocation(shaderProgram, "view");
  // Done!
  return true;
}

/* program & OpenGL initialization */
static void init(void)
{
  static GLfloat pos[4] = {5.f, 5.f, 10.f, 0.f};
  static GLfloat red[4] = {0.8f, 0.1f, 0.f, 1.f};
  static GLfloat green[4] = {0.f, 0.8f, 0.2f, 1.f};
  static GLfloat blue[4] = {0.2f, 0.2f, 1.f, 1.f};

  initShaders(pos);

  // glLightfv(GL_LIGHT0, GL_POSITION, pos);
  // glEnable(GL_CULL_FACE);
  // glEnable(GL_LIGHTING);
  // glEnable(GL_LIGHT0);
  // glEnable(GL_DEPTH_TEST);

  /* make the gears */
  // gear1 = glGenLists(1);
  // glNewList(gear1, GL_COMPILE);
  // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  gear(1.f, 4.f, 1.f, 20, 0.7f, red, gear1B, gear1A, gear1S);
  // glEndList();

  // gear2 = glGenLists(1);
  // glNewList(gear2, GL_COMPILE);
  // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
  gear(0.5f, 2.f, 2.f, 10, 0.7f, green, gear2B, gear2A, gear2S);
  // glEndList();

  // gear3 = glGenLists(1);
  // glNewList(gear3, GL_COMPILE);
  // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  gear(1.3f, 2.f, 0.5f, 10, 0.7f, blue, gear3B, gear3A, gear3S);
  // glEndList();

  // glEnable(GL_NORMALIZE);
}


/* program entry */
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow( 300, 300, "Gears", NULL, NULL );
    if (!window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval( 1 );

    glfwGetFramebufferSize(window, &width, &height);
    reshape(window, width, height);

    // Parse command-line options
    init();

    // Main loop
    while( !glfwWindowShouldClose(window) )
    {
        // Draw gears
        draw();

        // Update animation
        animate();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate GLFW
    glfwTerminate();

    // Exit program
    exit( EXIT_SUCCESS );
}

