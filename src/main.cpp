#include <cstring>
#include <iostream>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>

#include <common.hpp>
#include <structs.h>

#include <tools/error.h>
#include <tools/loadShader.hpp>
#include <tools/raycast.h>
#include <rendering/text2D.h>
#include <controls.h>
#include <objects.h>
#include <planets.h>
#include <physics.h>
#include <world/axis.h>
#include <world/grid.h>
#include <world/skybox.h>

// Global state variable definition
std::vector<Planet> planets;
ControlState state(planets);

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Gravity Sim++", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
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
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
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
	initRaycastingShader();
	SkyBox skybox;

	// Initialize global state properties
	state.programID = programID;
	state.gridVisible = false;
	state.viewMode = ViewMode::ORBIT;
	state.controlMode = ControlMode::VIEW;
	state.selectedPlanet = nullptr;
	state.axisHandler = new Axis();

	Grid worldGrid(20, 0.5f);

	TextureInfo info;

	// In main.cpp, replace your planet initialization code with:

	// SUN (at origin)
	auto sunMass = 1.989e29f; // kg
	auto sunPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	auto sunVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	info.useTexture = true;
	info.texturePath = "sun.jpg";
	info.textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	Planet sun = Planet("Sun", info, sunPosition, sunVelocity, glm::vec3(0.0f), 1.0f, sunMass);
	planets.push_back(sun);

	// EARTH
	auto earthMass = 5.972e28f;		// kg (fixed from your 5.972e29f)
	auto earthDistance = 1.496e11f; // meters (1 AU from sun)
	auto earthPosition = glm::vec3(earthDistance, 0.0f, 0.0f);
	// Calculate orbital velocity: v = sqrt(G * M_sun / r)
	float earthOrbitalSpeed = std::sqrt(gravitationalConstant * sunMass / earthDistance);
	auto earthVelocity = glm::vec3(0.0f, 0.0f, earthOrbitalSpeed); // ~29,780 m/s
	info.useTexture = true;
	info.texturePath = "earth.jpg";
	Planet earth = Planet("Earth", info, earthPosition, earthVelocity, glm::vec3(0.0f), 0.5f, earthMass);
	planets.push_back(earth);

	
	// MOON (orbiting Earth)
	auto moonMass = 7.342e27f;	  // kg
	auto moonDistance = 3.844e10f; // meters from Earth
	auto moonPosition = earthPosition + glm::vec3(moonDistance, 0.0f, 0.0f);
	// Calculate moon's orbital velocity around Earth
	float moonOrbitalSpeed = std::sqrt(gravitationalConstant * earthMass / moonDistance);
	auto moonVelocity = earthVelocity + glm::vec3(0.0f, 0.0f, moonOrbitalSpeed); // ~1,022 m/s relative to Earth
	info.useTexture = true;
	info.texturePath = "moon.jpg";
	Planet moon = Planet("Moon", info, moonPosition, moonVelocity, glm::vec3(0.0f), 0.3f, moonMass);
	planets.push_back(moon);

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

		handleStateChange(window);

		skybox.render();

		glEnable(GL_DEPTH_TEST);
		glUseProgram(programID);

		lightPosition_worldSpace = planets[0].getScaledPosition();
		glUniform3fv(lightPositionID, 1, &lightPosition_worldSpace[0]);

		for (Planet &planet : planets)
		{
			glm::vec3 scaledPositionModelSpace = planet.getScaledPosition();

			glm::mat4 ObjectModelMatrix = glm::mat4(1.0f);									  // Reset to identity
			ObjectModelMatrix = glm::translate(ObjectModelMatrix, scaledPositionModelSpace);  // Translate to planet's position
			ObjectModelMatrix = glm::scale(ObjectModelMatrix, glm::vec3(planet.getRadius())); // Scale planet based on time for demonstration

			glm::mat4 ObjectMVP = state.ProjectionMatrix * state.ViewMatrix * ObjectModelMatrix;

			glUniformMatrix4fv(matrixID, 1, GL_FALSE, &ObjectMVP[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &ObjectModelMatrix[0][0]); // send per-object model matrix
			glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &state.ViewMatrix[0][0]);

			glUniform3fv(diffuseColorID, 1, &planet.color[0]);

			planet.render();

			if (state.selectedPlanet == &planet && state.axisHandler != nullptr)
			{
				state.axisHandler->renderAxis(planet);
			}
		}

		for (Planet &planet : state.planets)
		{
			glm::vec3 screenCoords = planet.getPlanetScreenCoords();
			if (state.viewMode == ViewMode::ORBIT && state.selectedPlanet == &planet)
			{
				screenCoords.y += planet.getRadius() * 300.0f;
			}
			else
			{
				screenCoords.y += planet.getRadius() * 2000.0f / glm::length(state.ViewMatrix[3] - glm::vec4(planet.getScaledPosition(), 1.0f));
			}
			printText2D(planet.name,
						static_cast<int>(screenCoords.x),
						static_cast<int>(screenCoords.y),
						20);
		}

		if (state.gridVisible) {
			worldGrid.update();
			worldGrid.render();
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

		glfwSwapBuffers(window);

	} while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
