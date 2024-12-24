#version 330 core

out vec4 FragColor;

in vec3 currentPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D ambientTex;

// Light structure definition
struct Light {
    vec4 lightColor;
    vec3 lightPos;
    vec3 lightRot;
    float lightInten;
    int lightType; // 1 for directional, 2 for point, 3 for spotlight
};

// Array of lights
uniform Light lights[10]; // Adjust size based on the maximum number of lights you have
uniform int numLights;        // Number of active lights

// Camera position
uniform vec3 camPos;

float near = 0.1f;
float far = 100.0f;

vec4 pointLight(Light light){
    vec3 lightVec = light.lightPos - currentPos;
    float dist = length(lightVec);
    float a = 1.0f, b = 0.7f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f) * light.lightInten;

    float ambient = 0.2f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(camPos - currentPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;
    float fallbackSpecularTex = texture(specularTex, texCoord).r; 
    if(fallbackSpecularTex == 0.0f) fallbackSpecularTex = 1.0f;

    return (texture(diffuseTex, texCoord) * (diffuse * inten + texture(ambientTex, texCoord) * ambient) * light.lightInten + fallbackSpecularTex * specular * inten * 0.25) * light.lightColor;
}

vec4 directionalLight(Light light){
    float ambient = 0.2f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(vec3(-light.lightRot.x, 1.0, -light.lightRot.z)); 
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(camPos - currentPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;
    float fallbackSpecularTex = texture(specularTex, texCoord).r; 
    if(fallbackSpecularTex == 0.0f) fallbackSpecularTex = 1.0f;

    return (texture(diffuseTex, texCoord) * (diffuse + texture(ambientTex, texCoord) * ambient) * light.lightInten + fallbackSpecularTex * specular * light.lightInten * 0.25) * light.lightColor;
}

vec4 spotLight(Light light){
    float outerCone = 0.9f;
    float innerCone = 0.95f;

   

    float ambient = 0.2f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(light.lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(camPos - currentPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;
    float fallbackSpecularTex = texture(specularTex, texCoord).r; 
    if(fallbackSpecularTex == 0.0f) fallbackSpecularTex = 1.0f;

    float angle = dot(light.lightRot, -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f) * light.lightInten;

    return (texture(diffuseTex, texCoord) * (diffuse * inten + texture(ambientTex, texCoord) * ambient) + fallbackSpecularTex * specular * inten * light.lightInten * 0.1) * light.lightColor;
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

void main() {
    
    //can be used to see the depth buffer
    //vec4 zVal = vec4(vec3(liniarizeDepth(gl_FragCoord.z) / far), 1.0f);
	float depth = logisticDepth(gl_FragCoord.z, 0.125f, 20.0f);
    
    vec4 resultColor = vec4(0.0f);
    for(int i = 0; i < numLights; i++) {
        Light light = lights[i];
        if(light.lightType == 1)
            resultColor += directionalLight(light);
        else if(light.lightType == 2)
            resultColor += pointLight(light);
        else if(light.lightType == 3)
            resultColor += spotLight(light);
    }
    FragColor = resultColor * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
}
