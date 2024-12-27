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
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

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