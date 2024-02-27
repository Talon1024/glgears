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

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "khrplatform.h"
#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gear.h"
#include "input.h"
#include "camera.h"
#include "3dobject.h"

GLint uniformColour, uniformModel;
GLfloat angle = 0.f;

static Camera viewpoint;
static GLint shaderProgram;
static GLint uniformProjection, uniformWireframe, uniformLightPos, uniformLit, uniformZoom;

/* OpenGL draw function & timing */
static void draw(const std::vector<ThreeDimensionalObject> &objects)
{
    const KeyInputState* input = Input::GetKeyState();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = viewpoint.getViewProjMatrix();

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(uniformZoom, 1);
    glUniform3f(uniformLightPos, sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 5., cos(glfwGetTime()) * 10);
    glUniform1ui(uniformLit, input->lit);
    glUniform1ui(uniformWireframe, input->wireframe);

    for (const ThreeDimensionalObject& obj : objects) {
        obj.draw();
    }
}

/* update animation parameters */
static void animate(void)
{
    const KeyInputState* input = Input::GetKeyState();
    if (input->animate)
        angle = 100.f * (float) glfwGetTime();
    if (input->forward)
        viewpoint.move(glm::vec3(0, .125, 0));
    if (input->backward)
        viewpoint.move(glm::vec3(0, -.125, 0));
    if (input->left)
        viewpoint.move(glm::vec3(-.125, 0, 0));
    if (input->right)
        viewpoint.move(glm::vec3(.125, 0, 0));
    if (input->up)
        viewpoint.move(glm::vec3(0, 0, .125));
    if (input->down)
        viewpoint.move(glm::vec3(0, 0, -.125));
    const MouseInputState* mouse = Input::GetMouseState();
    viewpoint.theta += mouse->moveX;
    viewpoint.phi -= mouse->moveY;
    viewpoint.phi = glm::clamp<GLfloat>(viewpoint.phi, -90, 90);
}

static GLint loadShader(FILE* sourceFile, GLint shaderType)
{
    char* source;
    int length, shader, compileStatus = 0;
    // Get length, allocate memory for source code, and read it in
    fseek(sourceFile, 0, SEEK_END);
    length = ftell(sourceFile);
    source = new char[length];
    fseek(sourceFile, 0, SEEK_SET);
    fread(source, 1, length, sourceFile);
    // Create and compile shader
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);
    delete[] source;
    // Show error and warning messages from the compiler
    int textLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &textLength);
    if (textLength > 0)
    {
        char* compileErrorText = new char[textLength];
        glGetShaderInfoLog(shader, textLength, &textLength, compileErrorText);
        fputs(compileErrorText, stderr);
        delete[] compileErrorText;
    }
    // Check whether shader successfully compiled or not
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static bool initShaders()
{
    bool success = true;
    GLint vertexShader, fragmentShader;
    shaderProgram = glCreateProgram();
    // Read the shader source files
    FILE* vsSourceFile = fopen("default.vert", "r");
    if (!vsSourceFile)
    {
        fputs("default.vert cannot be opened!", stderr);
        success = false;
    }
    else
    {
        vertexShader = loadShader(vsSourceFile, GL_VERTEX_SHADER);
        if (!vertexShader) success = false;
        fclose(vsSourceFile);
    }

    FILE* fsSourceFile = fopen("default.frag", "r");
    if (!fsSourceFile)
    {
        fputs("default.frag cannot be opened!", stderr);
        success = false;
    }
    else
    {
        fragmentShader = loadShader(fsSourceFile, GL_FRAGMENT_SHADER);
        if (!fragmentShader) success = false;
        fclose(fsSourceFile);
    }
    if (!success)
    {
        glDeleteProgram(shaderProgram);
        return false;
    }

    // Link the shader program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    uniformLightPos = glGetUniformLocation(shaderProgram, "lightPos");
    uniformProjection = glGetUniformLocation(shaderProgram, "projView");
    uniformModel = glGetUniformLocation(shaderProgram, "model");
    uniformLit = glGetUniformLocation(shaderProgram, "lit");
    uniformZoom = glGetUniformLocation(shaderProgram, "zoom");
    uniformColour = glGetUniformLocation(shaderProgram, "colour");
    uniformWireframe = glGetUniformLocation(shaderProgram, "wireframe");
    // Done!
    return success;
}

/* program & OpenGL initialization */
static void init(std::vector<ThreeDimensionalObject> &objects)
{
    initShaders();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    objects.emplace_back(
        vec3_t {{0.8, 0.1, 0.0}}, // colour
        vec3_t {{-3.0, -2.0, 0.0}} // position
    );
    objects.back().setupForDrawing({1., 4., 1., 20, 0.7});

    objects.emplace_back(
        vec3_t {{0., 0.8, 0.2}}, // colour
        vec3_t {{3.1, -2., 0.0}}, // position
        -2.0, -9.0 // angleMultiply, angleAdd
    );
    objects.back().setupForDrawing({0.5, 2., 2., 10, 0.7});

    objects.emplace_back(
        vec3_t {{0.2, 0.2, 1.}}, // colour
        vec3_t {{-3.1, 4.2, 0.0}}, // position
        -2.0, -25.0 // angleMultiply, angleAdd
    );
    objects.back().setupForDrawing({1.3, 2., 0.5, 10, 0.7});

    viewpoint.position = glm::vec3(2.0, -5.0, 3.0);
    viewpoint.phi = -25.0;
    viewpoint.theta = -15.0;
}

static void onWindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    viewpoint.onWindowResize(window, width, height);
}

/* program entry */
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    const unsigned int windowWidth = 800;
    const unsigned int windowHeight = 540;

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow( windowWidth, windowHeight, "Gears", NULL, NULL );
    if (!window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, onWindowResize);
    glfwSetKeyCallback(window, Input::onKeyAction);
    glfwSetCursorPosCallback(window, Input::onMouseMove);
    glfwSetMouseButtonCallback(window, Input::onMouseButton);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    onWindowResize(window, windowWidth, windowHeight);
    glfwSwapInterval( 1 );

    std::vector<ThreeDimensionalObject> objects;

    // Parse command-line options
    init(objects);

    // Main loop
    while( !glfwWindowShouldClose(window) )
    {
        // Draw gears
        draw(objects);

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

