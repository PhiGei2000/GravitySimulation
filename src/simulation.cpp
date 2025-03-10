#include "simulation.hpp"

void Simulation::step() {
    int stepNumber = objects.front().positions.size();
    std::vector<glm::vec2> accelerations;

    // velocity verlet
    for (int i = 0; i < objects.size(); i++) {
        const glm::vec2& acceleration = calculateAcceleration(i, stepNumber - 1);

        objects[i].positions.push_back(objects[i].positions[stepNumber - 1] - objects[i].velocities[stepNumber - 1] * stepSize + acceleration * stepSize * stepSize / 2.0f);
        accelerations.push_back(acceleration);
    }

    for (int i = 0; i < objects.size(); i++) {
        const glm::vec2& nextAcceleration = calculateAcceleration(i, stepNumber);
        objects[i].velocities.push_back(objects[i].velocities[stepNumber - 1] + (accelerations[i] + nextAcceleration) * stepSize / 2.0f);
    }
}

glm::vec2 Simulation::calculateAcceleration(int mass, int timeStep) const {
    static constexpr float G = 6.6743E-11;
    glm::vec2 acceleration = glm::vec2(0.0f);

    timeStep = timeStep == -1 ? objects[mass].positions.size() - 1 : timeStep;

    for (int j = 0; j < objects.size(); j++) {
        if (j == mass)
            continue;

        const glm::vec2& distanceVec = objects[j].positions[timeStep] - objects[mass].positions[timeStep];
        const float distance = glm::sqrt(glm::dot(distance, distance));

        acceleration += -G * objects[j].attributes.mass / glm::pow(distance, 3.0f) * distanceVec;
    }

    return acceleration;
}
