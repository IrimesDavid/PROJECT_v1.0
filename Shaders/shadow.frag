#version 330 core

in vec2 TexCoord; // Interpolated texture coordinates from the vertex shader
uniform sampler2D alphaTex; // Alpha texture sampler
uniform int hasAlphaTex = 0; //by default, no alpha texture

void main(){
	if(hasAlphaTex == 1 && texture(alphaTex, TexCoord).r < 0.1)
    discard;
    
    // The fragment depth is written automatically to the depth buffer
}