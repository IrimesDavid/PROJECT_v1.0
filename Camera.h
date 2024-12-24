#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shaderClass.h"

class Camera {
private:
private:
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 const cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);

public:
	glm::vec3 cameraFrontDirection;
	glm::vec3 cameraRightDirection;
	glm::vec3 cameraPosition;
	int width;
	int height;

	float speed = 0.8f;
	float sensitivity = 0.1f;

	glm::mat4 cameraMatrix = glm::mat4(1.0f);


	Camera(int width, int height, glm::vec3 position);

	// Updates the camera matrix to the Vertex Shader
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	// Exports the camera matrix to a shader
	void Matrix(Shader& shader, const char* uniform);
	// Handles camera movement
	void Move(GLFWwindow* window, float deltaTime);
	//Handles camera rotation
	void Rotate(float pitch, float yaw);
};

#endif