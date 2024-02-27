#pragma once

#include "glad.h"
#include "vector.h"
#include "gear.h"

struct ThreeDimensionalObject {
    private:
    // OpenGL resource handles
    GLuint ibo;
    GLuint vbo;
    GLuint vao;
    // Used for rendering a complete object
    GLuint vertexCount;
    GLuint indexCount;

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
        indexCount(other.indexCount), colour(other.colour),
        position(other.position), angleMultiply(other.angleMultiply),
        angleAdd(other.angleAdd)
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
            indexCount = other.indexCount; other.indexCount = 0;
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
