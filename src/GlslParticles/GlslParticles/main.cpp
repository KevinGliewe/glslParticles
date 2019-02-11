#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <ctime>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Window.h"
#include "GlHelper.h"
#include "Camera.h"



const float toRadians = 3.14159265f / 180.0f;


GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat currentTime = 0.0f;

// Compute Shader
static const char* cShader = "../../assets/shaders/particleEmitter.cs.glsl";

// Vertex Shader
static const char* vShader = "../../assets/shaders/particleEmitter.vert.glsl";

// Fragment Shader
static const char* fShader = "../../assets/shaders/particleEmitter.frag.glsl";

const int particleCount = 1000;
const float particleLifeTime = 0.5f;

GLuint computeProgram;
GLuint shaderProgram;

GLuint vao;
GLuint posSSBO;
GLuint velSSBO;
GLuint lifeSSBO;

Window mainWindow;
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

void loadComputeShader() {
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

	// Read shaders
	std::string computeShaderStr = GlHelper::readFile(cShader);
	const char *computeShaderSrc = computeShaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	printf("Compiling compute shader.\n");
	glShaderSource(computeShader, 1, &computeShaderSrc, NULL);
	glCompileShader(computeShader);

	GlHelper::printShaderLog(computeShader);

	printf("Linking program.\n");
	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);
	GlHelper::printProgramLog(computeProgram);

	glDeleteShader(computeShader);
}

void loadDrawShader() {
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shaders
	std::string vertShaderStr = GlHelper::readFile(vShader);
	std::string fragShaderStr = GlHelper::readFile(fShader);
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	// Compile vertex shader
	printf("Compiling vertex shader.\n");
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);
	GlHelper::printShaderLog(vertShader);

	// Compile fragment shader
	printf("Compiling fragment shader.\n");
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);
	GlHelper::printShaderLog(fragShader);

	printf("Linking program.\n");
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	GlHelper::printProgramLog(shaderProgram);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void resetPositionSSBO() {
	glm::vec4* pos = (glm::vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		pos[i].x = -999999.0f;
		pos[i].y = -999999.0f;
		pos[i].z = -999999.0f;
		pos[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void resetVelocitySSBO() {
	glm::vec4* vel = (glm::vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		vel[i].x = 0.0f;
		vel[i].y = 0.0f;
		vel[i].z = 0.0f;
		vel[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void resetLifeTimeSSBO() {
	GLfloat* life = (GLfloat*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(GLfloat), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		life[i] = GlHelper::randrange(0.0f, particleLifeTime);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void resetBuffers()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	resetPositionSSBO();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
	resetVelocitySSBO();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifeSSBO);
	resetLifeTimeSSBO();
}

void generateBuffers() {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Position SSBO
	if (glIsBuffer(posSSBO)) {
		glDeleteBuffers(1, &posSSBO);
	};
	glGenBuffers(1, &posSSBO);
	// Bind to SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	// Generate empty storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleCount * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
	// Fill
	resetPositionSSBO();
	// Bind buffer to target index 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);


	// Velocity SSBO
	if (glIsBuffer(velSSBO)) {
		glDeleteBuffers(1, &velSSBO);
	};
	glGenBuffers(1, &velSSBO);
	// Bind to SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
	// Generate empty storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleCount * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
	// Fill
	resetVelocitySSBO();
	// Bind buffer to target index 1
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);


	// Lifetime SSBO
	if (glIsBuffer(lifeSSBO)) {
		glDeleteBuffers(1, &lifeSSBO);
	};
	glGenBuffers(1, &lifeSSBO);
	// Bind to SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifeSSBO);
	// Generate empty storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleCount * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// Fill
	resetLifeTimeSSBO();
	// Bind buffer to target index 1
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lifeSSBO);
}



int main() 
{
	std::srand(std::time(nullptr));

	/*
	//Set the error callback
	glfwSetErrorCallback(GlHelper::error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Request an OpenGL 4.3 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	//Create a window and create its OpenGL context
//	window = glfwCreateWindow(1920, 1200, "OpenGL Compute Shader Particle System", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(1280, 720, "OpenGL Compute Shader Particle System", NULL, NULL);

	//If the window couldn't be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, GlHelper::framebuffer_size_callback);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	// Set debug callback
	if (glDebugMessageCallback != NULL) {
		glDebugMessageCallback(GlHelper::glDebugCallback, NULL);
	}
	glEnable(GL_DEBUG_OUTPUT);

	// Output some info on the OpenGL implementation
	const GLubyte* glvendor = glGetString(GL_VENDOR);
	const GLubyte* glrenderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);

	printf("GL_VENDOR: %s\n", glvendor);
	printf("GL_RENDERER: %s\n", glrenderer);
	printf("GL_VERSION: %s\n", glversion);
	*/

	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	loadComputeShader();
	loadDrawShader();

	generateBuffers();

	
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0));
	glm::mat4 view = camera.calculateViewMatrix();
	glm::mat4 projection = glm::perspective(45.0f * toRadians, 1280.0f / 720.0f, 0.1f, 100.0f);

	lastTime = glfwGetTime();
	//Main Loop
	do
	{
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		view = camera.calculateViewMatrix();

		// Physics Step

		glUseProgram(computeProgram);

		glUniform1f(glGetUniformLocation(computeProgram, "dt"), deltaTime);
		glUniform3f(glGetUniformLocation(computeProgram, "g"), 0, -9.81f, 0);
		glUniform3f(glGetUniformLocation(computeProgram, "velSpawn"), 0, 1.0f, 0);
		glUniform1f(glGetUniformLocation(computeProgram, "lifeTime"), particleLifeTime);
		glUniform1f(glGetUniformLocation(computeProgram, "randseed"), GlHelper::randrange(0, 1.0f));

		int workingGroups = particleCount / 16;
		glDispatchCompute(workingGroups, 1, 1);

		glUseProgram(0);

		// Set memory barrier on per vertex base to make sure we get what was written by the compute shaders
		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

		// Draw Step
		glUseProgram(shaderProgram);

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection") , 1, GL_FALSE, glm::value_ptr(projection));
		
		glGetError();

		// Bind Pos Buffer
		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		GLuint posLocation = glGetAttribLocation(shaderProgram, "pos");
		glVertexAttribPointer(posLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posLocation);

		glPointSize(5);
		glDrawArrays(GL_POINTS, 0, particleCount);

		//glfwSwapBuffers(mainWindow.);
		mainWindow.swapBuffers();
	} //Check if the ESC key had been pressed or if the window had been closed
	while (!mainWindow.getShouldClose());
	//while (!glfwWindowShouldClose(window));

	//Close OpenGL window and terminate GLFW
	//glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);

	return 0;
}