#include <common.hpp>
#include <utility>

void renderScene() {
    for (Object& obj : state.objects) {
        obj.render();
    }
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
}

std::pair <int, int> getViewportSize() {
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &fbWidth, &fbHeight);
	return {fbWidth, fbHeight};
}