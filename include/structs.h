//
// Created by damir on 10/30/24.
//

#ifndef STRUCTS_H
#define STRUCTS_H
#include <glad/glad.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/matrix.hpp>
#include <vector>
#include <string>

class Planet;

// Physics constants
const float G = 1.0f;  // Gravitational constant

enum class ViewMode {
	FREE,
	ORBIT
};

enum class ControlMode {
	MOVE,
	SCALE,
	VIEW,
	NONE
};

typedef struct {
	bool useTexture;
	GLint textureSamplerID = -1;
	std::string texturePath;
	GLuint texture;
	glm::vec3 color;
} TextureInfo;

typedef struct {
	GLuint programID;
	int id;
	std::string name;
	std::vector<unsigned int> indices;
	GLuint VertexArrayID;
	GLuint elementBuffer;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint normalBuffer;
	glm::dvec3 position_modelSpace;
	glm::dvec3 velocity;
	glm::dvec3 acceleration;
	glm::vec3 color;
	TextureInfo textureInfo;
} Object;

struct ControlState {
	ViewMode viewMode;
	ControlMode controlMode;
	bool gridVisible;
	std::vector<Planet>& planets;
	Planet* selectedPlanet;
	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	ControlState(std::vector<Planet>& planet_ref) : planets(planet_ref) {
		
	}
};

#endif //STRUCTS_H
