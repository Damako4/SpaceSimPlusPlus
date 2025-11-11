#include <cstring>
#include <iostream>

#include "common/common.hpp"
#include "common/error.h"
#include "loadShader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <common/controls.h>
#include <common/objloader.h>
#include <common/vboindexer.h>
#include <soil2/SOIL2.h>
#include <common/world.h>
#include <common/objects.h>
#include <structs.h>
#include <common/raycast.h>

#include "common/text2D.h"
#include <common/planets.h>
#include <thread>
#include <common/physics.h>

using namespace glm;

int main()
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return 1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Set debug mode
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Cool Game", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetMouseButtonCallback(window, raycast);

	if (const int version = gladLoadGL(); version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		return 1;
	}

	std::cout << "Loaded GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << std::endl;
	std::cout << "Supported OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	// Used extensions: ARB_texture_view, KHR_debug
	// Load extensions
	if constexpr (GL_EXT_texture_compression_s3tc)
	{
		std::cout << "Loaded GL_EXT_texture_compression_s3tc extension" << std::endl;
	}
	if constexpr (GL_KHR_debug)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
		glDebugMessageCallback(debugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		std::cout << "Loaded GL_KHR_debug extension" << std::endl;
		std::cout << "GL_DEBUG_OUTPUT enabled" << std::endl;
	}

	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);

	const GLuint programID = LoadShaders("../src/shaders/vertexShader.glsl", "../src/shaders/fragmentShader.glsl");
	const GLuint castProgramID = LoadShaders("../src/shaders/raycasting/rayVertexShader.glsl", "../src/shaders/raycasting/rayFragmentShader.glsl");

	const GLint textureID = glGetUniformLocation(programID, "textureSampler");

	const GLint matrixID = glGetUniformLocation(programID, "MVP");
	const GLint lightPositionID = glGetUniformLocation(programID, "lightPosition_worldSpace");
	const GLint modelMatrixID = glGetUniformLocation(programID, "M");
	const GLint cameraMatrixID = glGetUniformLocation(programID, "V");

	const GLint lightColorID = glGetUniformLocation(programID, "lightColor");
	const GLint lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
	const GLint diffuseColorID = glGetUniformLocation(programID, "diffuseColor");
	const GLint ambientLightIntensityID = glGetUniformLocation(programID, "ambientLightIntensity");
	const GLint specularColorID = glGetUniformLocation(programID, "specularColor");
	const GLint specularLobeWidthID = glGetUniformLocation(programID, "specularLobeWidth");
	const GLint alphaID = glGetUniformLocation(programID, "alpha");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Cull tris where normal is not facing camera
	// glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(programID);

	auto lightPosition_worldSpace = glm::vec3(3.0f, -3.0f, 3.0f);
	glUniform3fv(lightPositionID, 1, &lightPosition_worldSpace[0]);

	auto lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(lightColorID, 1, &lightColor[0]);

	auto lightIntensity = 20.0f;
	glUniform1f(lightIntensityID, lightIntensity);

	auto ambientLightIntensity = 0.2f;
	glUniform1f(ambientLightIntensityID, ambientLightIntensity);

	auto diffuseColor = glm::vec3(1.0f, 1.0f, 0.0f);
	glUniform3fv(diffuseColorID, 1, &diffuseColor[0]);

	auto specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(specularColorID, 1, &specularColor[0]);

	constexpr GLint specularLobeWidth = 5;
	glUniform1i(specularLobeWidthID, specularLobeWidth);

	constexpr GLfloat alpha = 1.0f;
	glUniform1f(alphaID, alpha);
	// TODO: Implement proper transparency!!!

	initText2D("../src/textures/fontmap.tga");
	initAxis();
	initWorldGrid();
	initRaycastingShader();

	std::vector<Planet> planets;

	TextureInfo info;

	auto earthMass = 5.972e24f;
	auto earthPosition = vec3(1.5e11f, 0.0f, 0.0f);
	auto earthVelocity = vec3(0.0f, 0.0f, 29780.0f);
	info.useTexture = true;
	info.texturePath = "earth.jpg";
	info.textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	Planet earth = Planet(programID, "Earth", info, earthPosition, earthVelocity, glm::vec3(0.0f), 0.5f, earthMass);
	planets.push_back(earth);


	auto sunMass = 1.9885e30f;
	auto sunPosition = vec3(0.0f, 0.0f, 0.0f);
	auto sunVelocity = vec3(0.0f, 0.0f, 0.0f);
	info.useTexture = false;
	info.color = glm::vec3(0.0f, 1.0f, 0.0f);
	Planet sun = Planet(programID, "Sun", info, sunPosition, sunVelocity, glm::vec3(0.0f), 2.0f, sunMass);
	planets.push_back(sun);

	ControlState state(planets);
	state.gridVisible = false;
	state.viewMode = ViewMode::FREE;
	state.controlMode = ControlMode::VIEW;
	state.selectedPlanet = nullptr;

	computeMatricesFromInputs(window);
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	auto ModelMatrix = glm::mat4(1.0);

	double lastTime = glfwGetTime();
	double lastFPSTime = glfwGetTime();
	int nbFrames = 0;
	double frametime = 0;

	do
	{
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentTime = glfwGetTime();
		double dt = currentTime - lastTime;

		updatePhysics(planets, dt);

		nbFrames++;
		if (currentTime - lastFPSTime >= 1.0)
		{
			frametime = 1000.0 / static_cast<double>(nbFrames);
			nbFrames = 0;
			lastFPSTime = currentTime;
		}

		handleStateChange(window, state);
		
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();
		state.ProjectionMatrix = ProjectionMatrix;
		state.ViewMatrix = ViewMatrix;
		glfwSetWindowUserPointer(window, &state);

		for (Planet &planet : planets)
		{
			if (state.selectedPlanet != nullptr && planet.id == state.selectedPlanet->id)
			{
				drawAxis(planet.position_modelSpace, planet.getRadius()*1.5f);
				
			}
		}

		drawWorldGrid(ProjectionMatrix, ViewMatrix, centerOfMass(planets));

		glEnable(GL_DEPTH_TEST);
		glUseProgram(programID);

		for (Planet &planet : planets)
		{
			glm::vec3 scaledPositionModelSpace = planet.getScaledPosition();

			glm::mat4 ObjectModelMatrix = glm::mat4(1.0f);									   // Reset to identity
			ObjectModelMatrix = glm::translate(ObjectModelMatrix, scaledPositionModelSpace); // Translate to planet's position
			ObjectModelMatrix = glm::scale(ObjectModelMatrix, glm::vec3(planet.getRadius()));	   // Scale planet based on time for demonstration

			glm::mat4 ObjectMVP = ProjectionMatrix * ViewMatrix * ObjectModelMatrix;

			glUniformMatrix4fv(matrixID, 1, GL_FALSE, &ObjectMVP[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &ObjectModelMatrix[0][0]); // send per-object model matrix
			glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			glUniform3fv(diffuseColorID, 1, &planet.color[0]);

			planet.draw();

			/*
			printText2D(planet.name, 
						  static_cast<int>(getPlanetScreenCoords(planet, ViewMatrix, ProjectionMatrix).x),
						  static_cast<int>(getPlanetScreenCoords(planet, ViewMatrix, ProjectionMatrix).y),
						  20);
						  */
		}

		std::string modeString = "View Mode: ";
		if (state.viewMode == ViewMode::FREE)
		{
			modeString += "Free Camera";
		}
		else if (state.viewMode == ViewMode::ORBIT)
		{
			modeString += "Orbit Camera";
		}
		printText2D(modeString, WIDTH * 2 - 500, HEIGHT * 2 - 30, 20);
		std::string frametimeString = "Frametime : " + std::to_string(frametime);
		printText2D(frametimeString, 0, HEIGHT * 2 - 30, 30);
		std::string controlsString = "Controls: C - Free Camera | O - Orbit Camera | G - Move Object | S - Scale Object | V - Toggle Grid";
		printText2D(controlsString, 10, 10, 20);

		glfwSwapBuffers(window);

	} while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
