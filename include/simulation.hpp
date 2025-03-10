#pragma once
#include <glm/glm.hpp>

#include <vector>

template<typename T>
struct Object {
    T attributes;

    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;
};

struct Mass {
    float mass;
    float radius;
};

class Simulation {
  public:
    std::vector<Object<Mass>> objects;
    float stepSize = 1.0f;

    void step();

  private:
    glm::vec2 calculateAcceleration(int massIndex, int timeStep = -1) const;
};

template<typename T>
class SimulationState {
    const Simulation& simulation;

  public:
    inline SimulationState(const Simulation& simulation)
        : simulation(simulation) {
    }

    inline int objectsCount() const {
        return simulation.objects.size();
    }

    inline const T& getAttributes(int index) const {
        return simulation.objects[index].attributes;
    }

    inline const glm::vec2& getPosition(int index, int stepIndex = -1) const {
        switch (stepIndex) {
            case -1:
                return simulation.objects[index].positions.back();
            default:
                return simulation.objects[index].positions[stepIndex];
        }
    }

    inline const glm::vec2& getVelocity(int index, int stepIndex = -1) const {
        switch (stepIndex) {
            case -1:
                return simulation.objects[index].velocities.back();
            default:
                return simulation.objects[index].velocities[stepIndex];
        }
    }
};