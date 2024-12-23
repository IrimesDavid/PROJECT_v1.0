#ifndef light_CLASS_H
#define light_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shaderClass.h"

class Light {
private:
private:
	glm::vec4 lightColor;
	glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 lightFrontDirection;
	glm::vec3 lightRightDirection;
	glm::vec3 const lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 lightUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);

public:
	glm::vec3 lightPosition;
	int width;
	int height;

	float speed = 0.4f;
	float sensitivity = 0.1f;

	glm::mat4 lightModel = glm::mat4(1.0f);


	Light(glm::vec3 position, glm::vec4 color);

	// Updates the light matrix to the Vertex Shader
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	// Exports the light matrix to a shader
	void Matrix(Shader& shader, const char* uniform);
	// Handles light movement
	void Move(GLFWwindow* window, float deltaTime);
	//Handles light rotation
	void Rotate(float pitch, float yaw);
};

#endif