#version 330 core

layout(triangles) in;           // Input: triangles from the vertex shader
layout(triangle_strip, max_vertices = 3) out; // Output: triangle strip

// Inputs from the vertex shader
in vec3 currentPos[];
in vec3 Normal[];
in vec3 Tangent[];
in vec3 Bitangent[];
in vec3 color[];
in vec2 texCoord[];
in vec4 fragPosLight[];

// Outputs to the fragment shader
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec3 fragColor;
out vec2 fragTexCoord;
out vec4 fragPosLightTBN;
out mat3 fragTBNMatrix;

uniform vec3 camPos; // Camera position

void main() {
    for (int i = 0; i < 3; i++) { // Loop through triangle vertices
        // Pass through vertex attributes
        fragNormal = normalize(Normal[i]);
        fragTangent = normalize(Tangent[i]);
        fragBitangent = normalize(Bitangent[i]);
        fragColor = color[i];
        fragTexCoord = texCoord[i];
        fragPosLightTBN = fragPosLight[i];

        // Construct the TBN matrix for transforming vectors to tangent space
        fragTBNMatrix = mat3(
            normalize(fragTangent),
            normalize(fragBitangent),
            normalize(fragNormal)
        );

        // Transform vertex position to clip space
        gl_Position = gl_in[i].gl_Position;

        // Emit the vertex
        EmitVertex();
    }

    // End the triangle primitive
    EndPrimitive();
}
