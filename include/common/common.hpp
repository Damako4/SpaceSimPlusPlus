#ifndef COMMON_H
#define COMMON_H

#define WIDTH 1024
#define HEIGHT 768
#define FOV 45.0f
#define NEAR_CLIP 0.1f
#define FAR_CLIP 100.0f

#define SCALE_FACTOR 2.992e10 // Scale down positions for better numerical stability

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint loadBMP_custom(const char *filepath);
GLuint loadDDS(const char *filepath);

#endif //COMMON_H
