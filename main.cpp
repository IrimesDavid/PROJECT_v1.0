#include<filesystem>
namespace fs = std::filesystem;

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#include <glad/glad.h> // GLAD includes OpenGL headers internally
#endif
#include "Mesh.h" // this one contains all the other headers
#include "Model3D.hpp"
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//DATA

const unsigned int width = 1600;
const unsigned int height = 900;

int retina_width, retina_height; //for proper scaling

float currentTime = glfwGetTime();
float lastTime = 0.0f;
float deltaTime;

GLFWwindow* window;
Camera camera(width, height, glm::vec3(0.0f, 2.0f, 2.0f));
//Mouse movement
float lastX = 400, lastY = 300; // Initial center position (assuming 800x600 window)
bool firstMouse = true;          // Tracks if it's the first mouse movement
bool cursorState = true;

//Light
glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 1.2f, 0.8f);
glm::vec3 lightRot = glm::vec3(0.0f, -1.0f, 0.0f);
glm::mat4 lightModel = glm::mat4(1.0f);
float lightInten = 1.0f;
int lightType = 1;
bool modifyLight = false;

// Outline
glm::vec4 outlineColor = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);
glm::mat4 outlineModel = glm::mat4(1.0f);

//3D Objects
Model3D nanosuit;
Model3D ground;
Model3D monkey;
Model3D lightbulb;

//SHADERS
Shader lightShader;
Shader shaderProgram;
Shader outlineShader;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS

void windowResizeCallback(GLFWwindow* window, int newWidth, int newHeight) {

	if (newWidth == 0 || newHeight == 0) {
		return;  // Avoid setting an invalid viewport when the window is minimized
	}

	float viewportWidth = newWidth;
	float viewportHeight = newHeight;

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

	bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	bool altPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

	//CURSOR VISIBILITY
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		cursorState = true;
	}

	//LIGHT POSITION / ROTATION
	if (shiftPressed && key == GLFW_KEY_UP && action == GLFW_PRESS) {
		lightPos.y += 0.1;  // UP
		modifyLight = true;
	}
	else if (altPressed && key == GLFW_KEY_UP && action == GLFW_PRESS) {
		lightRot.z = fmod(lightRot.z - 0.25f, -3.0f); // ROTATE FORWARD
;		modifyLight = true;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		lightPos.z -= 0.1;  // FORWARD
		modifyLight = true;
	}
	if (shiftPressed && key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		lightPos.y -= 0.1;  // DOWN
		modifyLight = true;
	}
	else if (altPressed && key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		lightRot.z = fmod(lightRot.z + 0.25f, 3.0f);;  // ROTATE BACKWARD
		modifyLight = true;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		lightPos.z += 0.1;  // BACKWARD
		modifyLight = true;
	}
	if (altPressed && key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		lightRot.x = fmod(lightRot.x + 0.25f, 3.0f);; // ROTATE RIGHT
		modifyLight = true;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		lightPos.x += 0.1; // RIGHT
		modifyLight = true;
	}
	if (altPressed && key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		lightRot.x = fmod(lightRot.x - 0.25f, -3.0f);; // ROTATE LEFT
		modifyLight = true;
	}
	else if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		lightPos.x -= 0.1; // LEFT
		modifyLight = true;
	}

	//LIGHT TYPE
	if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS) {
		lightType = 1;
		shaderProgram.Activate();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "lightType"), lightType);
	}
	else if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) {
		lightType = 2;
		shaderProgram.Activate();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "lightType"), lightType);
	}
	else if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS) {
		lightType = 3;
		shaderProgram.Activate();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "lightType"), lightType);
	}

	//LIGHT INTENSITY
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS) {
		lightInten -= 0.5; // DECREASE
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "lightInten"), lightInten);
	}
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS) {
		lightInten += 0.5; // INCREASE
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "lightInten"), lightInten);
	}

	if (modifyLight) {
		lightModel = glm::mat4(1.0f);  // Reset to identity matrix
		lightModel = glm::translate(lightModel, lightPos);
		lightShader.Activate();
		glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		shaderProgram.Activate();
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightRot"), lightRot.x, lightRot.y, lightRot.z);

		modifyLight = false;
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

		// Update the camera rotation based on new yaw and pitch
		camera.Rotate(pitch, yaw);
	}
}

void handleEvents(Shader shader, bool firstCall) {

	// Makes sure to not execute all these more than once in a single while iteration (only true when calling the first time in the while loop)
	if (firstCall) {
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		camera.Move(window, deltaTime);
		camera.updateMatrix(45.0f, 0.1f, 1000.0f);

		glfwSetWindowSizeCallback(window, windowResizeCallback);
		glfwSetKeyCallback(window, keyboardCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, mouseCursorCallback);

		glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
		camera.Matrix(shader, "camMatrix");
	}
	else {
		camera.Matrix(shader, "camMatrix");
	}
}

void drawWithOutline(Model3D *model) {

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	model->Draw(shaderProgram, camera);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	outlineShader.Activate();
	model->Draw(outlineShader, camera);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);

	shaderProgram.Activate();
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//MAIN
int main()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	window = glfwCreateWindow(width, height, "OpenGL_v1.0", NULL, NULL);
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
	// enables backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	// Enables gamma correction when writing to an sRGB framebuffer
	glEnable(GL_FRAMEBUFFER_SRGB);
	// Enables stencil buffer
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);

	// Shader for the object(s)
	shaderProgram = Shader("Shaders/default.vert", "Shaders/default.frag");
	// Shader for light cube
	lightShader = Shader("Shaders/light.vert", "Shaders/light.frag");
	// Shader for the outlining of the objects
	outlineShader = Shader("Shaders/outline.vert", "Shaders/outline.frag");
	
	//------------------------------------------------------------------------------------

	lightModel = glm::translate(lightModel, lightPos);

	glm::vec3 objPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 objModel = glm::mat4(1.0f);
	objModel = glm::translate(objModel, objPos);

	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(objModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightRot"), lightRot.x, lightRot.y, lightRot.z);
	glUniform1i(glGetUniformLocation(shaderProgram.ID, "lightType"), lightType);
	glUniform1f(glGetUniformLocation(shaderProgram.ID, "lightInten"), lightInten);

	outlineShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(outlineShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(outlineModel));
	glUniform3f(glGetUniformLocation(outlineShader.ID, "camPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
	glUniform4f(glGetUniformLocation(outlineShader.ID, "outlineColor"), outlineColor.x, outlineColor.y, outlineColor.z, outlineColor.w);
	glUniform1f(glGetUniformLocation(outlineShader.ID, "outline"), 0.03f);

	//------------------------------------------------------------------------------------

	// Loads models
	nanosuit.LoadModel("Resources/nanosuit/nanosuit2.obj");
	ground.LoadModel("Resources/parking_lot/parking_lot.obj");
	//monkey.LoadModel("Resources/monkey/monkey.obj");
	lightbulb.LoadModel("Resources/lightbulb/lightbulb.obj");

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Specify the color of the background
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		// Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();	
		//handles all Inputs
		handleEvents(shaderProgram, true);

		//nanosuit.Draw(shaderProgram, camera);
		ground.Draw(shaderProgram, camera);
		//monkey.Draw(shaderProgram, camera);
		lightbulb.Draw(lightShader, camera);
		drawWithOutline(&nanosuit);

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