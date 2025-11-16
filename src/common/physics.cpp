#include <common.hpp>
#include <physics.h>
#include <structs.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <planets.h>

// Constants for stable orbital motion
const float timeScale = 8640.0f; // 8640.0f;        // 1 simulation frame = 1 day for visible orbital motion

glm::dvec3 calculateForces(std::vector<Planet> planets, Planet target)
{
    glm::dvec3 netForce = glm::dvec3(0.0);
    for (Planet planet : planets)
    {
        if (planet.id != target.id)
        {
            glm::dvec3 r = planet.position_modelSpace - target.position_modelSpace;
            double distance = glm::length(r);
            
            const double MIN_DISTANCE = 1e6; // Minimum separation: 1000 km in real-world units
            if (distance < MIN_DISTANCE) {
                distance = MIN_DISTANCE;
            }
            
            glm::dvec3 direction = glm::normalize(r);
            
            double forceMagnitude = gravitationalConstant * target.getMass() * planet.getMass() / (distance * distance);

            glm::dvec3 force = forceMagnitude * direction;
            netForce += force;
        }
    }
    return netForce;
}

void updatePhysics(std::vector<Planet> &planets, double dt)
{
    dt *= timeScale;
    auto old_planets = planets;
    for (int i = 0; i < planets.size(); i++)
    {
        Planet p = old_planets[i];
        
        glm::dvec3 force = calculateForces(old_planets, p);
        glm::dvec3 new_acc = force / glm::dvec3(p.getMass());  // a = F/m
        
        glm::dvec3 new_vel = p.velocity + (p.acceleration + new_acc) * (dt * 0.5);
        glm::dvec3 new_pos = p.position_modelSpace + p.velocity * dt + p.acceleration * (dt * dt * 0.5);

        planets[i].position_modelSpace = new_pos;
        planets[i].velocity = new_vel;
        planets[i].acceleration = new_acc;
    }
}