#pragma once

#include <string>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

class GlHelper
{
public:
	static void error_callback(int error, const char* description);
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
	static std::string readFile(const char *fileName);
	static float randrange(float fMin, float fMax);


	static void printProgramLog(GLuint program);
	static void printShaderLog(GLuint shader);

	GlHelper();
	~GlHelper();
};

