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
#include "input.h"
#include "camera.h"

static CameraHead cameraHead;
static CameraEye cameraEye;
static GLuint gear1A, gear1B, gear1S, gear2A, gear2B, gear2S, gear3A, gear3B, gear3S;
static GLint shaderProgram;
static GLint uniformProjection, uniformModel, uniformView, uniformLightPos, uniformLit, uniformZoom, uniformColour, uniformWireframe;
static GLfloat angle = 0.f;

/* OpenGL draw function & timing */
static void draw(void)
{
    const KeyInputState* input = Input::GetKeyState();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = cameraHead.getViewMatrix();
    glm::mat4 projection = cameraEye.getProjectionMatrix();

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(uniformZoom, 1);
    glUniform3f(uniformLightPos, sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 10);
    glUniform1ui(uniformLit, input->lit);
    glUniform1ui(uniformWireframe, input->wireframe);

    glm::mat4 model(1.0);
    model = glm::translate(model, glm::vec3(-3.0, -2.0, 0.0));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(uniformColour, 0.8f, 0.1f, 0.f);
    glBindBuffer(GL_ARRAY_BUFFER, gear1B);
    glBindVertexArray(gear1A);
    glDrawArrays(GL_TRIANGLES, 0, gear1S);

    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(3.1, -2., 0.0));
    model = glm::rotate(model, glm::radians(-2.f * angle - 9.f), glm::vec3(0.0, 0.0, 1.0));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(uniformColour, 0.f, 0.8f, 0.2f);
    glBindBuffer(GL_ARRAY_BUFFER, gear2B);
    glBindVertexArray(gear2A);
    glDrawArrays(GL_TRIANGLES, 0, gear2S);

    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(-3.1, 4.2, 0.0));
    model = glm::rotate(model, glm::radians(-2.f * angle - 25.f), glm::vec3(0.0, 0.0, 1.0));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(uniformColour, 0.2f, 0.2f, 1.f);
    glBindBuffer(GL_ARRAY_BUFFER, gear3B);
    glBindVertexArray(gear3A);
    glDrawArrays(GL_TRIANGLES, 0, gear3S);
}

/* update animation parameters */
static void animate(void)
{
    const KeyInputState* input = Input::GetKeyState();
    if (input->animate)
        angle = 100.f * (float) glfwGetTime();
    if (input->forward)
        cameraHead.move(glm::vec3(0, .125, 0));
    if (input->backward)
        cameraHead.move(glm::vec3(0, -.125, 0));
    if (input->left)
        cameraHead.move(glm::vec3(-.125, 0, 0));
    if (input->right)
        cameraHead.move(glm::vec3(.125, 0, 0));
    const MouseInputState* mouse = Input::GetMouseState();
    cameraHead.theta += mouse->moveX;
    cameraHead.phi -= mouse->moveY;
    cameraHead.phi = glm::clamp<GLfloat>(cameraHead.phi, -90, 90);
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
    uniformProjection = glGetUniformLocation(shaderProgram, "projection");
    uniformModel = glGetUniformLocation(shaderProgram, "model");
    uniformView = glGetUniformLocation(shaderProgram, "view");
    uniformLit = glGetUniformLocation(shaderProgram, "lit");
    uniformZoom = glGetUniformLocation(shaderProgram, "zoom");
    uniformColour = glGetUniformLocation(shaderProgram, "colour");
    uniformWireframe = glGetUniformLocation(shaderProgram, "wireframe");
    // Done!
    return success;
}

void addGear(
    GLuint& buffer, GLuint& array, GLuint& size, GLfloat innerRadius,
    GLfloat outerRadius, GLfloat width, GLint teeth, GLfloat toothDepth);

/* program & OpenGL initialization */
static void init(void)
{
    initShaders();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    addGear(gear1B, gear1A, gear1S, 1.f, 4.f, 1.f, 20, 0.7f);
    addGear(gear2B, gear2A, gear2S, 0.5f, 2.f, 2.f, 10, 0.7f);
    addGear(gear3B, gear3A, gear3S, 1.3f, 2.f, 0.5f, 10, 0.7f);
}

void addGear(
    GLuint& buffer, GLuint& array, GLuint& size, GLfloat innerRadius,
    GLfloat outerRadius, GLfloat width, GLint teeth, GLfloat toothDepth)
{
    /* make the gears */
    // Stride (total number of bytes for all vertex attributes in an interleaved buffer)
    GLuint VBOstride = sizeof(GearVertex);
    // Offsets of each attribute, in bytes
    void* posOffset = 0;
    void* nrmOffset = (void*)(3 * sizeof(float));
    void* colOffset = (void*)(6 * sizeof(float));
    float* gearBuffer = gear(size, innerRadius, outerRadius, width, teeth, toothDepth);

    // Set up buffer and vertex array
    glGenBuffers(1, &buffer);
    glGenVertexArrays(1, &array);
    // Upload buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(GearVertex), gearBuffer, GL_STATIC_DRAW);
    glBindVertexArray(array);
    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VBOstride, posOffset);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VBOstride, nrmOffset);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VBOstride, colOffset);
    glEnableVertexAttribArray(2);
    // Release bindings
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Free buffer, since it has been uploaded already
    delete[] gearBuffer;
}

static void onWindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    cameraEye.onWindowResize(window, width, height);
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

