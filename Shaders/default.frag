#version 330 core

out vec4 FragColor;

in vec3 currentPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;
in vec4 fragPosLight;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D ambientTex;
uniform sampler2D alphaTex;
uniform sampler2D shadowMap;


uniform int hasAlphaTex = 0; //by default, no alpha texture

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
float alphaVal = 1.0f; //the transparency of the fragment

vec4 pointLight(Light light){

    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
    discard;

    vec3 lightVec = light.lightPos - currentPos;
    float dist = length(lightVec);
    float a = 1.0f, b = 0.7f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f) * light.lightInten;

    float ambient = 0.1f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), ambient);

    float specular = 0.0f;
    float fallbackSpecularTex, fallbackAmbientTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - currentPos);
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        fallbackSpecularTex = (texture(specularTex, texCoord).r == 0.0f) ? 1.0f : texture(specularTex, texCoord).r; 
        fallbackAmbientTex = (texture(ambientTex, texCoord).r == 0.0f) ? 1.0f : texture(ambientTex, texCoord).r;
    }

    // shadow mapping (PCF)
    float shadow = 0.0f;
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
    if(lightCoords.z <= 1.0f){
        lightCoords = (lightCoords + 1.0f) / 2.0f;
        float currentDepth = lightCoords.z;
        float bias = max(0.025 * (1.0f - dot(normal, lightDirection)), 0.00005f);
       
        //soften shadows
        int sampleRadius = 3;
        vec2 pixelSize = 1.0f / textureSize(shadowMap, 0);
        for(int y = -sampleRadius; y <= sampleRadius; ++y){
            for(int x = -sampleRadius; x <= sampleRadius; ++x){
                float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
                if(currentDepth > closestDepth + bias)
                    shadow += 1.0f;
             }
        }
        shadow /= pow((sampleRadius * 2 + 1), 2);
    }

    return (texture(diffuseTex, texCoord) * (diffuse * (1.0f - shadow) * inten + fallbackAmbientTex * ambient) * alphaVal + fallbackSpecularTex * specular * (1.0f - shadow) * inten * alphaVal) * light.lightColor;
}

vec4 directionalLight(Light light){

    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
    discard;

    //ambient lighting
    float ambient = 0.1f;

    //diffuse lighting
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(vec3(-light.lightRot.x, 1.0 - light.lightRot.y, -light.lightRot.z)); 
    float diffuse = max(dot(normal, lightDirection), ambient);

    //specular lighting
    float specular = 0.0f;
    float fallbackSpecularTex, fallbackAmbientTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - currentPos);
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        fallbackSpecularTex = (texture(specularTex, texCoord).r == 0.0f) ? 1.0f : texture(specularTex, texCoord).r; 
        fallbackAmbientTex = (texture(ambientTex, texCoord).r == 0.0f) ? 1.0f : texture(ambientTex, texCoord).r;
    }

    // shadow mapping (PCF)
    float shadow = 0.0f;
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
    if(lightCoords.z <= 1.0f){
        lightCoords = (lightCoords + 1.0f) / 2.0f;
        float currentDepth = lightCoords.z;
        float bias = max(0.025 * (1.0f - dot(normal, lightDirection)), 0.0001f);
       
        //soften shadows
        int sampleRadius = 3;
        vec2 pixelSize = 1.0f / textureSize(shadowMap, 0);
        for(int y = -sampleRadius; y <= sampleRadius; ++y){
            for(int x = -sampleRadius; x <= sampleRadius; ++x){
                float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
                if(currentDepth > closestDepth + bias)
                    shadow += 1.0f;
             }
        }
        shadow /= pow((sampleRadius * 2 + 1), 2);
    }

    return (texture(diffuseTex, texCoord) * (diffuse * (1.0f - shadow) * alphaVal * light.lightInten + fallbackAmbientTex * ambient) + fallbackSpecularTex * specular * (1.0f - shadow) * alphaVal * light.lightInten) * light.lightColor;
}

vec4 spotLight(Light light){

    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
        discard;

    float outerCone = 0.9f;
    float innerCone = 0.95f;

    float ambient = 0.1f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(light.lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), ambient);

    float specular = 0.0f;
    float angle, inten, fallbackSpecularTex, fallbackAmbientTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - currentPos);
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        angle = dot(light.lightRot, -lightDirection);
        inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f) * light.lightInten;

        fallbackSpecularTex = (texture(specularTex, texCoord).r == 0.0f) ? 1.0f : texture(specularTex, texCoord).r; 
        fallbackAmbientTex = (texture(ambientTex, texCoord).r == 0.0f) ? 1.0f : texture(ambientTex, texCoord).r;
    }

    // shadow mapping (PCF)
    float shadow = 0.0f;
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
    if(lightCoords.z <= 1.0f){
        lightCoords = (lightCoords + 1.0f) / 2.0f;
        float currentDepth = lightCoords.z;
        float bias = max(0.025 * (1.0f - dot(normal, lightDirection)), 0.00005f);
       
        //soften shadows
        int sampleRadius = 3;
        vec2 pixelSize = 1.0f / textureSize(shadowMap, 0);
        for(int y = -sampleRadius; y <= sampleRadius; ++y){
            for(int x = -sampleRadius; x <= sampleRadius; ++x){
                float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
                if(currentDepth > closestDepth + bias)
                    shadow += 1.0f;
             }
        }
        shadow /= pow((sampleRadius * 2 + 1), 2);
    }

    return (texture(diffuseTex, texCoord) * (diffuse * (1.0f - shadow) * inten * alphaVal + fallbackAmbientTex * ambient) + fallbackSpecularTex * specular * (1.0f - shadow) * inten * alphaVal) * light.lightColor;
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

    // transparency calculation
    if(hasAlphaTex)
        alphaVal = texture(diffuseTex, texCoord).a;
    
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
    FragColor = resultColor * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), alphaVal);
}
