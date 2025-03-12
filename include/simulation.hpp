#pragma once
#include <glm/glm.hpp>

#include <functional>
#include <map>
#include <optional>
#include <vector>

template<typename T>
struct Object {
    T attributes;

    int creationTime = 0;
    int destructionTime = std::numeric_limits<int>::max();

    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;

    template<typename... TArgs>
    inline Object(const glm::vec2& initialPosition, const glm::vec2& initialVelocity, const TArgs&... args)
        : positions({initialPosition}), velocities({initialVelocity}), attributes{args...} {
    }

    inline bool exists(int time) const {
        return time >= creationTime && time <= destructionTime;
    }

    inline std::optional<glm::vec2> getPosition(int timeStep) const {
        if (!exists(timeStep)) {
            return {};
        }

        return std::optional<glm::vec2>(positions[timeStep - creationTime]);
    }

    inline std::optional<glm::vec2> getVelocity(int timeStep) const {
        if (!exists(timeStep)) {
            return {};
        }

        return std::optional(velocities[timeStep - creationTime]);
    }

    static bool collide(const Object<T>& first, const Object<T>& second, int timeStep);
};

struct Mass {
    float mass;
    float radius;
};

template<typename T>
class Simulation {
  public:
    using CollisionCallback = std::function<void(int, const std::vector<int>&, int, std::vector<Object<T>>&)>;
    using ForceCallback = std::function<glm::vec2(int, int, const std::vector<Object<T>>&)>;

    std::vector<Object<T>> objects;
    float stepSize;

    inline Simulation(const std::vector<Object<T>>& objects, const ForceCallback& a, float stepSize = 1.0f)
        : objects(objects), a(a), stepSize(stepSize) {
    }

    inline Simulation(const std::vector<Object<T>>& objects, const ForceCallback& a, const CollisionCallback& onCollision, float stepSize = 1.0f)
        : Simulation(objects, a, stepSize) {
        this->onCollision = onCollision;
        handleCollisions = true;
    }

    inline void step() {
        std::vector<glm::vec2> accelerations;

        // velocity verlet
        for (int i = 0; i < objects.size(); i++) {
            if (!objects[i].exists(currentTimeStep)) {
                accelerations.emplace_back(0.0f);
                continue;
            }

            const glm::vec2& acceleration = a(i, currentTimeStep, objects);

            objects[i].positions.push_back(objects[i].getPosition(currentTimeStep).value() + objects[i].getVelocity(currentTimeStep).value() * stepSize + acceleration * stepSize * stepSize / 2.0f);
            accelerations.push_back(acceleration);
        }

        for (int i = 0; i < objects.size(); i++) {
            if (!objects[i].exists(currentTimeStep)) {
                continue;
            }

            const glm::vec2& nextAcceleration = a(i, currentTimeStep + 1, objects);
            objects[i].velocities.push_back(objects[i].getVelocity(currentTimeStep).value() + (accelerations[i] + nextAcceleration) * stepSize / 2.0f);
        }

        currentTimeStep++;
        if (handleCollisions) {
            // check for collisions
            std::map<int, std::vector<int>> collisions;
            for (int i = 0; i < objects.size(); i++) {
                if (!objects[i].exists(currentTimeStep)) {
                    continue;
                }

                for (int j = 0; j < i; j++) {
                    if (!objects[j].exists(currentTimeStep)) {
                        continue;
                    }

                    if (Object<T>::collide(objects[i], objects[j], currentTimeStep)) {
                        collisions[i].push_back(j);
                    }
                }
            }

            for (const auto& [index, colls] : collisions) {
                onCollision.value()(index, colls, currentTimeStep, objects);
            }
        }
    }

    inline int endTime() const {
        return currentTimeStep;
    }

  private:
    ForceCallback a;

    std::optional<CollisionCallback> onCollision = std::nullopt;
    bool handleCollisions = false;

    int currentTimeStep = 0;
};

template<typename T>
class SimulationState {
    const Simulation<T>& simulation;

  public:
    inline SimulationState(const Simulation<T>& simulation)
        : simulation(simulation) {
    }

    inline int objectsCount(int time) const {
        int count = 0;
        for (auto it = begin(time); it != end(time); it++) {
            count++;
        }

        return count;
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

    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Object<T>;
        using pointer = const value_type*;
        using reference = const value_type&;

        inline Iterator(const Simulation<T>& simulation, int time, int start = 0)
            : m_simulation(simulation), m_time(time), m_index(start) {

            while (m_index < m_simulation.objects.size() && !m_simulation.objects[m_index].exists(m_time)) {
                m_index++;
            }
        }

        inline reference operator*() const {
            return m_simulation.objects[m_index];
        }

        inline pointer operator->() const {
            return &(m_simulation.objects[m_index]);
        }

        inline Iterator& operator++() {
            do {
                m_index++;
            } while (m_index < m_simulation.objects.size() && !m_simulation.objects[m_index].exists(m_time));

            return *this;
        }

        inline Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        inline friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.m_time == b.m_time && a.m_index == b.m_index;
        }

        inline friend bool operator!=(const Iterator& a, const Iterator& b) {
            return !(a == b);
        }

      private:
        const Simulation<T>& m_simulation;
        int m_time;
        int m_index;
    };

    inline Iterator begin(int time) const {
        return Iterator(simulation, time);
    }

    inline Iterator end(int time) const {
        int index = simulation.objects.size() - 1;
        while (index > 0 && !simulation.objects[index].exists(time)) {
            index--;
        }

        return Iterator(simulation, time, index + 1);
    }
};