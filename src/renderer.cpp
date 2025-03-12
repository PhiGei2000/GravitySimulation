#include "renderer.hpp"

#include "simulation.hpp"
#include "window.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <string>

std::vector<glm::vec2> Renderer::genCircle(const glm::vec2& center, float radius, unsigned int verticesCount) {
    std::vector<glm::vec2> vertices = {center};

    float anglePerVertex = 2.0f * glm::pi<float>() / verticesCount;
    for (int i = 0; i < verticesCount; i++) {
        vertices.push_back(center + radius * glm::vec2{glm::cos(anglePerVertex * i), glm::sin(anglePerVertex * i)});
    }

    return vertices;
}

Renderer::Renderer(const Window* window)
    : window(window) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // shaders
    const char* vertexSource =
        "#version 450\n"
        "layout(location = 0) in vec2 position;\n"

        "uniform mat4 projection;"

        "void main() {\n"
        "   gl_Position = projection * vec4(position.x, position.y, 0.0, 1.0);\n"
        "}\0";

    const char* fragmentSource =
        "#version 450\n"
        "out vec4 fragColor;\n"

        "void main() {\n"
        "   fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\0";

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);

        throw std::runtime_error(infoLog);
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);

        throw std::runtime_error(infoLog);
    }

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);

        throw std::runtime_error(infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Renderer::draw(const SimulationState<Mass>& state, int stepIndex) const {
    // update buffers
    std::vector<glm::vec2> vertices;
    std::vector<unsigned int> indices;

    int objectsCount = state.objectsCount(stepIndex);
    vertices.reserve(17 * objectsCount);
    indices.reserve(17 * 3 * objectsCount);

    unsigned int offset = 0;

    // for (int i = 0; i < objectsCount; i++) {
    for (auto it = state.begin(stepIndex); it != state.end(stepIndex); it++) {
        const glm::vec2 position = it->getPosition(stepIndex).value();

        const std::vector<glm::vec2>& circle = genCircle(position, it->attributes.radius);
        vertices.insert(vertices.end(), circle.begin(), circle.end());

        for (int j = 0; j < circle.size() - 1; j++) {
            unsigned int current = j + 1;
            unsigned int next = (j + 1) % (circle.size() - 1) + 1;

            indices.push_back(offset);
            indices.push_back(offset + current);
            indices.push_back(offset + next);
        }

        offset += circle.size();
    }

    int width = window->getWidth();
    int height = window->getHeight();
    const glm::mat4 projection = glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram);

    int projectionUniform = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    window->draw();
}