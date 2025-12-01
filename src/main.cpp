#include <cstring>
#include <iostream>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>

#include <common.hpp>
#include <types.h>

#include <tools/error.h>
#include <tools/loadShader.hpp>
#include <tools/raycast.h>
#include <rendering/text2D.h>
#include <controls.h>
#include <world/object.h>
#include <planets.h>
#include <physics.h>
#include <world/axis.h>
#include <world/grid.h>
#include <world/skybox.h>
#include <rendering/light.h>

std::vector<Planet> planets;
ControlState state(planets);

int main()
{
	setup();
	GLFWwindow *window = glfwGetCurrentContext();

	Shader::initMatricesUBO();
	std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("vertexShader.vert", "fragmentShader.frag", "geometryShader.geom");
	std::shared_ptr<Shader> textShader = std::make_shared<Shader>("text/textVertexShader.vert", "text/textFragmentShader.frag");
	std::shared_ptr<Shader> skyboxShader = std::make_shared<Shader>("skybox/skyboxVertexShader.vert", "skybox/skyboxFragmentShader.frag");
	std::shared_ptr<Shader> lineShader = std::make_shared<Shader>("lines/lineVertexShader.vert", "lines/lineFragmentShader.frag");
	std::shared_ptr<Shader> raycastShader = std::make_shared<Shader>("raycasting/rayVertexShader.vert", "raycasting/rayFragmentShader.frag");
	std::shared_ptr<Shader> normalShader = std::make_shared<Shader>("normals/normalVertexShader.vert", "normals/normalFragmentShader.frag", "normals/normalGeometryShader.geom");

	// INIT WORLD
	initWorld(*defaultShader);
	Text2D text2D("fontmap.tga", textShader);
	SkyBox skybox(skyboxShader);
	Grid worldGrid(20, 0.5f, lineShader);
	Axis axis(lineShader);
	Raycast raycast(raycastShader);

	glm::vec3 lightPosition = glm::vec3(2.0f, 4.0f, 3.0f);
	glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float lightIntensity = 20.0f;
	Light light1 = Light(lightPosition, lightAmbient, lightColor, lightIntensity);

	// INIT GLOBAL STATE
	state.gridVisible = false;
	state.viewMode = ViewMode::ORBIT;
	state.controlMode = ControlMode::NONE;
	state.selectedPlanet = nullptr;
	state.axisHandler = &axis;
	state.raycastHandler = &raycast;

	TextureInfo info;

	// SUN (at origin)
	auto sunMass = 1.989e29f; // kg
	auto sunPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	auto sunVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	info.useTexture = true;
	info.texturePath = "sun.jpg";
	info.textureSamplerID = defaultShader->uniforms["textureSampler"];
	Planet sun = Planet("Sun", info, sunPosition, sunVelocity, glm::vec3(0.0f), 1.0f, sunMass, defaultShader);
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
	Planet earth = Planet("Earth", info, earthPosition, earthVelocity, glm::vec3(0.0f), 0.5f, earthMass, defaultShader);
	planets.push_back(earth);

	// MOON (orbiting Earth)
	auto moonMass = 7.342e27f;	   // kg
	auto moonDistance = 3.844e10f; // meters from Earth
	auto moonPosition = earthPosition + glm::vec3(moonDistance, 0.0f, 0.0f);
	// Calculate moon's orbital velocity around Earth
	float moonOrbitalSpeed = std::sqrt(gravitationalConstant * earthMass / moonDistance);
	auto moonVelocity = earthVelocity + glm::vec3(0.0f, 0.0f, moonOrbitalSpeed); // ~1,022 m/s relative to Earth
	info.useTexture = true;
	info.texturePath = "moon.jpg";
	Planet moon = Planet("Moon", info, moonPosition, moonVelocity, glm::vec3(0.0f), 0.3f, moonMass, defaultShader);
	planets.push_back(moon);

	double lastTime = glfwGetTime();
	double lastFPSTime = glfwGetTime();
	int nbFrames = 0;
	double frametime = 0;

	info.useTexture = true;
	info.texturePath = "box/Wood_Crate_001_basecolor.jpg";
	Object box("box.obj", info, defaultShader);
	info.useTexture = true;
	info.texturePath = "floor/brick.jpg";
	//info.color = glm::vec3(0.9f);
	Object floor("floor.obj", info, defaultShader);
	floor.setPosition(glm::dvec3(0.0f, -1.0f, 0.0f));

	do
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// INPUT HANDLING
		glfwPollEvents();

		double currentTime = glfwGetTime();
		double dt = currentTime - lastTime;
		nbFrames++;
		if (currentTime - lastFPSTime >= 1.0)
		{
			frametime = 1000.0 / static_cast<double>(nbFrames);
			nbFrames = 0;
			lastFPSTime = currentTime;
		}
		updatePhysics(planets, dt);
		handleStateChange(window);
		Shader::updateMatricesUBO();

		// WORLD RENDERING
		skybox.render();
		if (state.gridVisible)
		{
			worldGrid.update();
			worldGrid.render();
		}
		if (state.axisHandler != nullptr && state.selectedPlanet != nullptr)
			state.axisHandler->render(*state.selectedPlanet);

		glEnable(GL_DEPTH_TEST);
		glUseProgram(defaultShader->id);

		// RENDER PLANETS
		/*
		for (auto& planet : state.planets) { planet.render(); }

		for (auto& planet : state.planets)
		{
			glm::vec3 screenCoords = planet.getPlanetScreenCoords();
			if (state.viewMode == ViewMode::ORBIT && state.selectedPlanet == &planet)
			{
				screenCoords.y += planet.getRadius() * 300.0f;
			}
			else
			{
				screenCoords.y += planet.getRadius() * 2000.0f / glm::length(Shader::ViewMatrix[3] - glm::vec4(planet.getScaledPosition(), 1.0f));
			}
			text2D.render(planet.name,
						static_cast<int>(screenCoords.x),
						static_cast<int>(screenCoords.y),
						20);
		}
		*/

		float radius = 5.0f;
		float x = (radius * std::cos(currentTime));
		float z = (radius * std::sin(currentTime));
		glm::vec3 lightPos = glm::vec3(x, 4.0f, z); // Light above the scene
		glUniform3fv(defaultShader->uniforms["pointLights[0].position"], 1, &lightPos[0]);

		box.render();

		floor.render();

		/*
		glUseProgram(normalShader->id);
		box.setShader(normalShader);
		box.render();
		floor.setShader(normalShader);
		floor.render();
		box.setShader(defaultShader);
		floor.setShader(defaultShader);
		*/
		
		// TEXT RENDERING
		std::string modeString = "View Mode: ";
		if (state.viewMode == ViewMode::FREE)
		{
			modeString += "Free Camera";
		}
		else if (state.viewMode == ViewMode::ORBIT)
		{
			modeString += "Orbit Camera";
		}
		text2D.render(modeString, WIDTH * 2 - 500, HEIGHT * 2 - 30, 20);
		std::string frametimeString = "Frametime : " + std::to_string(frametime);
		text2D.render(frametimeString, 0, HEIGHT * 2 - 30, 30);

		glfwSwapBuffers(window);

	} while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
