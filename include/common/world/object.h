#ifndef STRUCTS_H
#define STRUCTS_H

#include <glad/glad.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/matrix.hpp>
#include <vector>
#include <string>
#include <rendering/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <types.h>

class Planet;
class Axis;

class Object {
public:
    Object(std::string modelP, TextureInfo textureInfo, std::shared_ptr<Shader> shaderProgram);
	Object(TextureInfo textureInfo, std::shared_ptr<Shader> shaderProgram);
	Object(std::shared_ptr<Shader> shaderProgram);

	virtual ~Object() = default;

	virtual void render();

	// SETTERS
    void setPosition(const glm::dvec3& position) { position_modelSpace = position; }
    void setColor(const glm::vec3& color);
    void setTexture(const TextureInfo& textureInfo);
	void setShader(std::shared_ptr<Shader> newShader) { shader->id = newShader->id; }

	// GETTERS
	glm::dvec3 getPosition() const { return position_modelSpace; }
	TextureInfo getTexture() const { return textureInfo; }
	glm::mat4 getMVP() const {
		return Shader::ProjectionMatrix * Shader::ViewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(position_modelSpace));
	}
	glm::mat4 getModelMatrix() const {
		return glm::translate(glm::mat4(1.0f), glm::vec3(position_modelSpace));
	}
	glm::mat3 getNormalMatrix() const {
		return glm::transpose(glm::inverse(glm::mat3(getModelMatrix())));
	}

protected:
	std::shared_ptr<Shader> shader;

	glm::dvec3 position_modelSpace;

	std::vector<unsigned int> indices;
	GLuint vertexCount;
    GLuint i_vao;
	GLuint i_elementBuffer;
	GLuint i_vertexBuffer;
	GLuint i_uvBuffer;
	GLuint i_normalBuffer;

	TextureInfo textureInfo;

};

#endif //STRUCTS_H
