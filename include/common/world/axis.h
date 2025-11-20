#ifndef AXIS_H
#define AXIS_H

#include <glad/glad.h>
#include <glm/matrix.hpp>
#include <vector>
#include <world/object.h>

class Planet;

class Axis : public Object 
{
	public:
		Axis(std::shared_ptr<Shader> shaderProgram);

		std::vector<float> axisVertices;

		void render(Planet& planet);
};

#endif