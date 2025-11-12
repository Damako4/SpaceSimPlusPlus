#ifndef AXIS_H
#define AXIS_H

#include <glad/glad.h>
#include <glm/matrix.hpp>
#include <vector>

class Planet;

class Axis {
	public:
		Axis();

		GLuint axisVAO;
		GLuint axisVBO;
		GLuint mvpID;
		std::vector<float> axisVertices;

		GLuint lineColorID;

		void renderAxis(Planet& planet);
};

#endif