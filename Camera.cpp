#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	
	Camera::width = width;
	Camera::height = height;
	Camera::cameraPosition = position;

	// Calculate the initial camera front direction
	cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
	// Calculate the initial right direction
	cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
	// Makes camera look in the right direction from the right position
	view = glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	// Sets new camera matrix
	cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform) {
	// Exports camera matrix
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}


void Camera::Move(GLFWwindow* window, float deltaTime) {

	// Camera movement
	float normalSpeed = speed * deltaTime;
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPosition += normalSpeed * cameraFrontDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPosition += normalSpeed * -cameraRightDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPosition += normalSpeed * -cameraFrontDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPosition += normalSpeed * cameraRightDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cameraPosition += normalSpeed * cameraUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		cameraPosition += normalSpeed * -cameraUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed = 4.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		speed = 1.0f;
	}
}

	//Camera rotation
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis
void Camera::Rotate(float pitch, float yaw) {
	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFrontDirection = glm::normalize(front);

	// Recalculate right and up directions
	cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
	cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
}

bool Camera::playAnimation() {
	static float animationProgress = 0.0f; // Progress of the animation (0.0 to 1.0)
	const float animationDuration = 300.0f; // Total duration of the animation in seconds
	const float rotationSpeed = 45.0f;    // Degrees per second for camera rotation

	// Calculate animation step
	float animationStep = 1.0f / (animationDuration * 2.0f);

	// Update animation progress
	animationProgress += animationStep * speed;

	// Smooth circular motion around a target point
	float radius = 15.0f; // Distance from target
	float angle = glm::radians(360.0f * animationProgress); // Circular path

	// Calculate camera position (circle around origin)
	cameraPosition.x = radius * cos(angle);
	cameraPosition.z = radius * sin(angle);
	cameraPosition.y = 10.0f; // Elevate slightly for dynamic motion

	// Rotate camera to look at the origin
	float pitch = -25.0f;    // Fixed pitch for downward view
	float yaw = glm::degrees(angle) + 180.0f; // Face towards the center
	Rotate(pitch, yaw);

	// End animation when progress reaches 1.0
	if (animationProgress >= 1.0f) {
		animationProgress = 0.0f; // Reset for the next animation
		return true;             // Animation completed
	}

	return false; // Animation still running
}
