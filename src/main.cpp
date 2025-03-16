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

        Simulation<Mass, glm::vec2>::ForceCallback a = [](int index, const std::vector<Object<Mass>>& objects) {
            static constexpr float G = 6.6743E-11;
            glm::vec2 acceleration = glm::vec2(0.0f);

            for (int j = 0; j < objects.size(); j++) {
                if (j == index)
                    continue;

                const glm::vec2& distanceVec = objects[j].position - objects[index].position;
                const float distance = glm::sqrt(glm::dot(distanceVec, distanceVec));

                acceleration += G * objects[j].attributes.mass / glm::pow(distance, 3.0f) * distanceVec;
            }

            return acceleration;
        };

        Simulation<Mass, glm::vec2>::CollisionCallback onCollision = [](int index, const std::vector<int>& collisions, const std::vector<Object<Mass>>& objects) {
            float mass = objects[index].attributes.mass;
            float radiusSquare = objects[index].attributes.radius * objects[index].attributes.radius;
            glm::vec2 pos = objects[index].attributes.mass * objects[index].position;
            glm::vec2 vel = objects[index].attributes.mass * objects[index].velocity;

            for (int i : collisions) {
                mass += objects[i].attributes.mass;
                radiusSquare += objects[i].attributes.radius * objects[i].attributes.radius;
                pos += objects[i].attributes.mass * objects[i].position;
                vel += objects[i].attributes.mass * objects[i].velocity;
            }

            return Object<Mass>(pos / mass, vel / mass, mass, glm::sqrt(radiusSquare));
        };

        std::vector<Object<Mass>> objects;

        float density = 1E10f;
        for (int i = 0; i < 1000; i++) {
            float radius = rand() / static_cast<float>(RAND_MAX) * 5.0f + 1.0f;
            float mass = radius * radius * density;

            const glm::vec2& position = {
                rand() / static_cast<float>(RAND_MAX) * 800.0f - 400.0f,
                rand() / static_cast<float>(RAND_MAX) * 800.0f - 400.0f,
            };

            const glm::vec2& velocity = 4.5f * glm::normalize(glm::vec2{position.y, -position.x});

            objects.emplace_back(position, velocity, mass, radius);
        }

        Simulation<Mass, glm::vec2> sim = Simulation<Mass, glm::vec2>(objects, a, onCollision);

        for (int i = 0; i < 10000; i++) {
            sim.step();
        }

        int index = 0;
        auto frameDuration = 5ms;
        float timeSinceLastFrame = 0;
        float lastTime = 0;
        bool pause = true;

        while (!window.shouldClose()) {
            if (index < sim.endTime()) {
                renderer.draw(sim.getState(index));
            }
            else {
                renderer.draw(sim.states.back());
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
