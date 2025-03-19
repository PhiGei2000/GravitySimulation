#include "renderer.hpp"
#include "simulation.hpp"
#include "window.hpp"

#include "mass.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <thread>

using namespace std::chrono_literals;

using ValueType = double;
const int dimension = 2;

using Vec = glm::vec<dimension, ValueType>;

Simulation<dimension, ValueType, Mass> runSimulation() {
    Simulation<2, ValueType, Mass>::ForceCallback a = [](int index, const std::vector<Object<2, ValueType, Mass>>& objects) {
        static constexpr ValueType G = 6.6743E-11;
        Vec acceleration = Vec(0.0);

        for (int j = 0; j < objects.size(); j++) {
            if (j == index)
                continue;

            const Vec& distanceVec = objects[j].position - objects[index].position;
            const ValueType distance = glm::sqrt(glm::dot(distanceVec, distanceVec));

            acceleration += G * objects[j].attributes.mass / glm::pow(distance, 3.0f) * distanceVec;
        }

        return acceleration;
    };

    Simulation<dimension, ValueType, Mass>::CollisionCallback onCollision = [](int index, const std::vector<int>& collisions, const std::vector<Object<dimension, ValueType, Mass>>& objects) {
        float mass = objects[index].attributes.mass;
        float radiusSquare = objects[index].attributes.radius * objects[index].attributes.radius;
        Vec pos = static_cast<ValueType>(objects[index].attributes.mass) * objects[index].position;
        Vec vel = static_cast<ValueType>(objects[index].attributes.mass) * objects[index].velocity;

        for (int i : collisions) {
            mass += objects[i].attributes.mass;
            radiusSquare += objects[i].attributes.radius * objects[i].attributes.radius;
            pos += static_cast<ValueType>(objects[i].attributes.mass) * objects[i].position;
            vel += static_cast<ValueType>(objects[i].attributes.mass) * objects[i].velocity;
        }

        return Object<dimension, ValueType, Mass>(pos / static_cast<ValueType>(mass), vel / static_cast<ValueType>(mass), mass, glm::sqrt(radiusSquare));
    };

    std::vector<Object<dimension, ValueType, Mass>> objects;

    float density = 1E10f;
    for (int i = 0; i < 100; i++) {
        float radius = rand() / static_cast<float>(RAND_MAX) * 5.0f + 1.0f;
        float mass = radius * radius * density;

        const Vec& position = {
            rand() / static_cast<float>(RAND_MAX) * 800.0 - 400.0,
            rand() / static_cast<float>(RAND_MAX) * 800.0 - 400.0,
        };

        const Vec& velocity = static_cast<ValueType>(2.5) * glm::normalize(Vec{position.y, -position.x});

        objects.emplace_back(position, velocity, mass, radius);
    }

    Simulation<dimension, ValueType, Mass> sim = Simulation<dimension, ValueType, Mass>(objects, a, onCollision);

    for (int i = 0; i < 5000; i++) {
        sim.step();
    }

    return sim;
}

int main(int, char**) {
    const Simulation<dimension, ValueType, Mass>& sim = runSimulation();

    Window window;
    try {
        window.init();

        Renderer<dimension, ValueType> renderer(&window);
        renderer.updateBuffers(sim.states.front());

        int index = 0;
        auto frameDuration = 5ms;
        float timeSinceLastFrame = 0;
        float lastTime = 0;
        bool pause = true;

        while (!window.shouldClose()) {
            renderer.draw();

            float time = glfwGetTime();
            timeSinceLastFrame += time - lastTime;
            lastTime = time;
            if (std::chrono::duration<float, std::milli>(timeSinceLastFrame * 1000.0f) > frameDuration) {
                if (!pause) {
                    index++;

                    if (index < sim.endTime()) {
                        renderer.updateBuffers(sim.getState(index));
                    }
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
