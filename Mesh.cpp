#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures) {

	PrepareVertexData(vertices, indices);

	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	for (GLuint i = 0; i < textures.size(); i++)
		if (this->textures[i].type == "alphaTex") {
			this->alphaFlg = true;
			break;
		}

	// Bind Vertex Array Object
	vao.Bind();
	vbo = VBO(vertices);
	ebo = EBO(indices);

	// Links VBO attributes such as coordinates and colors to vao
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); //aPos
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); //aNormal
	vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); //aColor
	vao.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); //aTex

	vao.LinkAttrib(vbo, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)(11 * sizeof(float)));  // aTangent
	vao.LinkAttrib(vbo, 5, 3, GL_FLOAT, sizeof(Vertex), (void*)(14 * sizeof(float)));  // aBitangent

	// Unbind all to prevent accidentally modifying them
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Mesh::PrepareVertexData(std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
	// Loop over each triangle (3 vertices per triangle)
	for (size_t i = 0; i < indices.size(); i += 3) {
		// Get the indices of the triangle
		GLuint i0 = indices[i];
		GLuint i1 = indices[i + 1];
		GLuint i2 = indices[i + 2];

		// Get the vertices of the triangle
		Vertex& v0 = vertices[i0];
		Vertex& v1 = vertices[i1];
		Vertex& v2 = vertices[i2];

		// Calculate the edge vectors in model space
		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;

		// Calculate the differences in texture coordinates
		glm::vec2 deltaUV1 = v1.texUV - v0.texUV;
		glm::vec2 deltaUV2 = v2.texUV - v0.texUV;

		// Calculate the determinant and the tangent/bitangent
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		// Calculate the tangent and bitangent
		glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
		glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

		// Normalize the tangent and bitangent
		tangent = glm::normalize(tangent);
		bitangent = glm::normalize(bitangent);

		// Assign to the vertices
		v0.tangent = tangent;
		v0.bitangent = bitangent;
		v1.tangent = tangent;
		v1.bitangent = bitangent;
		v2.tangent = tangent;
		v2.bitangent = bitangent;
	}
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
		if (this->textures[i].type == "normalTex") {
			glUniform1i(glGetUniformLocation(shader.ID, "hasNormalTex"), 1);
		}
		else
			glUniform1i(glGetUniformLocation(shader.ID, "hasNormalTex"), 0);
		if (this->textures[i].type == "displacementTex") {
			glUniform1i(glGetUniformLocation(shader.ID, "hasDisplacementTex"), 1);
		}
		else
			glUniform1i(glGetUniformLocation(shader.ID, "hasDisplacementTex"), 0);
		if (this->textures[i].type == "emissiveTex") {
			glUniform1i(glGetUniformLocation(shader.ID, "hasEmissiveTex"), 1);
		}
		else
			glUniform1i(glGetUniformLocation(shader.ID, "hasEmissiveTex"), 0);
		if (this->textures[i].type == "metallicTex") {
			glUniform1i(glGetUniformLocation(shader.ID, "hasMetallicTex"), 1);
		}
		else
			glUniform1i(glGetUniformLocation(shader.ID, "hasMetallicTex"), 0);
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
