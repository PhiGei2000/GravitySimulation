#pragma once
#include <glm/glm.hpp>

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

template<typename T, typename TVec>
class Simulation;

template<typename T, typename TVec = glm::vec2>
struct Object {
  private:
    int id = -1;
    friend class Simulation<T, TVec>;

  public:
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

    static bool collide(const Object<T>& first, const Object<T>& second);
};

struct Mass {
    float mass;
    float radius;
};

template<typename T, typename TVec = glm::vec2>
struct Trajectory {
    int objectID;
    T attributes;

    std::vector<TVec> positions;
    std::vector<TVec> velocities;
};

template<typename T, typename TVec>
class Simulation {
  private:
    int objectID = 0;

  public:
    using CollisionCallback = std::function<Object<T, TVec>(int, const std::vector<int>&, const std::vector<Object<T, TVec>>&)>;
    using ForceCallback = std::function<TVec(int, const std::vector<Object<T, TVec>>&)>;
    using State = std::vector<Object<T, TVec>>;

    std::vector<State> states;
    float stepSize;

    inline Simulation(const State& initialState, const ForceCallback& a, float stepSize = 1.0f)
        : states({initialState}), a(a), stepSize(stepSize) {
        for (auto& object : states.front()) {
            object.id = objectID++;
        }
    }

    inline Simulation(const State& initialState, const ForceCallback& a, const CollisionCallback& onCollision, float stepSize = 1.0f)
        : Simulation(initialState, a, stepSize) {
        this->onCollision = onCollision;
        handleCollisions = true;
    }

    template<typename... TArgs>
    inline Object<T, TVec>& createObject(int time, const TVec& position, const TVec& velocity, const TArgs&... args) {
        Object<T, TVec>& object = states[time].emplaceBack(position, velocity, args...);
        object.id = objectID++;

        return object;
    }

    inline void step() {
        std::vector<TVec> accelerations;

        State next = states.back();
        // velocity verlet
        for (int i = 0; i < states[currentTimeStep].size(); i++) {
            const TVec& acceleration = a(i, next);

            next[i].position = states[currentTimeStep][i].position + states[currentTimeStep][i].velocity * stepSize + acceleration * stepSize * stepSize / 2.0f;
            accelerations.push_back(acceleration);
        }

        for (int i = 0; i < next.size(); i++) {
            const TVec& nextAcceleration = a(i, next);
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

    inline std::vector<Trajectory<T, TVec>> getTrajectories() const {
        std::map<int, Trajectory<T, TVec>> trajectories;

        for (const auto& state : states) {
            for (const auto& object : state) {
                trajectories[object.id].positions.push_back(object.position);
                trajectories[object.id].velocities.push_back(object.velocity);
            }
        }

        std::vector<Trajectory<TVec>> result;
        result.reserve(trajectories.size());

        for (auto& [id, trajectory] : trajectories) {
            trajectory.objectId = id;
            result.push_back(trajectory);
        }

        return result;
    }

  private:
    ForceCallback a;

    std::optional<CollisionCallback> onCollision = std::nullopt;
    bool handleCollisions = false;

    int currentTimeStep = 0;
};