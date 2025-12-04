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
#include <rendering/render.h>
#include <rendering/shadow.h>
#include <rendering/debugQuad.h>

std::vector<Planet> planets;
std::vector<Object> objects;
ControlState state(planets, objects);

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
	ShadowMap::shadowShader = std::make_shared<Shader>("shadows/shadowVertexShader.vert", "shadows/shadowFragmentShader.frag");
	std::shared_ptr<Shader> debugDepthShader = std::make_shared<Shader>("debug/debugDepthVertex.vert", "debug/debugDepthFragment.frag");

	ShadowMap shadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);
	DebugQuad debugQuad(debugDepthShader);

	// Bind UBO to binding point 0 for all shaders that use the Matrices block
	GLuint matricesBlockIndex;
	
	matricesBlockIndex = glGetUniformBlockIndex(defaultShader->id, "Matrices");
	if (matricesBlockIndex != GL_INVALID_INDEX) {
		glUniformBlockBinding(defaultShader->id, matricesBlockIndex, 0);
	}
	
	matricesBlockIndex = glGetUniformBlockIndex(skyboxShader->id, "Matrices");
	if (matricesBlockIndex != GL_INVALID_INDEX) {
		glUniformBlockBinding(skyboxShader->id, matricesBlockIndex, 0);
	}
	
	matricesBlockIndex = glGetUniformBlockIndex(normalShader->id, "Matrices");
	if (matricesBlockIndex != GL_INVALID_INDEX) {
		glUniformBlockBinding(normalShader->id, matricesBlockIndex, 0);
	}

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
	state.lights[0] = light1;
	
	// INIT GLOBAL STATE
	state.gridVisible = false;
	state.viewMode = ViewMode::ORBIT;
	state.controlMode = ControlMode::NONE;
	state.selectedPlanet = nullptr;
	state.axisHandler = &axis;
	state.raycastHandler = &raycast;

	TextureInfo info;

	// Reserve space for planets and objects to prevent vector reallocation
	planets.reserve(10);
	objects.reserve(10);

	// SUN (at origin)
	auto sunMass = 1.989e29f; // kg
	auto sunPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	auto sunVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	info.useTexture = true;
	info.texturePath = "sun.jpg";
	info.textureSamplerID = defaultShader->uniforms["textureSampler"];
	planets.emplace_back("Sun", info, sunPosition, sunVelocity, glm::vec3(0.0f), 1.0f, sunMass, defaultShader);

	// EARTH
	auto earthMass = 5.972e28f;		// kg (fixed from your 5.972e29f)
	auto earthDistance = 1.496e11f; // meters (1 AU from sun)
	auto earthPosition = glm::vec3(earthDistance, 0.0f, 0.0f);
	// Calculate orbital velocity: v = sqrt(G * M_sun / r)
	float earthOrbitalSpeed = std::sqrt(gravitationalConstant * sunMass / earthDistance);
	auto earthVelocity = glm::vec3(0.0f, 0.0f, earthOrbitalSpeed); // ~29,780 m/s
	info.useTexture = true;
	info.texturePath = "earth.jpg";
	planets.emplace_back("Earth", info, earthPosition, earthVelocity, glm::vec3(0.0f), 0.5f, earthMass, defaultShader);

	// MOON (orbiting Earth)
	auto moonMass = 7.342e27f;	   // kg
	auto moonDistance = 3.844e10f; // meters from Earth
	auto moonPosition = earthPosition + glm::vec3(moonDistance, 0.0f, 0.0f);
	// Calculate moon's orbital velocity around Earth
	float moonOrbitalSpeed = std::sqrt(gravitationalConstant * earthMass / moonDistance);
	auto moonVelocity = earthVelocity + glm::vec3(0.0f, 0.0f, moonOrbitalSpeed); // ~1,022 m/s relative to Earth
	info.useTexture = true;
	info.texturePath = "moon.jpg";
	planets.emplace_back("Moon", info, moonPosition, moonVelocity, glm::vec3(0.0f), 0.3f, moonMass, defaultShader);

	double lastTime = glfwGetTime();
	double lastFPSTime = glfwGetTime();
	int nbFrames = 0;
	double frametime = 0;

	info.useTexture = true;
	info.texturePath = "box/Wood_Crate_001_basecolor.jpg";
	state.objects.emplace_back("box.obj", info, defaultShader);
	
	info.useTexture = true;
	info.texturePath = "floor/brick.jpg";
	state.objects.emplace_back("floor.obj", info, defaultShader);
	state.objects.back().setPosition(glm::dvec3(0.0f, -1.0f, 0.0f));

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

		float radius = 5.0f;
		float x = (radius * std::cos(currentTime));
		float z = (radius * std::sin(currentTime));
		glm::vec3 lightPos = glm::vec3(3.0f, 3.0f, 3.0f); // Light above the scene
		state.lights[0].position = lightPos;

		shadowMap.renderShadowMap();

		auto [fbWidth, fbHeight] = getViewportSize();
		glViewport(0, 0, fbWidth, fbHeight);

		glUseProgram(defaultShader->id);
		
		glUniform3fv(defaultShader->uniforms["pointLights[0].position"], 1, &lightPos[0]);
		
		GLint lightSpaceMatrixLoc = defaultShader->getUniform("lightSpaceMatrix");
		if (lightSpaceMatrixLoc != -1) {
			glm::mat4 lightSpaceMat = shadowMap.getLightSpaceMatrix();
			glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMat[0][0]);
		}
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowMap.getDepthTexture());
		GLint depthMapLoc = defaultShader->getUniform("depthMap");
		if (depthMapLoc != -1) {
			glUniform1i(depthMapLoc, 1); // Tell shader depthMap is in texture unit 1
		}

		renderScene();
		
		if (state.normalsVisible)
		{
			glUseProgram(normalShader->id);
			for (Object& obj : state.objects) {
				obj.renderNormals(normalShader);
			}
		}

		// Render depth map to screen for debugging
		glDisable(GL_DEPTH_TEST);
		debugQuad.render(shadowMap.getDepthTexture());
		glEnable(GL_DEPTH_TEST);

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
