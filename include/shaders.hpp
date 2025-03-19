#pragma once
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

template<int dim, typename TValue>
constexpr const char* getVertexShader();

template<>
inline constexpr const char* getVertexShader<2, float>() {
    return "#version 450\n"
           "layout(location = 0) in vec2 position;\n"

           "uniform mat4 projection;"

           "void main() {\n"
           "   gl_Position = projection * vec4(position, 0.0, 1.0);\n"
           "}\0";
}

template<>
inline constexpr const char* getVertexShader<2, double>() {
    return "#version 450\n"
           "layout(location = 0) in dvec2 position;\n"

           "uniform dmat4 projection;"

           "void main() {\n"
           "   gl_Position = vec4(projection * dvec4(position, 0.0, 1.0));\n"
           "}\0";
}

constexpr const char* getFragmentShader() {
    return "#version 450\n"
           "out vec4 fragColor;\n"

           "void main() {\n"
           "   fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
           "}\0";
}

template<typename TValue>
void uploadMatrix(unsigned int shader, const std::string& name, const glm::mat<4, 4, TValue>& mat);

template<>
inline void uploadMatrix(unsigned int shader, const std::string& name, const glm::mat4& mat) {
    int location = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

template<>
inline void uploadMatrix(unsigned int shader, const std::string& name, const glm::dmat4& mat) {
    int location = glGetUniformLocation(shader, "projection");
    glUniformMatrix4dv(location, 1, GL_FALSE, glm::value_ptr(mat));
}
