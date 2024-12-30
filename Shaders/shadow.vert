#version 330 core

layout (location = 0) in vec3 aPos;
//Normals
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;

out vec2 TexCoord; // Pass to the fragment shader

uniform mat4 lightProjection;
uniform mat4 model;

void main(){
	
	gl_Position = lightProjection * model * vec4(aPos, 1.0f);
	TexCoord = aTex; // Pass texture coordinates
}