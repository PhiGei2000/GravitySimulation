#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

template<int dimension, typename TValue>
class Camera {
  protected:
    glm::ivec2 screenSize;
    float zoom = 1.0f;

  public:
    using MatType = glm::mat<4, 4, TValue>;

    virtual MatType getProjectionMatrix() const = 0;

    inline void updateScreenSize(int width, int height) {
        screenSize = glm::ivec2(width, height);
    }

    inline void setZoom(float factor) {
        zoom = factor;
    }

    inline float getZoom() const {
        return zoom;
    }
};

template<typename TValue>
class Camera<2, TValue> {
  protected:
    glm::ivec2 screenSize;
    float zoom = 1.0f;

  public:
    using MatType = glm::mat<4, 4, TValue>;
    glm::vec<2, TValue> center = glm::vec<2, TValue>(static_cast<TValue>(0.0));

    virtual MatType getProjectionMatrix() const {
        TValue left, right, bottom, top;
        left = -static_cast<TValue>(screenSize.x / 2.0) / static_cast<TValue>(zoom);
        right = static_cast<TValue>(screenSize.x / 2.0) / static_cast<TValue>(zoom);
        bottom = -static_cast<TValue>(screenSize.y / 2.0) / static_cast<TValue>(zoom);
        top = static_cast<TValue>(screenSize.y / 2.0) / static_cast<TValue>(zoom);

        return glm::translate(glm::ortho<TValue>(left, right, bottom, top), glm::vec<3, TValue>(center.x, center.y, static_cast<TValue>(0.0)));
    }

    inline void updateScreenSize(int width, int height) {
        screenSize = glm::ivec2(width, height);
    }

    inline void setZoom(float factor) {
        zoom = factor;
    }

    inline float getZoom() const {
        return zoom;
    }
};

template<typename TValue>
using Camera2D = Camera<2, TValue>;