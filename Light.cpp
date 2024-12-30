#include "Light.h"

Light::Light(glm::vec3 pos, glm::vec3 rot, glm::vec4 col, float inten, int t) {

	Light::position = pos;
	Light::color = col;
	Light::intensity = inten;
	Light::type = t;
	modelMatrix = glm::translate(modelMatrix, position);

	// Calculate the initial light front direction
	lightFrontDirection = glm::normalize(lightTarget - position);
	// Calculate the initial right direction
	lightRightDirection = glm::normalize(glm::cross(lightFrontDirection, lightUpDirection));
}

void Light::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Adds perspective to the scene
	if (this->type == 1) {
		projection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, nearPlane, farPlane);
		view = glm::lookAt(10.0f * position, position + lightFrontDirection, lightUpDirection);

		lightProjectionMat = projection * view;
	}
	else {
		projection = glm::perspective(glm::radians(FOVdeg), 1.0f, nearPlane, farPlane);
		view = glm::lookAt(position, position + lightFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f));

		lightProjectionMat = projection * view;
	}
}

void Light::Modify(GLFWwindow* window, float deltaTime, glm::vec3 cameraFrontDirection, glm::vec3 cameraRightDirection) {

	// note: forwardDirection wont influence the height (and we use camera's front and right directions)
	cameraFrontDirection = glm::normalize(glm::vec3(cameraFrontDirection.x, 0, cameraFrontDirection.z));
	bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	// LIGHT MOVEMENT
	float normalSpeed = speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if(shiftPressed)
			position += normalSpeed * lightUp;
		else
			position += normalSpeed * cameraFrontDirection;  //front
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		position += normalSpeed * -cameraRightDirection; //left
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (shiftPressed)
			position += normalSpeed * -lightUp;
		else
			position += normalSpeed * -cameraFrontDirection; //backward
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position += normalSpeed * cameraRightDirection; //right
	}

	//LIGHT TYPE
	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) {
		type = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS) {
		type = 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS) {
		type = 3;
	}

	//LIGHT INTENSITY / RGB values
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) {
			color.x = std::max(color.x - normalSpeed * 15, 0.0f);
		}
		else if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS) {
			color.y = std::max(color.y - normalSpeed * 15, 0.0f);
		}
		else if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS) {
			color.z = std::max(color.z - normalSpeed * 15, 0.0f);
		}
		else {
			intensity = std::max(intensity - (normalSpeed * 15), 0.0f);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) {
			color.x = std::min(color.x + normalSpeed * 15, 1.0f);
		}
		else if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS) {
			color.y = std::min(color.y + normalSpeed * 15, 1.0f);
		}
		else if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS) {
			color.z = std::min(color.z + normalSpeed * 15, 1.0f);
		}
		else {
			intensity = std::min(intensity + normalSpeed * 15, 30.0f);
		}
	}

	if (type == 1) { //directional light doesnt have a rotation, it relies on the position
		lightFrontDirection = glm::normalize(lightTarget - position);
		lightRightDirection = glm::normalize(glm::cross(lightFrontDirection, lightUp));
		lightUpDirection = glm::normalize(glm::cross(lightRightDirection, lightFrontDirection));
	}

	// Make sure to reflect the changes
	modelMatrix = glm::translate(glm::mat4(1.0f), position);
}

//LIGHT ROTATION
//yaw - light rotation around the y axis
//pitch - light rotation around the x axis
void Light::Rotate(float pitch, float yaw) {
	if (type != 1) {
		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		lightFrontDirection = glm::normalize(front);

		// Recalculate right and up directions
		lightRightDirection = glm::normalize(glm::cross(lightFrontDirection, lightUp));
		lightUpDirection = glm::normalize(glm::cross(lightRightDirection, lightFrontDirection));
	}
}

void Light::applyUniforms(Shader& baseShader, Shader& shadowShader, int index, int numLigths) {

	std::string base = "lights[" + std::to_string(index) + "]";

	baseShader.Activate();
	glUniform4f(glGetUniformLocation(baseShader.ID, (base + ".lightColor").c_str()), color.r, color.g, color.b, color.a);
	glUniform3f(glGetUniformLocation(baseShader.ID, (base + ".lightPos").c_str()), position.x, position.y, position.z);
	glUniform3f(glGetUniformLocation(baseShader.ID, (base + ".lightRot").c_str()), lightFrontDirection.x, lightFrontDirection.y, lightFrontDirection.z);
	glUniform1f(glGetUniformLocation(baseShader.ID, (base + ".lightInten").c_str()), intensity);
	glUniform1i(glGetUniformLocation(baseShader.ID, (base + ".lightType").c_str()), type);
	glUniform1i(glGetUniformLocation(baseShader.ID, "numLights"), numLigths);

	glUniform1i(glGetUniformLocation(shadowShader.ID, "numLights"), numLigths);
}

void swap(Light& a, Light& b) noexcept {
	using std::swap;

	// Swap the member variables of Light class
	swap(a.position, b.position);
	swap(a.color, b.color);
	swap(a.intensity, b.intensity);
	swap(a.type, b.type);
	swap(a.modelMatrix, b.modelMatrix);
	swap(a.lightProjectionMat, b.lightProjectionMat);
	swap(a.lightFrontDirection, b.lightFrontDirection);
	swap(a.lightRightDirection, b.lightRightDirection);
	swap(a.lightUpDirection, b.lightUpDirection);
	swap(a.lightTarget, b.lightTarget);
	swap(a.lightbulb, b.lightbulb);
	swap(a.width, b.width);
	swap(a.height, b.height);
	swap(a.speed, b.speed);
	swap(a.sensitivity, b.sensitivity);
}
