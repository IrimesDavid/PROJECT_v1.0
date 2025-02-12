#version 330 core

// Positions/Coordinates
layout(location = 0) in vec3 aPos;
// Normals
layout(location = 1) in vec3 aNormal;
// Colors
layout(location = 2) in vec3 aColor;
// Texture Coordinates
layout(location = 3) in vec2 aTex;

layout(location = 4) in vec3 aTangent;  // Tangent
layout(location = 5) in vec3 aBitangent;  // Bitangent

out vec3 Tangent;            // Tangent vector
out vec3 Bitangent;          // Bitangent vector

out vec3 currentPos;         // Position in world space
out vec3 Normal;             // Normal vector
out vec3 color;              // Vertex color
out vec2 texCoord;           // Texture coordinates
out vec4 fragPosLight;       // Position in light space

uniform mat4 camMatrix;      // Camera matrix
uniform mat4 model;          // Model matrix
uniform mat4 lightProjection; // Light projection matrix

void main() {
    // Calculate the world position
    currentPos = vec3(model * vec4(aPos, 1.0f));

    // Assign normals
    Normal = aNormal;


    Tangent = normalize(mat3(model) * aTangent);
    Bitangent = normalize(mat3(model) * aBitangent);

    // Assign color and texture coordinates
    color = aColor;
    texCoord = aTex;

    // Transform position into light space
    fragPosLight = lightProjection * vec4(currentPos, 1.0f);

    // Final vertex position in clip space
    gl_Position = camMatrix * vec4(currentPos, 1.0f);
}
