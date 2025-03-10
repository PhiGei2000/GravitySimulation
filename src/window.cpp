#include "window.hpp"

#include <iostream>
#include <stdexcept>

void Window::init() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    window = glfwCreateWindow(width, height, "Gravity simulation", nullptr, nullptr);
    if (window == NULL) {
        glfwTerminate();

        throw std::runtime_error("Failed to create window");
    }

    glfwSetWindowSizeCallback(window, sizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    glfwSetWindowUserPointer(window, this);
    glfwMakeContextCurrent(window);

    glewInit();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::close() {
    glfwTerminate();
}

void Window::draw() const {
    glfwPollEvents();
    glfwSwapBuffers(window);
}

bool Window::wasPressed(int key) {
    bool pressed = false;
    try {
        pressed = keyActions.at(key) == KeyActions::PRESSED;
    }
    catch (std::out_of_range) {
    }

    keyActions[key] = KeyActions::RELEASED;
    return pressed;
}

int Window::getWidth() const {
    return width;
}

int Window::getHeight() const {
    return height;
}

void Window::sizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    Window* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));
    windowPtr->width = width;
    windowPtr->height = height;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));

    switch (action) {
        case GLFW_PRESS:
            windowPtr->keyActions[key] = KeyActions::PRESSED;
            break;
        case GLFW_RELEASE:
            windowPtr->keyActions[key] = KeyActions::RELEASED;
            break;
    }
}