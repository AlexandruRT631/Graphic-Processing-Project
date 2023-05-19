//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 1600;
int glWindowHeight = 900;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.2f;

bool pressedKeys[1024];
float angleY = 90.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D buildings;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D car;
gps::Model3D wheelFrontLeft;
gps::Model3D wheelFrontRight;
gps::Model3D wheelBackLeft;
gps::Model3D wheelBackRight;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

double lastX = 400;
double lastY = 300;
float mouseSensitivity = 0.05;

glm::mat4 carStartPos;
glm::mat4 carPos;
int remainingTime = 600;

bool autoCamera = false;
glm::vec3 oldCameraPosition;
glm::vec3 oldCameraTarget;
glm::vec3 oldCameraUp;
int cameraAnimationState = 0;
int remainingTimeCamera = 475;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	projection = glm::perspective(glm::radians(45.0f),
		(float)width / (float)height / 2,
		0.1f, 800.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		showDepthMap = false;
	}

	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		showDepthMap = false;
	}

	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		showDepthMap = false;
	}

	if (pressedKeys[GLFW_KEY_V]) {
		showDepthMap = true;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_Y]) {
		autoCamera = !autoCamera;
		if (autoCamera) {
			oldCameraPosition = myCamera.getCameraPosition();
			oldCameraTarget = myCamera.getCameraTarget();
			oldCameraUp = myCamera.getCameraUp();
			myCamera = gps::Camera(glm::vec3(3.0f, 1.0f, 60.0f), glm::vec3(3.0f, 1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			cameraAnimationState = 0;
			remainingTimeCamera = 475;
		}
		else {
			myCamera = gps::Camera(oldCameraPosition, oldCameraTarget, oldCameraUp);
		}
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	double xdiff = (lastX - xpos) * mouseSensitivity;
	double ydiff = (lastY - ypos)* mouseSensitivity;

	lastX = xpos;
	lastY = ypos;
	
	if (!autoCamera) {
		myCamera.rotate(ydiff, xdiff);
	}
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W] && !autoCamera) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S] && !autoCamera) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A] && !autoCamera) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D] && !autoCamera) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_Q] && !autoCamera) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_E] && !autoCamera) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_M]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_N]) {
		angleY -= 1.0f;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("objects/nanosuit/nanosuit.obj");
	buildings.LoadModel("objects/buildings/buildings.obj");
	ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	car.LoadModel("objects/car/car.obj");
	wheelBackLeft.LoadModel("objects/wheelBackLeft/wheelBackLeft.obj");
	wheelBackRight.LoadModel("objects/wheelFrontRight/wheelFrontRight.obj");
	wheelFrontLeft.LoadModel("objects/wheelBackLeft/wheelBackLeft.obj");
	wheelFrontRight.LoadModel("objects/wheelFrontRight/wheelFrontRight.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	carPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	carPos = glm::scale(carPos, glm::vec3(0.8f, 0.8f, 0.8f));
	carPos = glm::translate(carPos, glm::vec3(-6.5f, -1.275f, -65.0f));
	carStartPos = carPos;
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO

	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = -70.0f, far_plane = 70.0f;
	glm::mat4 lightProjection = glm::ortho(-70.0f, 70.0f, -70.0f, 70.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();
	buildings.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 4.0f));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	nanosuit.Draw(shader);


	model = carPos;
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	car.Draw(shader);
	wheelBackLeft.Draw(shader);
	wheelFrontRight.Draw(shader);
	model = carPos;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.05f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	wheelBackRight.Draw(shader);
	model = carPos;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.05f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	wheelFrontLeft.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ground.Draw(shader);
}

void initSkybox() {
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void carAnimation() {
	if (remainingTime > 0) {
		remainingTime--;
		carPos = glm::translate(carPos, glm::vec3(0.0f, 0.0f, 0.25f));
	}
	else {
		remainingTime = 600;
		carPos = carStartPos;
	}
}

void cameraAnimation() {
	if (autoCamera) {
		if (remainingTimeCamera > 0 && cameraAnimationState % 2 == 0) {
			remainingTimeCamera--;
			myCamera.move(gps::MOVE_FORWARD, 0.25f);
		}
		else if (remainingTimeCamera > 0 && cameraAnimationState % 2 == 1 && cameraAnimationState < 6) {
			remainingTimeCamera--;
			myCamera.rotate(0.0f, 6.0f);
		}
		else if (remainingTimeCamera > 0 && cameraAnimationState % 2 == 1 && cameraAnimationState > 6) {
			remainingTimeCamera--;
			myCamera.rotate(0.0f, -6.0f);
		}
		else {
			cameraAnimationState = (cameraAnimationState + 1) % 12;
			switch (cameraAnimationState) {
			case 0:
				remainingTimeCamera = 475;
				break;
			case 1:
				remainingTimeCamera = 15;
				break;
			case 2:
				remainingTimeCamera = 225;
				break;
			case 3:
				remainingTimeCamera = 15;
				break;
			case 4:
				remainingTimeCamera = 225;
				break;
			case 5:
				remainingTimeCamera = 15;
				break;
			case 6:
				remainingTimeCamera = 400;
				break;
			case 7:
				remainingTimeCamera = 15;
				break;
			case 8:
				remainingTimeCamera = 250;
				break;
			case 9:
				remainingTimeCamera = 15;
				break;
			case 10:
				remainingTimeCamera = 175;
				break;
			case 11:
				remainingTimeCamera = 15;
				break;
			default:
				break;
			}
		}
	}
}

void renderScene() {
	carAnimation();
	cameraAnimation();
	
	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, false);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// final scene rendering pass (with shadows)

	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myCustomShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	// bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	// point lights
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[0].position"), 1, glm::value_ptr(glm::vec3(-0.35f, 2.1f, 2.9f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[0].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[1].position"), 1, glm::value_ptr(glm::vec3(0.05f, 2.1f, -8.8f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[1].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[2].position"), 1, glm::value_ptr(glm::vec3(12.2f, 2.1f, -7.9f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[2].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[3].position"), 1, glm::value_ptr(glm::vec3(11.6f, 2.1f, 4.3f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[3].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[4].position"), 1, glm::value_ptr(glm::vec3(-0.75f, 2.1f, -54.55f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[4].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[5].position"), 1, glm::value_ptr(glm::vec3(-47.75f, 2.1f, -53.45f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[5].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[6].position"), 1, glm::value_ptr(glm::vec3(-46.8f, 2.1f, -7.85f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[6].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[7].position"), 1, glm::value_ptr(glm::vec3(-47.4f, 2.1f, 4.3f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[7].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[8].position"), 1, glm::value_ptr(glm::vec3(6.6f, 2.1f, -54.3f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLights[8].color"), 1, glm::value_ptr(glm::vec3(255.0f, 0.0f, 0.0f)));

	drawObjects(myCustomShader, false);

	//draw a white cube around the light

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = lightRotation;
	model = glm::translate(model, 50.0f * lightDir);
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);
	mySkyBox.Draw(skyboxShader, view, projection);

	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}

	// camera position
	// glm::vec3 cameraPosition = myCamera.getCameraPosition();
	// std::cout << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << std::endl;
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
