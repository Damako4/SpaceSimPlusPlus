#include <cstring>
#include <vector>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec2.hpp>
#include <soil2/SOIL2.h>

#include <common.hpp>

#include <rendering/text2D.h>
#include <tools/loadShader.hpp>

unsigned int textTextureID;
unsigned int textVertexBufferID;
unsigned int textUVBufferID;
unsigned int textShaderID;
unsigned int textUniformID;
unsigned int textVAO;

void initText2D(const char * texturePath) {
	textTextureID = SOIL_load_OGL_texture(texturePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y | SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_COMPRESS_TO_DXT);
	
	// Generate VAO and buffers
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVertexBufferID);
	glGenBuffers(1, &textUVBufferID);

	textShaderID = LoadShaders("../src/shaders/text/textVertexShader.glsl", "../src/shaders/text/textFragmentShader.glsl");
	textUniformID = glGetUniformLocation(textShaderID, "textureSampler");
}

void printText2D(const std::string& text, int x, int y, int size) {
	unsigned int length = text.length();

	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		auto vertex_up_left    = glm::vec2( x+i*size     , y+size );
		auto vertex_up_right   = glm::vec2( x+i*size+size, y+size );
		auto vertex_down_right = glm::vec2( x+i*size+size, y      );
		auto vertex_down_left  = glm::vec2( x+i*size     , y      );

		vertices.push_back(vertex_up_left   );
		vertices.push_back(vertex_down_left );
		vertices.push_back(vertex_up_right  );

		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		const char character = text[i];
		float uv_x = static_cast<float>(character%16)/16.0f;
		float uv_y = static_cast<float>(character/16)/16.0f;

		glm::vec2 uv_up_left    = glm::vec2( uv_x           , 1.0f - uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/16.0f, 1.0f - uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/16.0f, 1.0f - (uv_y + 1.0f/16.0f) );
		glm::vec2 uv_down_left  = glm::vec2( uv_x           , 1.0f - (uv_y + 1.0f/16.0f) );

		UVs.push_back(uv_up_left   );
		UVs.push_back(uv_down_left );
		UVs.push_back(uv_up_right  );

		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);
	}

	// Bind VAO before setting up vertex attributes
	glBindVertexArray(textVAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, textVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, textUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glUseProgram(textShaderID);

	const GLint widthID = glGetUniformLocation(textShaderID, "width");
	const GLint heightID = glGetUniformLocation(textShaderID, "height");

	glUniform1i(widthID, WIDTH);
	glUniform1i(heightID, HEIGHT);

	const GLint textureSamplerID = glGetUniformLocation(textShaderID, "textureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textTextureID);
	glUniform1i(textureSamplerID, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// VAO is already bound with vertex attributes set up
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// Unbind VAO
	glBindVertexArray(0);

}