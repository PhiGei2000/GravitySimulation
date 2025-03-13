#include "simulation.hpp"

template<>
bool Object<Mass>::collide(const Object<Mass>& first, const Object<Mass>& second) {
    int collisionDistance = first.attributes.radius + second.attributes.radius;

    const glm::vec2& distance = first.position - second.position;
    return glm::dot(distance, distance) <= collisionDistance * collisionDistance;
}