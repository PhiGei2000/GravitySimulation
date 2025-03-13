#pragma once
#include "simulation.hpp"

#include <vector>

#include <glm/glm.hpp>

struct Window;

class Renderer {
  private:
    const Window* window;
    unsigned int shaderProgram;
    unsigned int vbo, vao, ebo;

    static std::vector<glm::vec2> genCircle(const glm::vec2& center, float radius = 30.0f, unsigned int verticesCount = 16);

  public:
    Renderer(const Window* window);

    template<typename T>
    void draw(const std::vector<Object<T>>& state) const;
};