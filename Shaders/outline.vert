#version 330 core

layout (location = 0) in vec3 aPos;
//Normals
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;

out vec2 TexCoord; // Pass to the fragment shader

uniform mat4 camMatrix;
uniform mat4 model;
uniform float outline;
uniform vec3 camPos;

void main(){

	// Calculate world-space position of the vertex
    vec4 worldPos = model * vec4(aPos, 1.0f);
    
    // Compute distance from the camera
    float distance = length(camPos - worldPos.xyz);
    
    // Scale the outline based on distance (e.g., inverse scaling)
    float scaledOutline = outline * distance * 0.1; // Adjust the multiplier for desired effect

    TexCoord = aTex; // Pass texture coordinates

    // Apply outline logic
    vec3 currentPos = vec3(model * vec4(aPos + aNormal * scaledOutline, 1.0f));
    gl_Position = camMatrix * vec4(currentPos, 1.0f);
}