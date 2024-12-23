#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
//Normals
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;


out vec3 currentPos;
// Outputs the color for the Fragment Shader
out vec3 Normal;
out vec3 color;
// Outputs the texture coordinates to the fragment shader
out vec2 texCoord;

uniform mat4 camMatrix;
// Model matrix for the object
uniform mat4 model;

void main(){
	
	//we use it to calculate the direction of light
	currentPos = vec3(model * vec4(aPos, 1.0f));

	// Assign normals
	Normal = aNormal;

	// Assigns the colors from the Vertex Data to "color"
	color = aColor;

	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = aTex;

	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(currentPos, 1.0f);
};