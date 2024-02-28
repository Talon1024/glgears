#include "3dobject.h"

#include "gear.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Offsets of each attribute, in bytes
const void* ThreeDimensionalObject::posOffset = 0;
const void* ThreeDimensionalObject::nrmOffset = (void*)(3 * sizeof(float));
const void* ThreeDimensionalObject::colOffset = (void*)(6 * sizeof(float));

extern GLfloat angle;
extern GLuint uniformModel, uniformColour;

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

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void ThreeDimensionalObject::setupForDrawing(GearBlueprint bp) {

    // Stride (total number of bytes for all vertex attributes in an interleaved buffer)
    GLuint VBOstride = sizeof(GearVertex);

    GearBuffers gearBuffers = gear(bp);
    indexCount = gearBuffers.indexCount();

    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);

    // Set up buffer and vertex array
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glGenVertexArrays(1, &vao);
    // Set up vertex array
    glBindVertexArray(vao);
    // Upload index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        gearBuffers.indexBuffer.size() * sizeof(IndexTriangle),
        gearBuffers.indexBuffer.data(),
        GL_STATIC_DRAW
    );
    // Upload vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        gearBuffers.vertexBuffer.size() * sizeof(GearVertex),
        gearBuffers.vertexBuffer.data(),
        GL_STATIC_DRAW
    );
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
    // delete[] gearBuffer;
}
