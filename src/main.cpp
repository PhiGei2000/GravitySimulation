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

constexpr float au = 149.6f;
constexpr float yr = 1.0f;
constexpr float d = yr / 365.0f;
constexpr float solar_mass = 1.988416E30f;
constexpr ValueType G = 4 * glm::pi<ValueType>() * glm::pi<ValueType>() * au * au * au / (yr * yr) / solar_mass;

struct Planet {
    float distance;
    float orbitalPeriod;
    float mass;
    float radius;
};

constexpr Planet planets[] = {
    Planet{0.387f, 0.2408f, 3.3011E23f, 5 * 0.3829f},   // mercury
    Planet{0.723f, 0.6152f, 4.8675E24f, 5 * 0.9499f},   // venus
    Planet{1.0f, 1.0f, 5.9722E24f, 5.0f},               // earth
    Planet{1.523f, 1.880f, 6.4171E24f, 5 * 0.533f},     // mars
    Planet{5.204f, 11.852f, 1.8982E27f, 2.5 * 11.209f}, // jupiter
    Planet{9.5826f, 29.448f, 5.6834E26f, 2.5 * 9.124f}, // saturn
    Planet{19.1913f, 84.021f, 8.681E25f, 5 * 4.007f},   // uranus
    Planet{30.07f, 164.8f, 1.02409E26f, 5 * 3.883f},    // neptune
};

Simulation<dimension, ValueType, Mass>
runSimulation() {
    Simulation<2, ValueType, Mass>::ForceCallback a = [](int index, const std::vector<Object<2, ValueType, Mass>>& objects) {
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

    objects.emplace_back(glm::vec2(0.0f), glm::vec2(0.0f), solar_mass, 10.0f); // sun
    for (const Planet planet : planets) {
        objects.emplace_back(glm::vec2{planet.distance * au, 0.0f}, glm::vec2{0.0f, -2 * au * planet.distance * glm::pi<float>() / (planet.orbitalPeriod * yr)}, planet.mass, planet.radius);
    }

    Simulation<dimension, ValueType, Mass> sim = Simulation<dimension, ValueType, Mass>(objects, a, onCollision, yr / 1000);

    for (int i = 0; i < 50000; i++) {
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
        int framesPerStep = 10;
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
                    index += framesPerStep;

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
