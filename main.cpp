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

static GLint uniformColour, uniformModel;
static GLfloat angle = 0.f;

struct ThreeDimensionalObject {
    private:
    // OpenGL resource handles
    GLuint ibo;
    GLuint vbo;
    GLuint vao;
    // Used for rendering a complete object
    GLuint vertexCount;

    public:

    // Prevent copying! See: https://www.khronos.org/opengl/wiki/Common_Mistakes#RAII_and_hidden_destructor_calls
    // Also, this class needs a move constructor and a move assignment operator
    // since this class also has a destructor
    ThreeDimensionalObject(ThreeDimensionalObject& other) = delete;
    ThreeDimensionalObject& operator= (ThreeDimensionalObject& other) = delete;

    // Default values for angleMultiply and angleAdd
    ThreeDimensionalObject(
        vec3_t colour,
        vec3_t position
    ) : colour(colour),
        position(position),
        angleMultiply(1.0),
        angleAdd(0.0) {}

    // Custom values for angleMultiply and angleAdd
    ThreeDimensionalObject(
        vec3_t colour,
        vec3_t position,
        float angleMultiply,
        float angleAdd
    ) : colour(colour),
        position(position),
        angleMultiply(angleMultiply),
        angleAdd(angleAdd) {}

    ThreeDimensionalObject(ThreeDimensionalObject&& other) : ibo(other.ibo),
        vbo(other.vbo), vao(other.vao), vertexCount(other.vertexCount),
        colour(other.colour), position(other.position),
        angleMultiply(other.angleMultiply), angleAdd(other.angleAdd)
    {
        other.ibo = 0;
        other.vbo = 0;
        other.vao = 0;
    }

    ThreeDimensionalObject& operator= (ThreeDimensionalObject&& other) {
        if (this != &other) {
            ibo = other.ibo; other.ibo = 0;
            vbo = other.vbo; other.vbo = 0;
            vao = other.vao; other.vao = 0;
            vertexCount = other.vertexCount; other.vertexCount = 0;
            colour = other.colour; other.colour = vec3_t {};
            position = other.position; other.position = vec3_t {};
            angleMultiply = other.angleMultiply; other.angleMultiply = 1.0;
            angleAdd = other.angleAdd; other.angleAdd = 0.0;
        }
        return *this;
    }

    ~ThreeDimensionalObject() {
        if (ibo) glDeleteBuffers(1, &ibo);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
    }

    // Uniforms
    vec3_t colour;
    vec3_t position;
    // Angle offsets
    float angleMultiply;
    float angleAdd;

    void draw() const;
    void setupForDrawing(GearBlueprint bp);

    static const void* posOffset;
    static const void* nrmOffset;
    static const void* colOffset;
};

// Offsets of each attribute, in bytes
const void* ThreeDimensionalObject::posOffset = 0;
const void* ThreeDimensionalObject::nrmOffset = (void*)(3 * sizeof(float));
const void* ThreeDimensionalObject::colOffset = (void*)(6 * sizeof(float));

void ThreeDimensionalObject::draw() const {
    glm::mat4 model(1.0);
    model = glm::translate(
        model,
        glm::vec3(position.x, position.y, position.z)
    );
    model = glm::rotate(
        model,
        glm::radians(angleMultiply * angle + angleAdd),
        glm::vec3(0.0, 0.0, 1.0)
    );

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(uniformColour, colour.x, colour.y, colour.z);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void ThreeDimensionalObject::setupForDrawing(GearBlueprint bp) {

    // Stride (total number of bytes for all vertex attributes in an interleaved buffer)
    GLuint VBOstride = sizeof(GearVertex);

    GearVertex* gearBuffer = gear(vertexCount, bp);

    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);

    // Set up buffer and vertex array
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glGenVertexArrays(1, &vao);
    // Upload buffer
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GearVertex), gearBuffer, GL_STATIC_DRAW);
    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VBOstride, posOffset);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VBOstride, nrmOffset);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VBOstride, colOffset);
    glEnableVertexAttribArray(2);
    // Release bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Free buffer, since it has been uploaded already
    delete[] gearBuffer;
}

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
    glUniform3f(uniformLightPos, sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 5., sin(glfwGetTime()) * 10);
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

