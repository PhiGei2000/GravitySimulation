#include "renderer.hpp"
#include "simulation.hpp"
#include "window.hpp"

#include "glm/glm.hpp"

#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main(int, char**) {
    Window window;
    try {
        window.init();

        Renderer renderer(&window);
        std::vector<glm::vec2> positions = {
            glm::vec2{0.0f, 0.0f},
            glm::vec2{100.0f, 0.0f},
        };

        Simulation sim = {std::vector<Object<Mass>>{
                              {{1E13, 10.0f}, std::vector<glm::vec2>{{0.0f, 0.0f}}, std::vector<glm::vec2>{{0.0f, 0.0f}}},
                              {{1E12, 10.0f}, std::vector<glm::vec2>{{100.0f, 0.0f}}, std::vector<glm::vec2>{{0.0f, 3.0f}}},
                              {{1E12, 10.0f}, std::vector<glm::vec2>{{-200.0f, 0.0f}}, std::vector<glm::vec2>{{0.0f, -2.5f}}},
                          },
                          1.0f};

        for (int i = 0; i < 100000; i++) {
            sim.step();
        }

        SimulationState<Mass> state = SimulationState<Mass>(sim);
        int index = 0;

        auto frameDuration = 25ms;
        float timeSinceLastFrame = 0;
        float lastTime = 0;
        bool pause = true;

        while (!window.shouldClose()) {
            if (index < sim.objects.front().positions.size()) {
                renderer.draw(state, index);
            }
            else {
                renderer.draw(state);
            }

            float time = glfwGetTime();
            timeSinceLastFrame += time - lastTime;
            lastTime = time;
            if (std::chrono::duration<float, std::milli>(timeSinceLastFrame * 1000.0f) > frameDuration) {
                if (!pause) {
                    index++;
                }
                timeSinceLastFrame = 0;
            }

            if (window.wasPressed(GLFW_KEY_B)) {
                index = 0;
                timeSinceLastFrame = 0;
            }
            else if (window.wasPressed(GLFW_KEY_P)) {
                pause = !pause;
            }
        }
    }
    catch (std::runtime_error e) {
        std::cout << e.what() << std::endl;
    }

    window.close();
}
