#pragma once
#include "glType.hpp"
#include "shaders.hpp"
#include "simulation.hpp"
#include "window.hpp"

#include <stdexcept>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

template<int dim, typename TValue>
class Renderer {
  protected:
    Window* window;
    unsigned int shaderProgram;
    unsigned int vbo, vao, ebo;
    unsigned int drawCount;

    inline void setupShaders() {
        const char* vertexSource = getVertexShader<dim, TValue>();

        const char* fragmentSource = getFragmentShader();

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
    }

    inline void bindBuffers() const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    inline void unbindBuffers() const {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void setupBuffers() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        bindBuffers();

        setupVertexAttributes<dim, TValue>();

        unbindBuffers();
    }

  public:
    inline Renderer(Window* window)
        : window(window) {
        setupBuffers();

        setupShaders();

        auto scrollCallback = [](const Window* window, double xOffset, double yOffset) {

        };

        window->addScrollCallback(scrollCallback);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    inline void draw() const {
        int width = window->getWidth();
        int height = window->getHeight();
        const glm::mat<4, 4, TValue> projection = glm::ortho<TValue>(-width / 2, width / 2, -height / 2, height / 2);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glUseProgram(shaderProgram);

        uploadMatrix(shaderProgram, "projection", projection);

        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        window->draw();
    }

    template<ObjectAttributes<dim, TValue> T>
    inline void updateBuffers(const std::vector<Object<dim, TValue, T>>& state) {
        std::vector<glm::vec<dim, TValue>> vertices;
        std::vector<unsigned int> indices;
        unsigned int indexOffset = 0;

        for (const auto& object : state) {
            indexOffset += object.getGeometry(vertices, indices, indexOffset);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(typename Object<dim, TValue, T>::TVec) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
        drawCount = indices.size();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};
