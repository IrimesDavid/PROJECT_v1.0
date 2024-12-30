#version 330 core

out vec4 FragColor;
uniform vec4 outlineColor;

in vec2 TexCoord; // Interpolated texture coordinates from the vertex shader
uniform sampler2D alphaTex; // Alpha texture sampler
uniform int hasAlphaTex = 0; //by default, no alpha texture

void main() {

    if(hasAlphaTex == 1 && texture(alphaTex, TexCoord).r < 0.1)
    discard;

    // Set the fragment color
    FragColor = outlineColor;
}
