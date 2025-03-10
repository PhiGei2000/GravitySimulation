#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <map>

enum KeyActions {
    PRESSED,
    RELEASED
};

struct Window {
  private:
    GLFWwindow* window;
    int width = 800, height = 600;

    std::map<int, KeyActions> keyActions;

  public:
    void init();
    bool shouldClose() const;
    void close();

    void draw() const;

    int getWidth() const;
    int getHeight() const;

    bool wasPressed(int key);

    static void sizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
