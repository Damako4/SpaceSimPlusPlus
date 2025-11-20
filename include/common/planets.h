#ifndef PLANETS_H
#define PLANETS_H

#include <world/object.h>
#include <string>
#include <memory>
#include <types.h>

class Planet : public Object
{
public:
	Planet(const std::string &name, TextureInfo info, glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, float planetRadius, float planetMass, std::shared_ptr<Shader> shaderProgram);
	int id;
	std::string name;

	void render();
	void renderRay();

	void setVelocity(const glm::dvec3& v) { velocity = v; };
    void setAcceleration(const glm::dvec3& a) { acceleration = a; };

	float getRadius() const { return radius; }
	float getMass() const { return mass; }
	glm::vec3 getPlanetScreenCoords() const;
	glm::vec3 getScaledPosition() const { return glm::vec3(position_modelSpace / PHYSICS_SCALE_FACTOR); }
	glm::dvec3 getVelocity() const { return velocity; }
	glm::dvec3 getAcceleration() const { return acceleration; }

	glm::mat4 getMVP() const
	{
		return Shader::ProjectionMatrix * Shader::ViewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(getScaledPosition()));
	}
	glm::mat4 getModelMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), glm::vec3(getScaledPosition()));
	}
	glm::mat4 getNormalMatrix() const
	{
		return glm::transpose(glm::inverse(glm::mat3(getModelMatrix())));
	}

private:
	static int nextID;

	float mass;
	float radius;

	glm::dvec3 velocity;
	glm::dvec3 acceleration;

	void generateSphereVertices(float radius, int sectorCount, int stackCount, std::vector<unsigned int> &indices,
								std::vector<glm::vec3> &vertices,
								std::vector<glm::vec3> &normals,
								std::vector<glm::vec2> &uv);
};

#endif // PLANETS_H