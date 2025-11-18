#ifndef STRUCTS_H
#define STRUCTS_H

#include <glad/glad.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/matrix.hpp>
#include <vector>
#include <string>

class Planet;
class Axis;

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
	int id;
	std::string name;
	std::vector<unsigned int> indices;
	GLuint mvpID;
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

struct CameraState{
    glm::vec3 position = glm::vec3(4, 4, 4);
    float horizontalAngle = 90.0f;
    float verticalAngle = 0.0f;
};

struct ControlState {
	ViewMode viewMode;
	ControlMode controlMode;
	bool gridVisible;
	std::vector<Planet>& planets;
	Planet* selectedPlanet;
	Axis* axisHandler;
	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	CameraState freeCamState;

	GLint matrixID;
	GLint lightPositionID;
	GLint modelMatrixID;
	GLint cameraMatrixID;
	GLint diffuseColorID;

	GLuint programID;
	GLuint lineProgramID;
	GLuint textProgramID;
	GLuint cubemapProgramID;

	ControlState(std::vector<Planet>& planet_ref) : planets(planet_ref) {
		
	}
};

extern ControlState state;

#endif //STRUCTS_H
