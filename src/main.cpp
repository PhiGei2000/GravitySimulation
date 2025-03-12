#include "renderer.hpp"
#include "simulation.hpp"
#include "window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main(int, char**) {
    Window window;
    try {
        window.init();

        Renderer renderer(&window);

        Simulation<Mass>::ForceCallback a = [](int index, int timeStep, const std::vector<Object<Mass>>& objects) {
            static constexpr float G = 6.6743E-11;
            glm::vec2 acceleration = glm::vec2(0.0f);

            timeStep = timeStep == -1 ? objects[index].positions.size() - 1 : timeStep;
            return objects[index].getPosition(timeStep).and_then([&](const glm::vec2& pos1) {
                                                           for (int j = 0; j < objects.size(); j++) {
                                                               if (j == index || !objects[j].exists(timeStep))
                                                                   continue;

                                                               auto p2 = objects[j].getPosition(timeStep);
                                                               if (!p2.has_value()) {
                                                                   continue;
                                                               }

                                                               const glm::vec2& distanceVec = p2.value() - pos1;
                                                               const float distance = glm::sqrt(glm::dot(distanceVec, distanceVec));

                                                               acceleration += G * objects[j].attributes.mass / glm::pow(distance, 3.0f) * distanceVec;
                                                           }

                                                           return std::optional(acceleration);
                                                       })
                .value_or(glm::vec2(0.0f));
        };

        Simulation<Mass>::CollisionCallback onCollision = [](int index, const std::vector<int>& collisions, int time, std::vector<Object<Mass>>& objects) {
            float mass = objects[index].attributes.mass;
            float radiusSquare = objects[index].attributes.radius * objects[index].attributes.radius;
            glm::vec2 pos = objects[index].attributes.mass * objects[index].getPosition(time).value();
            glm::vec2 vel = objects[index].attributes.mass * objects[index].getVelocity(time).value();

            // objects[index].attributes.mass = 0;
            objects[index].destructionTime = time;

            for (int i : collisions) {
                mass += objects[i].attributes.mass;
                radiusSquare += objects[i].attributes.radius * objects[i].attributes.radius;
                pos += objects[i].attributes.mass * objects[i].getPosition(time).value();
                vel += objects[i].attributes.mass * objects[i].getVelocity(time).value();

                // objects[i].attributes.mass = 0;
                objects[i].destructionTime = time;
            }

            objects.emplace_back(pos / mass, vel / mass, mass, glm::sqrt(radiusSquare)).creationTime = time;
        };

        std::vector<Object<Mass>> objects = {
            {glm::vec2{0.0f}, glm::vec2{0.0f}, 1E12f, 10.0f},
            {glm::vec2{100.0f, 0.0f}, glm::vec2{0.0f, 1.0f}, 1E11f, 5.0f},
            {glm::vec2{-150.0f, 0.0f}, glm::vec2{0.0f, -0.5f}, 1E11f, 5.0f},
        };

        Simulation<Mass> sim = Simulation<Mass>(objects, a, onCollision);

        for (int i = 0; i < 10000; i++) {
            sim.step();
        }

        SimulationState<Mass> state = SimulationState<Mass>(sim);
        int index = 0;

        auto frameDuration = 5ms;
        float timeSinceLastFrame = 0;
        float lastTime = 0;
        bool pause = true;

        while (!window.shouldClose()) {
            if (index < sim.endTime()) {
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
