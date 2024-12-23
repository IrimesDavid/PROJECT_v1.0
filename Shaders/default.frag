#version 330 core
// Outputs colors in RGBA
out vec4 FragColor;

in vec3 currentPos;
in vec3 Normal;
// Inputs the color from the Vertex Shader
in vec3 color;
// Inputs the texture coordinates from the Vertex Shader
in vec2 texCoord;

// Gets the Texture Unit from the main function (diffuse texture)
uniform sampler2D diffuseTex;
// Gets the Texture Unit from the main function (specular)
uniform sampler2D specularTex;
// (ambient occlusion)
uniform sampler2D ambientTex;

// Get the color from the light source
uniform vec4 lightColor;
// Get the position of the light
uniform vec3 lightPos;
// Get the rotation of the light
uniform vec3 lightRot;
// Modular light intensity
uniform float lightInten;
// Modular light type
uniform int lightType;
// Camera position
uniform vec3 camPos;

//for Depth
float near = 0.1f;
float far = 100.0f;

vec4 pointLight(){
	
	vec3 lightVec = lightPos - currentPos;
	float dist = length(lightVec);
	//equation for light distance and fade properties
	float a = 1.0f, b = 0.7f;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f) * lightInten;

	//ambient lighting
	float ambient = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	//specular lighting
	float specularLight = 0.5f;
	vec3 viewDirection = normalize(camPos - currentPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;
	float fallbackSpecularTex = texture(specularTex, texCoord).r; //if we dont have a texture, we make sure to still have a specular point on that mesh
	if(fallbackSpecularTex == 0.0f)
		fallbackSpecularTex = 1.0f;

	return (texture(diffuseTex, texCoord) * (diffuse * inten + texture(ambientTex, texCoord) * ambient) * lightInten + fallbackSpecularTex * specular * inten  * lightInten * 0.25) * lightColor;
}

vec4 directionalLight(){
	
	//ambient lighting
	float ambient = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(vec3(-lightRot.x, 1.0, -lightRot.z)); //note light direction is from the object to the light (so its inverted)
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	//specular lighting
	float specularLight = 0.5f;
	vec3 viewDirection = normalize(camPos - currentPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;
	float fallbackSpecularTex = texture(specularTex, texCoord).r; //if we dont have a texture, we make sure to still have a specular point on that mesh
	if(fallbackSpecularTex == 0.0f)
		fallbackSpecularTex = 1.0f;

	return (texture(diffuseTex, texCoord) * (diffuse + texture(ambientTex, texCoord) * ambient) * lightInten + fallbackSpecularTex * specular * lightInten * 0.25) * lightColor;
}

vec4 spotLight(){
	
	//represent cos values of two angles
	float outerCone = 0.9f;
	float innerCone = 0.95f;

	float dist = length(lightPos - currentPos);
	//equation for light distance and fade properties
	float a = 0.1f, b = 0.5f;
	float intenClassic = 1.0f / (a * dist * dist + b * dist + 1.0f) * lightInten; //for extra spotlight fade at large distances

	//ambient lighting
	float ambient = 0.2f;

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - currentPos);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	//specular lighting
	float specularLight = 0.5f;
	vec3 viewDirection = normalize(camPos - currentPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;
	float fallbackSpecularTex = texture(specularTex, texCoord).r; //if we dont have a texture, we make sure to still have a specular point on that mesh
	if(fallbackSpecularTex == 0.0f)
		fallbackSpecularTex = 1.0f;

	float angle = dot(lightRot, -lightDirection);
	float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f) * lightInten;

	return (texture(diffuseTex, texCoord) * (diffuse * intenClassic * inten + texture(ambientTex, texCoord) * ambient) + fallbackSpecularTex * specular * inten * lightInten * 0.1) * lightColor;
}

float liniarizeDepth(float depth){
	
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

// make fog effect
float logisticDepth(float depth, float stepness, float offset)
{ //note: last two arguments can be made modular

	float zVal = liniarizeDepth(depth);
	return (1 / (1 + exp(-stepness * (zVal - offset))));
}

void main()
{
	//vec4 zVal = vec4(vec3(liniarizeDepth(gl_FragCoord.z) / far), 1.0f); //can be used to see the depth buffer
	float depth = logisticDepth(gl_FragCoord.z, 0.125f, 20.0f);
	if(lightType == 1)
		FragColor = directionalLight() * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
	else if(lightType == 2)
		FragColor = pointLight() * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
	else if(lightType == 3)
		FragColor = spotLight() * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
}