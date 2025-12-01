#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include <common.hpp>
#include <iostream>

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	if (std::ifstream VertexShaderStream(vertex_file_path, std::ios::in); VertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else
	{
		printf("Impossible to open %s.", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	if (std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in); FragmentShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else
	{
		printf("Impossible to open %s.", fragment_file_path);
		getchar();
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;
	bool geo = false;
	GLuint GeometryShaderID;

	if (strcmp(geometry_file_path, "") != 0)
	{
		geo = true;
		GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		std::string GeometryShaderCode;
		if (std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in); GeometryShaderStream.is_open())
		{
			std::stringstream sstr;
			sstr << GeometryShaderStream.rdbuf();
			GeometryShaderCode = sstr.str();
			GeometryShaderStream.close();
		}
		else
		{
			printf("Impossible to open %s.", geometry_file_path);
			getchar();
			return 0;
		}

		printf("Compiling shader : %s\n", geometry_file_path);
		char const *GeometrySourcePointer = GeometryShaderCode.c_str();
		glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer, nullptr);
		glCompileShader(GeometryShaderID);

		glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> GeometryShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(GeometryShaderID, InfoLogLength, nullptr, &GeometryShaderErrorMessage[0]);
			printf("%s\n", &GeometryShaderErrorMessage[0]);
		}
	}
	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const *VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const *FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	if (geo)
		glAttachShader(ProgramID, GeometryShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	if (geo)
	{
		glDetachShader(ProgramID, GeometryShaderID);

		glDeleteShader(GeometryShaderID);
	}

	return ProgramID;
}