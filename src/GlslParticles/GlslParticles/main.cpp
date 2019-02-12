#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <ctime>
#include <limits>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Window.h"
#include "GlHelper.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



const float toRadians = 3.14159265f / 180.0f;


GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat currentTime = 0.0f;

// Compute Shader
static const char* cShader = "../../assets/shaders/particleEmitter.cs.glsl";

// Vertex Shader
static const char* vShader = "../../assets/shaders/particleEmitter.vert.glsl";

// Geometry Shader
static const char* gShader = "../../assets/shaders/particleEmitter.geom.glsl";

// Fragment Shader
static const char* fShader = "../../assets/shaders/particleEmitter.frag.glsl";

// Texture File
static const char* textureFile = "../../assets/Textures/fish.png";

const int particleCount = 20000;
const float particleLifeTime = 0.8f;

GLuint computeProgram;
GLuint shaderProgram;

GLuint vao;
GLuint posSSBO;
GLuint velSSBO;
GLuint lifeSSBO;

GLuint textureID;

Window mainWindow;
Camera camera = Camera(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

void loadComputeShader() {
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

	// Read shaders
	std::string computeShaderStr = GlHelper::readFile(cShader);
	const char *computeShaderSrc = computeShaderStr.c_str();

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
	GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shaders from files
	std::string vertShaderStr = GlHelper::readFile(vShader);
	std::string geomShaderStr = GlHelper::readFile(gShader);
	std::string fragShaderStr = GlHelper::readFile(fShader);
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *geomShaderSrc = geomShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	// Compile vertex shader
	printf("Compiling vertex shader.\n");
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);
	GlHelper::printShaderLog(vertShader);

	// Compile geometry shader
	printf("Compiling geometry shader.\n");
	glShaderSource(geomShader, 1, &geomShaderSrc, NULL);
	glCompileShader(geomShader);
	GlHelper::printShaderLog(geomShader);

	// Compile fragment shader
	printf("Compiling fragment shader.\n");
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);
	GlHelper::printShaderLog(fragShader);

	printf("Linking program.\n");
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, geomShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	GlHelper::printProgramLog(shaderProgram);

	// Cleanup
	glDeleteShader(vertShader);
	glDeleteShader(geomShader);
	glDeleteShader(fragShader);
}

void loadTexture() {
	int width = 0;
	int height = 0;
	int bitDepth = 0;

	unsigned char *textureData = stbi_load(textureFile, &width, &height, &bitDepth, 0);
	if (!textureData)
	{
		printf("Failed to find: %s\n", textureFile);
		return;
	}


	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(textureData);
}

void resetPositionSSBO() {
	// Place particles far away until they respawn for the first time
	float fMax = std::numeric_limits<float>::max();
	glm::vec4* pos = (glm::vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		pos[i].x = fMax;
		pos[i].y = fMax;
		pos[i].z = fMax;
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
	// Init the lifetime with a random value between 0 and particleLifeTime, so we get an constant stream respawing.
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
	// Bind buffer to target index 2
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lifeSSBO);
}



int main() 
{
	// Initialize rand seed
	std::srand(std::time(nullptr));

	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	loadComputeShader();
	loadDrawShader();
	loadTexture();

	generateBuffers();

	// MVP
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


		// ######################################################
		// #                 Physics Step                       #
		// ######################################################
		glUseProgram(computeProgram);

		// Set uniform variables
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

		// ######################################################
		// #                   Draw Step                        #
		// ######################################################
		glUseProgram(shaderProgram);

		glDisable(GL_DEPTH_TEST);

		// Set uniform variables
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection") , 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1f(glGetUniformLocation(shaderProgram, "particle_size"), 0.01);

		// Bind fish texture for billboarding
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Bind Pos Buffer
		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		GLuint posLocation = glGetAttribLocation(shaderProgram, "pos");
		glVertexAttribPointer(posLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posLocation);

		// Draw the Particles
		glDrawArrays(GL_POINTS, 0, particleCount);

		mainWindow.swapBuffers();
		mainWindow.update();
	} //Check if the ESC key had been pressed or if the window had been closed
	while (!mainWindow.getShouldClose());

	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);

	return 0;
}