#pragma once
#include <glm/glm.hpp>

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

template<typename T>
struct Object {
    T attributes;

    glm::vec2 position;
    glm::vec2 velocity;

    template<typename... TArgs>
    inline Object(const glm::vec2& initialPosition, const glm::vec2& initialVelocity, const TArgs&... args)
        : position(initialPosition), velocity(initialVelocity), attributes{args...} {
    }

    static bool collide(const Object<T>& first, const Object<T>& second);
};

struct Mass {
    float mass;
    float radius;
};

template<typename T>
class Simulation {
  public:
    using CollisionCallback = std::function<Object<T>(int, const std::vector<int>&, const std::vector<Object<T>>&)>;
    using ForceCallback = std::function<glm::vec2(int, const std::vector<Object<T>>&)>;
    using State = std::vector<Object<T>>;

    std::vector<State> states;
    float stepSize;

    inline Simulation(const State& initialState, const ForceCallback& a, float stepSize = 1.0f)
        : states({initialState}), a(a), stepSize(stepSize) {
    }

    inline Simulation(const State& initialState, const ForceCallback& a, const CollisionCallback& onCollision, float stepSize = 1.0f)
        : Simulation(initialState, a, stepSize) {
        this->onCollision = onCollision;
        handleCollisions = true;
    }

    inline void step() {
        std::vector<glm::vec2> accelerations;

        std::vector<Object<T>> next = states.back();
        // velocity verlet
        for (int i = 0; i < states[currentTimeStep].size(); i++) {
            const glm::vec2& acceleration = a(i, next);

            next[i].position = states[currentTimeStep][i].position + states[currentTimeStep][i].velocity * stepSize + acceleration * stepSize * stepSize / 2.0f;
            accelerations.push_back(acceleration);
        }

        for (int i = 0; i < next.size(); i++) {
            const glm::vec2& nextAcceleration = a(i, next);
            next[i].velocity = next[i].velocity + (accelerations[i] + nextAcceleration) * stepSize / 2.0f;
        }

        currentTimeStep++;
        if (handleCollisions) {
            // check for collisions
            std::map<int, std::vector<int>> collisions;
            std::set<int> objectsToRemove;

            for (int i = 0; i < next.size(); i++) {
                for (int j = 0; j < i; j++) {
                    if (Object<T>::collide(next[i], next[j])) {
                        collisions[i].push_back(j);
                        objectsToRemove.insert({i, j});
                    }
                }
            }

            for (const auto& [index, colls] : collisions) {
                next.emplace_back(onCollision.value()(index, colls, next));
            }

            for (auto it = objectsToRemove.rbegin(); it != objectsToRemove.rend(); it++) {
                next.erase(next.begin() + *it);
            }
        }

        states.push_back(next);
    }

    inline int endTime() const {
        return currentTimeStep;
    }

    inline const State& getState(int time) const {
        return states[time];
    }

  private:
    ForceCallback a;

    std::optional<CollisionCallback> onCollision = std::nullopt;
    bool handleCollisions = false;

    int currentTimeStep = 0;
};