#include "Light.h"

Light::Light(glm::vec3 position, glm::vec4 color) {

	Light::lightPosition = position;
	Light::lightColor = color;

	// Calculate the initial light front direction
	lightFrontDirection = glm::normalize(lightTarget - lightPosition);
	// Calculate the initial right direction
	lightRightDirection = glm::normalize(glm::cross(lightFrontDirection, lightUpDirection));
}

void Light::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Makes light look in the right direction from the right position
	view = glm::lookAt(lightPosition, lightPosition + lightFrontDirection, lightUpDirection);
	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	// Sets new light matrix
	lightModel = projection * view;
}

void Light::Matrix(Shader& shader, const char* uniform) {
	// Exports light matrix
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(lightModel));
}


void Light::Move(GLFWwindow* window, float deltaTime) {

	// light movement
	float normalSpeed = speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		lightPosition += normalSpeed * lightFrontDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		lightPosition += normalSpeed * -lightRightDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		lightPosition += normalSpeed * -lightFrontDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		lightPosition += normalSpeed * lightRightDirection;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		lightPosition += normalSpeed * lightUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		lightPosition += normalSpeed * -lightUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed = 0.8f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		speed = 0.4f;
	}
}

//light rotation
//yaw - light rotation around the y axis
//pitch - light rotation around the x axis
void Light::Rotate(float pitch, float yaw) {
	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	lightFrontDirection = glm::normalize(front);

	// Recalculate right and up directions
	lightRightDirection = glm::normalize(glm::cross(lightFrontDirection, lightUp));
	lightUpDirection = glm::normalize(glm::cross(lightRightDirection, lightFrontDirection));
}