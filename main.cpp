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
#include "gear.h"

/*
struct model_t {
  GLuint array;
  GLuint buffer;
  GLuint vertexCount;
  glm::mat4 transform;
};

struct model_list_item_t {
  model_list_item_t* next;
  model_t* data;
};

struct model_list_t {
  model_list_item_t* head;
} models;

void add_model(model_t* model)
{
  if (!models.head)
  {
    model_list_item_t* current = new model_list_item_t;
    current->data = model;
    models.head = current;
  }
  else
  {
    model_list_item_t* current = models.head;
    while (current->next != nullptr)
    {
      current = current->next;
    }
    current->next = new model_list_item_t;
    current = current->next;
    current->data = model;
  }
}
*/
static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLuint gear1A, gear1B, gear1S, gear2A, gear2B, gear2S, gear3A, gear3B, gear3S;
static GLint shaderProgram, vertexShader, fragmentShader;
static GLint uniformProjection, uniformModel, uniformView, uniformLightPos, uniformLit, uniformZoom;
static GLfloat angle = 0.f;
static glm::mat4 projection(1.0f);
static bool wireframe = false;
static bool lit = true;
static bool rotategears = true;

/* OpenGL draw function & timing */
static void draw(void)
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view(1.0f);
  view = glm::translate(view, glm::vec3(0.0, 0.0, -20.0));
  view = glm::rotate(view, glm::radians(view_rotx), glm::vec3(1.0, 0.0, 0.0));
  view = glm::rotate(view, glm::radians(view_roty), glm::vec3(0.0, 1.0, 0.0));
  view = glm::rotate(view, glm::radians(view_rotz), glm::vec3(0.0, 0.0, 1.0));

  glUseProgram(shaderProgram);
  glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
  glUniform1f(uniformZoom, 1);
  glUniform3f(uniformLightPos, sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 10);
  glUniform1ui(uniformLit, lit);

  glm::mat4 model(1.0);
  model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear1B);
  glBindVertexArray(gear1A);
  glDrawArrays(GL_TRIANGLES, 0, gear1S);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(3.1, -2.0, 0.0));
  model = glm::rotate(model, glm::radians(-2.f * angle - 9.f), glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear2B);
  glBindVertexArray(gear2A);
  glDrawArrays(GL_TRIANGLES, 0, gear2S);

  model = glm::mat4(1.0);
  model = glm::translate(model, glm::vec3(-3.1, 4.2, 0.0));
  model = glm::rotate(model, glm::radians(-2.f * angle - 25.f), glm::vec3(0.0, 0.0, 1.0));
  glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
  glBindBuffer(GL_ARRAY_BUFFER, gear3B);
  glBindVertexArray(gear3A);
  glDrawArrays(GL_TRIANGLES, 0, gear3S);

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
  if (rotategears)
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
  case GLFW_KEY_V:
    if (!wireframe)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    wireframe = !wireframe;
    break;
  case GLFW_KEY_L:
    lit = !lit;
    break;
  case GLFW_KEY_T:
    rotategears = !rotategears;
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
  // float fov = (float) glm::radians(100.);
  // projection = glm::perspective(fov, aspect, znear, zfar);
}


static bool initShaders()
{
  char* vertexShaderSource;
  int vertexShaderLength;
  char* fragmentShaderSource;
  int fragmentShaderLength;
  int compileStatus = 0;

  // Read the shader source files
  FILE* vsSourceFile = fopen("default.vert", "r");
  if (!vsSourceFile)
  {
    fputs("default.vert cannot be opened!", stderr);
    return false;
  }

  fseek(vsSourceFile, 0, SEEK_END);
  vertexShaderLength = ftell(vsSourceFile);
  vertexShaderSource = new char[vertexShaderLength];
  fseek(vsSourceFile, 0, SEEK_SET);
  fread(vertexShaderSource, 1, vertexShaderLength, vsSourceFile);
  
  FILE* fsSourceFile = fopen("default.frag", "r");
  if (!fsSourceFile)
  {
    fputs("default.frag cannot be opened!", stderr);
    return false;
  }

  fseek(fsSourceFile, 0, SEEK_END);
  fragmentShaderLength = ftell(fsSourceFile);
  fragmentShaderSource = new char[fragmentShaderLength];
  fseek(fsSourceFile, 0, SEEK_SET);
  fread(fragmentShaderSource, 1, fragmentShaderLength, fsSourceFile);

  // Compile the shaders
  shaderProgram = glCreateProgram();
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, &vertexShaderLength);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE)
  {
    int infoLength;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
    char* compileError = (char*) malloc(infoLength);
    glGetShaderInfoLog(vertexShader, infoLength, &infoLength, compileError);
    fputs(compileError, stderr);
    free(compileError);
    return false;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, &fragmentShaderLength);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE)
  {
    int infoLength;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
    char* compileError = (char*) malloc(infoLength);
    glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, compileError);
    fputs(compileError, stderr);
    free(compileError);
    return false;
  }

  // Link the shader program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  uniformLightPos = glGetUniformLocation(shaderProgram, "lightPos");
  uniformProjection = glGetUniformLocation(shaderProgram, "projection");
  uniformModel = glGetUniformLocation(shaderProgram, "model");
  uniformView = glGetUniformLocation(shaderProgram, "view");
  uniformLit = glGetUniformLocation(shaderProgram, "lit");
  uniformZoom = glGetUniformLocation(shaderProgram, "zoom");
  // Done!
  return true;
}

/* program & OpenGL initialization */
static void init(void)
{
  // static GLfloat pos[4] = {5.f, 5.f, 10.f, 0.f};
  static GLfloat red[4] = {0.8f, 0.1f, 0.f, 1.f};
  static GLfloat green[4] = {0.f, 0.8f, 0.2f, 1.f};
  static GLfloat blue[4] = {0.2f, 0.2f, 1.f, 1.f};

  GLuint gear1E, gear2E, gear3E;

  initShaders();

  glLineWidth(2.0);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // glEnable(GL_LIGHTING);
  // glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  /* make the gears */
  gear(1.f, 4.f, 1.f, 20, 0.7f, red, gear1B, gear1A, gear1S, gear1E);
  // glEndList();

  // gear2 = glGenLists(1);
  // glNewList(gear2, GL_COMPILE);
  // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
  gear(0.5f, 2.f, 2.f, 10, 0.7f, green, gear2B, gear2A, gear2S, gear2E);
  // glEndList();

  // gear3 = glGenLists(1);
  // glNewList(gear3, GL_COMPILE);
  // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  gear(1.3f, 2.f, 0.5f, 10, 0.7f, blue, gear3B, gear3A, gear3S, gear3E);
  //gear(.25, .75, 0.125f, 10, 0.125f, blue, gear1B, gear1A, gear1S);
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

    // glfwWindowHint(GLFW_DEPTH_BITS, 16);
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
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

