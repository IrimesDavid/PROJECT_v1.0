#include<filesystem>
namespace fs = std::filesystem;

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#include <glad/glad.h> // GLAD includes OpenGL headers internally
#endif
#include "Mesh.h" // this one contains all the other headers
#include "Model3D.hpp"
#include "Light.h"
#include "Skybox.h"
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//DATA

int viewportWidth = 1600;
int viewportHeight = 900;

int retina_width, retina_height; //for proper scaling

float currentTime = glfwGetTime();
float lastTime = 0.0f;
float deltaTime;

GLFWwindow* window;
Camera camera(viewportWidth, viewportHeight, glm::vec3(0.0f, 2.0f, 2.0f));
//Mouse movement
float lastX = 800, lastY = 450; // Initial center position (assuming 800x600 window)
bool firstMouse = true;          // Tracks if it's the first mouse movement
bool cursorState = true;
glm::vec2 savedCursorPos;

//Lights
std::vector<Light> lights;
int selectedLightIndex = 0;
bool altPressed = false;
bool hideLightObjects = false;

// Outline
glm::vec4 outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
glm::mat4 outlineModel = glm::mat4(1.0f);

//3D Objects
Model3D nanosuit;
Model3D ground;
Model3D glass;
glm::mat4 objModel = glm::mat4(1.0f);
int rasterizeMode = 0;

//SHADERS
Shader lightShader;
Shader shaderProgram;
Shader outlineShader;
Shader skyboxShader;
Shader shadowShader;

//SKYBOX
SkyBox mySkyBox;

// for anti-aliasing
unsigned int samples = 8;

// SHADOW
//FRAME BUFFER FOR SHADOW MAP (DIRECTIONAL LIGHTS) -> orthogonal projection
unsigned int shadowMapFBO;
unsigned int shadowMapWidth = 4096, shadowMapHeight = 4096;
unsigned int shadowMap;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS

void windowResizeCallback(GLFWwindow* window, int newWidth, int newHeight) {

	if (newWidth == 0 || newHeight == 0) {
		return;  // Avoid setting an invalid viewport when the window is minimized
	}

	viewportWidth = newWidth;
	viewportHeight = newHeight;

	// Calculate the offset to center the viewport (horizontal centering)
	int offsetX = (newWidth - viewportWidth) / 2;
	int offsetY = 0;

	// Update the viewport to cover the new window size, with centering
	glViewport(offsetX, offsetY, viewportWidth, viewportHeight);

	// Update the camera's aspect ratio
	camera.width = newWidth;
	camera.height = newHeight;

	// Check if the window is maximized
	bool isMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
	if (isMaximized) {
		// Make the cursor invisible
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else {
		// Restore the cursor visibility
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	// Cycle through rasterizeModes
	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
		rasterizeMode = (rasterizeMode + 1) % 3; //for now only 2 modes: with or without outline
		if (rasterizeMode == 2)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Check if Alt is pressed
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
		if (!altPressed) {
			altPressed = true;

			// Save the current cursor position when Alt is first pressed
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			savedCursorPos = glm::vec2(xpos, ypos);
		}
	}
	else if (altPressed) {
		// When Alt is released, restore cursor position
		altPressed = false;

		// Reset cursor to saved position
		glfwSetCursorPos(window, savedCursorPos.x, savedCursorPos.y);
	}

	// CURSOR VISIBILITY
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		cursorState = true;
	}

	// LIGHT OBJECTS VISIBILITY
	if (key == GLFW_KEY_KP_0 && action == GLFW_PRESS) {
		hideLightObjects = !hideLightObjects;
	}

	// ADD LIGHT
	if (key == GLFW_KEY_KP_ENTER && action == GLFW_PRESS) {
		lights.push_back(Light(camera.cameraPosition, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 2));
		selectedLightIndex = lights.size() - 1;
		for(int i = 0; i < lights.size(); ++i)
			lights[i].lightbulb.LoadModel("Resources/lightbulb/lightbulb.obj");
	}

	// REMOVE LIGHT
	if (key == GLFW_KEY_KP_DECIMAL && action == GLFW_PRESS) {
		if (lights.size() >= 1) {
			if (lights.size() == 1) {
				lights.clear();

				shaderProgram.Activate();
				glUniform4f(glGetUniformLocation(shaderProgram.ID, "lights[0].lightColor"), 0.0f, 0.0f, 0.0f, 0.0f);
			}
			else {
				swap(lights[selectedLightIndex], lights.back());
				lights.pop_back();
				selectedLightIndex %= lights.size();
			}
		}
	}

	// Cycle through lights
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		if(!lights.empty())
			selectedLightIndex = (selectedLightIndex + 1) % lights.size();
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstMouse = true; // Reset for smooth mouse movement
		cursorState = false;
	}
}

void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) {
	static float yaw = -90.0f; // Initial yaw (facing -z)
	static float pitch = 0.0f; // Initial pitch (looking straight ahead)
	if (!cursorState) {
		// If this is the first frame, just update the initial position and return
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
			return;
		}

		// Calculate offsets
		float xOffset = xpos - lastX;
		float yOffset = lastY - ypos; // Reversed: y-coordinates range bottom to top
		lastX = xpos;
		lastY = ypos;

		// Apply sensitivity factor
		xOffset *= camera.sensitivity;
		yOffset *= camera.sensitivity;

		// Update yaw and pitch angles
		yaw += xOffset;
		pitch += yOffset;

		// Constrain the pitch to avoid screen flip
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

	
		if (altPressed) {
			if(!lights.empty())
				lights[selectedLightIndex].Rotate(pitch, yaw);

		}
		else
			// Update the camera rotation based on new yaw and pitch
			camera.Rotate(pitch, yaw);
	}
}

int initOpenGL() {
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//for anti-aliasing
	glfwWindowHint(GLFW_SAMPLES, samples);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of, naming it "OpenGL_v1"
	window = glfwCreateWindow(viewportWidth, viewportHeight, "OpenGL_v1.0", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	// enables the DepthBuffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// for anti-aliasing
	glEnable(GL_MULTISAMPLE);

	// enables backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Enables gamma correction when writing to an sRGB framebuffer
	//glEnable(GL_FRAMEBUFFER_SRGB);
	// 
	// Enables stencil buffer
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// for blending (semi-transparent objects) //it doesnt work >:(
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// Specify the viewport of OpenGL in the Window
	glViewport(0, 0, viewportWidth, viewportHeight);

	// init Shadow Map
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}

void loadShaders() {
	// Shader for the object(s)
	shaderProgram = Shader("Shaders/default.vert", "Shaders/default.frag");
	// Shader for light cube
	lightShader = Shader("Shaders/light.vert", "Shaders/light.frag");
	// Shader for the outlining of the objects
	outlineShader = Shader("Shaders/outline.vert", "Shaders/outline.frag");
	// Shader for the skybox
	skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	// Shader for cast shadows
	shadowShader = Shader("Shaders/shadow.vert", "Shaders/shadow.frag");
}

void initShaders() {
	//if we want to move the whole scene (all the ordinary objects)
	//glm::vec3 objPos = glm::vec3(0.0f, 0.0f, 0.0f);
	//objModel = glm::translate(objModel, objPos);

	// OBJECTS
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(objModel));

	// LIGHTS
	lights.push_back(Light(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 2));
	for (int i = 0; i < lights.size(); ++i)
		lights[i].applyUniforms(shaderProgram, shadowShader, i, lights.size());

	// OUTLINE
	outlineShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(outlineShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lights[selectedLightIndex].modelMatrix));
	glUniform3f(glGetUniformLocation(outlineShader.ID, "camPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
	glUniform4f(glGetUniformLocation(outlineShader.ID, "outlineColor"), outlineColor.x, outlineColor.y, outlineColor.z, outlineColor.w);
	glUniform1f(glGetUniformLocation(outlineShader.ID, "outline"), 0.03f);

	//SKYBOX
	mySkyBox.setSkybox("Resources/skybox/skybox2/");

	//SHADOW
	shadowShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(objModel));

	//for (int i = 0; i < lights.size(); ++i) {
	//	glUniformMatrix4fv(glGetUniformLocation(shadowShader.ID, ("lightProjection[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(lights[i].lightProjectionMat));
	//}

}

void loadModels() {
	// Loads ordinary objects
	nanosuit.LoadModel("Resources/nanosuit/nanosuit2.obj");
	ground.LoadModel("Resources/parking_lot/parking_lot.obj");
	glass.LoadModel("Resources/glass/glass.obj");

	// Load light objects
	for (int i = 0; i < lights.size(); ++i)
		lights[i].lightbulb.LoadModel("Resources/lightbulb/lightbulb.obj");
}

void handleEvents(Shader shader, bool firstCall) {

	// Makes sure to not execute all these more than once in a single while iteration (only true when calling the first time in the while loop)
	if (firstCall) {
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		camera.Move(window, deltaTime);
		camera.updateMatrix(45.0f, 0.1f, 1000.0f);

		if (!lights.empty()) {
			lights[selectedLightIndex].Modify(window, deltaTime, camera.cameraFrontDirection, camera.cameraRightDirection);
			lights[selectedLightIndex].applyUniforms(shaderProgram, shadowShader, selectedLightIndex, lights.size());
			lights[selectedLightIndex].updateMatrix(90.0f, 0.5f, 100.0f);

			shadowShader.Activate();
			glUniformMatrix4fv(glGetUniformLocation(shadowShader.ID, ("lightProjection[" + std::to_string(selectedLightIndex) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(lights[selectedLightIndex].lightProjectionMat));

			shaderProgram.Activate();
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, ("lightProjection[" + std::to_string(selectedLightIndex) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(lights[selectedLightIndex].lightProjectionMat));
		}
		glfwSetKeyCallback(window, keyboardCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, mouseCursorCallback);
		glfwSetWindowSizeCallback(window, windowResizeCallback);

		glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
		camera.Matrix(shader, "camMatrix");
	}
	else {
		camera.Matrix(shader, "camMatrix");
	}
}

void drawObjectWithOutline(Model3D* model, Shader& shader) {

	shader.Activate();
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	model->Draw(shader, camera);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	outlineShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(outlineShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(objModel));
	model->Draw(outlineShader, camera);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);

}

void drawLightWithOutline(Model3D *model, Shader &shader) {
	if (!lights.empty()) {
		shader.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lights[selectedLightIndex].modelMatrix));
		glUniform4f(glGetUniformLocation(shader.ID, "lightColor"), lights[selectedLightIndex].color.r, lights[selectedLightIndex].color.g, lights[selectedLightIndex].color.b, 1.0f);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		model->Draw(shader, camera);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		outlineShader.Activate();
		glUniformMatrix4fv(glGetUniformLocation(outlineShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lights[selectedLightIndex].modelMatrix));

		model->Draw(outlineShader, camera);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glEnable(GL_DEPTH_TEST);
	}
}

void drawLights() {
	if (!lights.empty()) {
		for (int i = 0; i < lights.size(); ++i)
			if (i != selectedLightIndex) {
				glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lights[i].modelMatrix));
				glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lights[i].color.r, lights[i].color.g, lights[i].color.b, lights[i].color.a);

				lights[i].lightbulb.Draw(lightShader, camera);
			}
		drawLightWithOutline(&lights[selectedLightIndex].lightbulb, lightShader);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN
int main()
{
	initOpenGL();
	loadShaders();
	initShaders();
	loadModels();


	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Specify the color of the backgrounds
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		//handles all Inputs
		handleEvents(shaderProgram, true);

		// Draw the skybox
		mySkyBox.Draw(skyboxShader, camera.view, camera.projection);

		// Draw cast shadows
		glViewport(0, 0, shadowMapWidth, shadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadowShader.Activate();

		nanosuit.Draw(shadowShader, camera);
		ground.Draw(shadowShader, camera);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, viewportWidth, viewportHeight);

		//Draw ordinary objects
		// Send the light matrix to the shader
		shaderProgram.Activate();

		// Bind the Shadow Map
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), 2);

		switch (rasterizeMode) {
			case 0:
				nanosuit.Draw(shaderProgram, camera);
				ground.Draw(shaderProgram, camera);
				break;
			case 1:
				drawObjectWithOutline(&nanosuit, shaderProgram);
				drawObjectWithOutline(&ground, shaderProgram);
				break;
			default:
				nanosuit.Draw(shaderProgram, camera);
				ground.Draw(shaderProgram, camera);
				break;
		}

		// Draw transparent objects last
		glass.Draw(shaderProgram, camera);

		//Draw light objects
		if (!hideLightObjects) {
			lightShader.Activate();
			drawLights();
		}

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	// Delete all the objects we've created
	shaderProgram.Delete();
	lightShader.Delete();
	outlineShader.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}