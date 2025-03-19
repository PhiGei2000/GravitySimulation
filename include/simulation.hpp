#pragma once
#include "object.hpp"

#include <functional>
#include <map>
#include <optional>
#include <set>

template<int dim, typename TValue, ObjectAttributes<dim, TValue> T>
class Simulation {
  private:
    int objectID = 0;

  public:
    using Object = Object<dim, TValue, T>;

    using State = std::vector<Object>;
    using Trajectory = Object::Trajectory;
    using TVec = Object::TVec;

    using CollisionCallback = std::function<Object(int, const std::vector<int>&, const std::vector<Object>&)>;
    using ForceCallback = std::function<TVec(int, const std::vector<Object>&)>;

    std::vector<State> states;
    TValue stepSize;

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
    inline Object& createObject(int time, const TVec& position, const TVec& velocity, const TArgs&... args) {
        Object& object = states[time].emplaceBack(position, velocity, args...);
        object.id = objectID++;

        return object;
    }

    inline void step() {
        std::vector<TVec> accelerations;

        State next = states.back();
        // velocity verlet
        for (int i = 0; i < states[currentTimeStep].size(); i++) {
            const TVec& acceleration = a(i, next);

            next[i].position = states[currentTimeStep][i].position + states[currentTimeStep][i].velocity * stepSize + acceleration * stepSize * stepSize / static_cast<TValue>(2);
            accelerations.push_back(acceleration);
        }

        for (int i = 0; i < next.size(); i++) {
            const TVec& nextAcceleration = a(i, next);
            next[i].velocity = next[i].velocity + (accelerations[i] + nextAcceleration) * stepSize / static_cast<TValue>(2);
        }

        currentTimeStep++;
        if (handleCollisions) {
            // check for collisions
            std::map<int, std::vector<int>> collisions;
            std::set<int> objectsToRemove;

            for (int i = 0; i < next.size(); i++) {
                for (int j = 0; j < i; j++) {
                    if (collide(next[i], next[j])) {
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

    inline std::vector<Trajectory> getTrajectories() const {
        std::map<int, Trajectory> trajectories;

        for (const auto& state : states) {
            for (const auto& object : state) {
                trajectories[object.id].positions.push_back(object.position);
                trajectories[object.id].velocities.push_back(object.velocity);
            }
        }

        std::vector<Trajectory> result;
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