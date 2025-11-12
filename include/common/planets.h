#ifndef PLANETS_H
#define PLANETS_H

#include <structs.h>
#include <string>

class Planet : public Object {
	public:
	Planet(const std::string &name, TextureInfo info, glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, float planetRadius, float planetMass);
        std::string name;
	
		void render();
		float getRadius() const;
		float getMass() const;
		glm::vec3 getPlanetScreenCoords() const;
		glm::vec3 getScaledPosition() const;

	private:
        float mass;
        float radius;

		void generateSphereVertices(float radius, int sectorCount, int stackCount, std::vector<unsigned int>& indices,
                            std::vector<glm::vec3>& vertices,
                            std::vector<glm::vec3>& normals,
                            std::vector<glm::vec2>& uv);
};

#endif //PLANETS_H