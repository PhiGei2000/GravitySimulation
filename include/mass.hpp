#pragma once
#include "object.hpp"

struct Mass {
    float mass;
    float radius;

    template<typename TValue>
    inline unsigned int getGeometry(const glm::vec<2, TValue>& position,
                                    std::vector<glm::vec<2, TValue>>& vertices,
                                    std::vector<unsigned int>& indices,
                                    unsigned int indexOffset) const {
        static constexpr unsigned int verticesCount = 16;

        vertices.push_back(position);

        TValue anglePerVertex = static_cast<TValue>(2.0) * glm::pi<TValue>() / verticesCount;
        for (int i = 0; i < verticesCount; i++) {
            vertices.push_back(position + static_cast<TValue>(radius) * glm::vec<2, TValue>(glm::cos(anglePerVertex * i), glm::sin(anglePerVertex * i)));

            indices.push_back(indexOffset);
            indices.push_back(indexOffset + i + 1);
            indices.push_back(indexOffset + ((i + 1) % verticesCount) + 1);
        }

        return verticesCount + 1;
    }
};

template<int dim, typename TValue>
inline bool collide(const Object<dim, TValue, Mass>& first, const Object<dim, TValue, Mass>& second) {
    const typename Object<dim, TValue, Mass>::TVec& distance = first.position - second.position;

    float collisionDistance = first.attributes.radius + second.attributes.radius;
    return glm::dot(distance, distance) <= static_cast<TValue>(collisionDistance);
}
