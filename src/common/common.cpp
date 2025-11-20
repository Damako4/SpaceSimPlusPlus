#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <common/common.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <tools/raycast.h>
#include <controls.h>
#include <tools/error.h>

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

void onExit() {
	GLFWwindow* window = glfwGetCurrentContext();
	if (window != nullptr)
		glfwDestroyWindow(window);
	glfwTerminate();
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void initWorld(Shader defaultShader) {
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(defaultShader.id);

	auto lightPosition_worldSpace = glm::vec3(3.0f, -3.0f, 3.0f);
	glUniform3fv(defaultShader.uniforms["lightPosition_worldspace"], 1, &lightPosition_worldSpace[0]);

	auto lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(defaultShader.uniforms["lightColor"], 1, &lightColor[0]);

	auto lightIntensity = 20.0f;
	glUniform1f(defaultShader.uniforms["lightIntensity"], lightIntensity);

	auto ambientLightIntensity = 0.2f;
	glUniform1f(defaultShader.uniforms["ambientLightIntensity"], ambientLightIntensity);

	auto diffuseColor = glm::vec3(1.0f, 1.0f, 0.0f);
	glUniform3fv(defaultShader.uniforms["diffuseColor"], 1, &diffuseColor[0]);

	auto specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(defaultShader.uniforms["specularColor"], 1, &specularColor[0]);

	constexpr GLint specularLobeWidth = 5;
	glUniform1i(defaultShader.uniforms["specularLobeWidth"], specularLobeWidth);

	constexpr GLfloat alpha = 1.0f;
	glUniform1f(defaultShader.uniforms["alpha"], alpha);
}

void setup() {
	atexit(onExit);

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		exit(1);
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
		exit(1);
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// SETUP CALLBACKS
	glfwSetMouseButtonCallback(window, raycast);
	glfwSetScrollCallback(window, scroll_callback);

	if (const int version = gladLoadGL(); version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		exit(1);
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
}

GLuint loadDDS(const char * imagepath){

	unsigned char header[124];

	FILE *fp;

	/* try to open the file */
	fp = fopen(imagepath, "rb");
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
		return 0;
	}

	/* verify the type of file */
	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(fp);
		return 0;
	}

	/* get the surface desc */
	fread(&header, 124, 1, fp);

	unsigned int height      = *reinterpret_cast<unsigned int *>(&(header[8]));
	unsigned int width	     = *reinterpret_cast<unsigned int *>(&(header[12]));
	const unsigned int linearSize	 = *reinterpret_cast<unsigned int *>(&(header[16]));
	const unsigned int mipMapCount = *reinterpret_cast<unsigned int *>(&(header[24]));
	const unsigned int fourCC      = *reinterpret_cast<unsigned int *>(&(header[80]));


	/* how big is it going to be including all mipmaps? */
	unsigned int bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	auto *buffer = static_cast<unsigned char *>(malloc(bufsize * sizeof(unsigned char)));
	fread(buffer, 1, bufsize, fp);
	/* close the file pointer */
	fclose(fp);

	unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch(fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
			0, size, buffer + offset);

		offset += size;
		width  /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if(width < 1) width = 1;
		if(height < 1) height = 1;

	}

	free(buffer);

	return textureID;


}


GLuint loadBMP_custom(const char *filepath) {
	unsigned char header[54];

	FILE *file = fopen(filepath, "rb");
	if (!file){printf("Image could not be opened!\n"); return 0;}
	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return false;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return false;
	}
	unsigned int dataPos = *reinterpret_cast<int *>(&(header[0x0A]));
	unsigned int imageSize = *reinterpret_cast<int *>(&(header[0x22]));
	unsigned int width = *reinterpret_cast<int *>(&(header[0x12]));
	unsigned int height = *reinterpret_cast<int *>(&(header[0x16]));
	if (imageSize == 0) imageSize = width * height * 3;
	if (dataPos == 0) dataPos = 54;
	auto *data = new unsigned char[imageSize];
	fread(data,1,imageSize,file);
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}