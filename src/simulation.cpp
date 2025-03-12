#include "simulation.hpp"

template<>
bool Object<Mass>::collide(const Object<Mass>& first, const Object<Mass>& second, int timeStep) {
    return first.getPosition(timeStep).and_then([&](const glm::vec2& pos1) {
                                          return second.getPosition(timeStep).and_then([&](const glm::vec2& pos2) {
                                              int collisionDistance = first.attributes.radius + second.attributes.radius;

                                              const glm::vec2& distance = pos1 - pos2;
                                              return std::optional(glm::dot(distance, distance) <= collisionDistance * collisionDistance);
                                          });
                                      })
        .value_or(false);
}