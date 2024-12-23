#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string.h>
#include <vector>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

struct Material {

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Mesh {

public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;

	VAO vao;
	VBO vbo;
	EBO ebo;

	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);

	void Draw(Shader& shader, Camera& camera);
};

#endif