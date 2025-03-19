#pragma once
#include <GL/glew.h>

template<typename T>
int glType();

template<>
inline int glType<double>() {
    return GL_DOUBLE;
}

template<>
inline int glType<float>() {
    return GL_FLOAT;
}

template<int dim, typename T>
inline void setupVertexAttributes() {
    switch (glType<T>()) {
        case GL_DOUBLE:
            glVertexAttribLPointer(0, dim, glType<T>(), dim * sizeof(T), (void*)0);
            break;
        default:
            glVertexAttribPointer(0,
                                  dim,
                                  glType<T>(),
                                  GL_FALSE,
                                  dim * sizeof(T),
                                  (void*)0);
            break;
    }

    glEnableVertexAttribArray(0);
}
