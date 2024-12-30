#ifndef light_CLASS_H
#define light_CLASS_H

#include <utility> // for std::swap
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "Model3D.hpp"

#include "shaderClass.h"

class Light {
private:
private:
	glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lightRightDirection;
	glm::vec3 const lightUp = glm::vec3(0.0f, 1.0f, 0.0f);

public:
	glm::vec3 lightFrontDirection;
	glm::vec3 lightUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);

	Model3D lightbulb;
	glm::vec3 position;
	glm::vec4 color;
	float intensity;
	int type; // 1 = directional, 2 = point, 3 = spot

	int width = 50;
	int height = 50;

	float speed = 1.0f;
	float sensitivity = 0.05f;

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 lightProjectionMat = glm::mat4(1.0f);

	Light(glm::vec3 pos, glm::vec3 rot, glm::vec4 col, float inten, int t);

	// Updates the light matrix to the Vertex Shader
	void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
	// Handles light movement
	void Modify(GLFWwindow* window, float deltaTime, glm::vec3 cameraFrontDirection, glm::vec3 cameraRightDirection);
	// Handles light rotation
	void Rotate(float pitch, float yaw);
	// Self explanatory
	void applyUniforms(Shader& baseShader, Shader& shadowShader, int index, int numLigths);

	friend void swap(Light& a, Light& b) noexcept;
};

#endif