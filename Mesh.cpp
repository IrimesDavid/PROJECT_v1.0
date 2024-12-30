#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures) {

	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	// Bind Vertex Array Object
	vao.Bind();
	vbo = VBO(vertices);
	ebo = EBO(indices);

	// Links VBO attributes such as coordinates and colors to vao
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Mesh::Draw(Shader& shader, Camera& camera) {
	shader.Activate();

	//set textures
	for (GLuint i = 0; i < textures.size(); i++) {
		this->textures[i].texUnit(shader, this->textures[i].type.c_str(), i);
		this->textures[i].Bind(i);

		if (this->textures[i].type == "alphaTex") {
			glUniform1i(glGetUniformLocation(shader.ID, "hasAlphaTex"), 1);
		}
		else
			glUniform1i(glGetUniformLocation(shader.ID, "hasAlphaTex"), 0);
	}

	vao.Bind();
	glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//  Unbind all textures
	for (GLuint i = 0; i < this->textures.size(); i++) {

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Set shader uniforms for camera
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
	camera.Matrix(shader, "camMatrix");

	//Render the mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
