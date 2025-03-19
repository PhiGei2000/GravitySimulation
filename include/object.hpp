#pragma once
#include <concepts>
#include <vector>

#include <glm/glm.hpp>

template<typename T, int dim, typename TValue>
concept Drawable = requires(const T& attributes,
                            const glm::vec<dim, TValue>& position,
                            std::vector<glm::vec<dim, TValue>>& vertices,
                            std::vector<unsigned int>& indices,
                            unsigned int indexOffset) {
{ attributes.getGeometry(position, vertices, indices, indexOffset) } -> std::same_as<unsigned int>;
};

template<typename T, int dim, typename TValue>
concept ObjectAttributes = Drawable<T, dim, TValue>;

template<int dim, typename TValue, ObjectAttributes<dim, TValue> T>
class Simulation;

template<int dim, typename TValue, ObjectAttributes<dim, TValue> T>
struct Object {
  private:
    int id = -1;
    friend class Simulation<dim, TValue, T>;

  public:
    using TVec = glm::vec<dim, TValue>;
    using type = Object<dim, TValue, T>;

    T attributes;

    TVec position;
    TVec velocity;

    template<typename... TArgs>
    inline Object(const TVec& initialPosition, const TVec& initialVelocity, const TArgs&... args)
        : position(initialPosition), velocity(initialVelocity), attributes{args...} {
    }

    inline int getID() const {
        return id;
    }

    struct Trajectory {
        int objectID;
        T attributes;

        std::vector<TVec> positions;
        std::vector<TVec> velocities;
    };

    inline unsigned int getGeometry(std::vector<TVec>& vertices, std::vector<unsigned int>& indices, unsigned int indexOffset = 0) const {
        return attributes.getGeometry(position, vertices, indices, indexOffset);
    }
};

template<typename T, int dim, typename TValue>
bool collide(const Object<dim, TValue, T>& first, const Object<dim, TValue, T>& second);