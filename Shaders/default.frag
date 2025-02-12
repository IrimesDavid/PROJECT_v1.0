#version 330 core

out vec4 FragColor;

in vec3 currentPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;
in vec4 fragPosLight;

//for normal and parallax occlusion calculations
in vec3 Tangent;
in vec3 Bitangent;

// Texture maps
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D ambientTex;
uniform sampler2D alphaTex;
uniform sampler2D normalTex;
uniform sampler2D displacementTex;
uniform sampler2D emissiveTex;
uniform sampler2D metallicTex;

uniform sampler2D shadowMap;
uniform samplerCube skyboxCubeMap;

// Flags
uniform int hasAlphaTex;
uniform int hasNormalTex;
uniform int hasDisplacementTex;
uniform int hasEmissiveTex;
uniform int hasMetallicTex;
uniform int normalParallaxFlg;

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

// Fog toggle
uniform int enableFog;

// Camera position
uniform vec3 camPos;

// Extra
float near = 0.1f;
float far = 100.0f;
float alphaVal = 1.0f; //the transparency of the fragment
vec4 fogColor; // color of the fog (modular, based on the skybox)
float ambient = 0.02f;
bool availableShadowMap = true; //the shadow map is used only by one light (we only use one shadow map)

vec4 pointLight(Light light){

    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
    discard;

    vec3 lightVec = light.lightPos - currentPos;
    float dist = length(lightVec);
    float a = 1.0f, b = 0.7f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f) * light.lightInten;
    
    

    // view direction
    vec3 viewDirection = normalize(camPos - currentPos);
    vec2 UVs = texCoord;
    //normal
    vec3 normal;

    if(normalParallaxFlg == 1 && hasDisplacementTex == 1){
        
         mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
        
        vec3 tangentViewDir;
        if(normalize(Normal).x != 0.0)
            tangentViewDir = TBN * vec3(viewDirection.x, -viewDirection.y, viewDirection.z); // facing X or -X
        else if(normalize(Normal).y != 0.0)
            tangentViewDir = TBN * vec3(-viewDirection.x, viewDirection.y, viewDirection.z); // facing Y or -Y
        else
            tangentViewDir = TBN * vec3(viewDirection.x, viewDirection.y, viewDirection.z); //facing Z or -Z
        

        // Variables that control parallax occlusion quality (displacement mapping)
        float heightScale = 0.05f;
        const float minLayers = 8.0f;
        const float maxLayers = 64.0f;
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), tangentViewDir)));
        float layerDepth = 1.0f / numLayers;
        float currentLayerDepth = 0.0f;

        //Remove the z division for less aberated results
        vec2 S = tangentViewDir.xy / tangentViewDir.z * heightScale;
        vec2 deltaUVs = S / numLayers;

        
        float currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
    
        //Loop until the point of the heightmap is 'hit'
        while(currentLayerDepth < currentDepthMapVal){
            UVs -= deltaUVs;
            currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
            currentLayerDepth += layerDepth;
        }
        //Apply occlusion (interpolation with previous value)
        vec2 prevTexCoords = UVs + deltaUVs;
        float afterDepth = currentDepthMapVal - currentLayerDepth;
        float beforeDepth = 1.0f - texture(displacementTex, prevTexCoords).r - currentLayerDepth + layerDepth;
        float weight = afterDepth / (afterDepth - beforeDepth);
        UVs = prevTexCoords * weight + UVs * (1.0f - weight);

        //Get rid of anything outside the normal range
        if(UVs.x > 1.0 || UVs.y > 1.0  || UVs.x < 0.0 || UVs.y < 0.0)
            discard;

       //consider that we already have a normal map, if we have a displacement map
        normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
    }
     else{
        if(normalParallaxFlg == 1 && hasNormalTex == 1){
            mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
            normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
        }
        else
            normal = normalize(Normal);
     }

    //diffuse
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), ambient);

    float specular = 0.0f;
    float fallbackSpecularTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        fallbackSpecularTex = (texture(specularTex, UVs).r == 0.0f) ? 1.0f : texture(specularTex, UVs).r; 
        
    }

    // reflexions (metallic texture)
    vec3 reflectionDir = reflect(-viewDirection, normal);
    vec4 reflectionColor = texture(skyboxCubeMap, reflectionDir);

    float metalness = 0.0f;
    if(hasMetallicTex == 1)
        metalness = texture(metallicTex, UVs).r;

    // shadow mapping (PCF)
    float shadow = 0.0f;
   /* vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
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
    }*/
    


    return (texture(diffuseTex, UVs) * (diffuse * (1.0f - shadow) * inten) + fallbackSpecularTex * specular * (1.0f - shadow) * inten + reflectionColor * metalness * inten) * light.lightColor;
}

vec4 directionalLight(Light light){
    
    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
    discard;

    
    

    // view direction
    vec3 viewDirection = normalize(camPos - currentPos);
    vec2 UVs = texCoord;
    vec3 normal;

    if(normalParallaxFlg == 1 && hasDisplacementTex == 1){

        mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
        
        vec3 tangentViewDir;
        if(normalize(Normal).x != 0.0)
            tangentViewDir = TBN * vec3(viewDirection.x, -viewDirection.y, viewDirection.z); // facing X or -X
        else if(normalize(Normal).y != 0.0)
            tangentViewDir = TBN * vec3(-viewDirection.x, viewDirection.y, viewDirection.z); // facing Y or -Y
        else
            tangentViewDir = TBN * vec3(viewDirection.x, viewDirection.y, viewDirection.z); //facing Z or -Z
        

        // Variables that control parallax occlusion quality (displacement mapping)
        float heightScale = 0.05f;
        const float minLayers = 8.0f;
        const float maxLayers = 64.0f;
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), tangentViewDir)));
        float layerDepth = 1.0f / numLayers;
        float currentLayerDepth = 0.0f;

        //Remove the z division for less aberated results
        vec2 S = tangentViewDir.xy / tangentViewDir.z * heightScale;
        vec2 deltaUVs = S / numLayers;

        
        float currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
    
        //Loop until the point of the heightmap is 'hit'
        while(currentLayerDepth < currentDepthMapVal){
            UVs -= deltaUVs;
            currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
            currentLayerDepth += layerDepth;
        }
        //Apply occlusion (interpolation with previous value)
        vec2 prevTexCoords = UVs + deltaUVs;
        float afterDepth = currentDepthMapVal - currentLayerDepth;
        float beforeDepth = 1.0f - texture(displacementTex, prevTexCoords).r - currentLayerDepth + layerDepth;
        float weight = afterDepth / (afterDepth - beforeDepth);
        UVs = prevTexCoords * weight + UVs * (1.0f - weight);

        //Get rid of anything outside the normal range
        if(UVs.x > 1.0 || UVs.y > 1.0  || UVs.x < 0.0 || UVs.y < 0.0)
            discard;

       //consider that we already have a normal map, if we have a displacement map
        normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
    }
    else{
        if(normalParallaxFlg == 1 && hasNormalTex == 1){
            mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
            normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
        }
        else
            normal = normalize(Normal);
     }
    //diffuse lighting
    vec3 lightDirection = normalize(vec3(-light.lightRot.x, 1.0 - light.lightRot.y, -light.lightRot.z)); 
    float diffuse = max(dot(normal, lightDirection), ambient);

    //specular lighting
    float specular = 0.0f;
    float fallbackSpecularTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        fallbackSpecularTex = (texture(specularTex, UVs).r == 0.0f) ? 1.0f : texture(specularTex, UVs).r; 
        
    }


    // reflexions (metallic texture)
    vec3 reflectionDir = reflect(-viewDirection, normal);
    vec4 reflectionColor = texture(skyboxCubeMap, reflectionDir);

    float metalness = 0.0f;
    if(hasMetallicTex == 1)
        metalness = texture(metallicTex, UVs).r;

    // shadow mapping (PCF)
    float shadow = 0.0f;
    if(availableShadowMap){
        availableShadowMap = false;

        vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
        if(lightCoords.z <= 1.0f){
            lightCoords = (lightCoords + 1.0f) / 2.0f;
            float currentDepth = lightCoords.z;
            float bias = max(0.025 * (1.0f - dot(normalize(Normal), lightDirection)), 0.0001f);
       
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
    }

    return (texture(diffuseTex, UVs) * (diffuse * (1.0f - shadow) * light.lightInten) + fallbackSpecularTex * specular * (1.0f - shadow) * light.lightInten + reflectionColor * metalness * light.lightInten) * light.lightColor;
}

vec4 spotLight(Light light){

    if(hasAlphaTex == 1 && texture(alphaTex, texCoord).r < 0.1)
        discard;

    float outerCone = 0.9f;
    float innerCone = 0.95f;

    vec3 lightVec = light.lightPos - currentPos;
    float dist = length(lightVec);
    float a = 0.3f, b = 0.1f;
    float distInten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    

    // view direction
    vec3 viewDirection = normalize(camPos - currentPos);
    vec2 UVs = texCoord;
    vec3 normal;


    if(normalParallaxFlg == 1 && hasDisplacementTex == 1){

        mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
        
        vec3 tangentViewDir;
        if(normalize(Normal).x != 0.0)
            tangentViewDir = TBN * vec3(viewDirection.x, -viewDirection.y, viewDirection.z); // facing X or -X
        else if(normalize(Normal).y != 0.0)
            tangentViewDir = TBN * vec3(-viewDirection.x, viewDirection.y, viewDirection.z); // facing Y or -Y
        else
            tangentViewDir = TBN * vec3(viewDirection.x, viewDirection.y, viewDirection.z); //facing Z or -Z
        

        // Variables that control parallax occlusion quality (displacement mapping)
        float heightScale = 0.05f;
        const float minLayers = 8.0f;
        const float maxLayers = 64.0f;
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), tangentViewDir)));
        float layerDepth = 1.0f / numLayers;
        float currentLayerDepth = 0.0f;

        //Remove the z division for less aberated results
        vec2 S = tangentViewDir.xy / tangentViewDir.z * heightScale;
        vec2 deltaUVs = S / numLayers;

        
        float currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
    
        //Loop until the point of the heightmap is 'hit'
        while(currentLayerDepth < currentDepthMapVal){
            UVs -= deltaUVs;
            currentDepthMapVal = 1.0f - texture(displacementTex, UVs).r;
            currentLayerDepth += layerDepth;
        }
        //Apply occlusion (interpolation with previous value)
        vec2 prevTexCoords = UVs + deltaUVs;
        float afterDepth = currentDepthMapVal - currentLayerDepth;
        float beforeDepth = 1.0f - texture(displacementTex, prevTexCoords).r - currentLayerDepth + layerDepth;
        float weight = afterDepth / (afterDepth - beforeDepth);
        UVs = prevTexCoords * weight + UVs * (1.0f - weight);

        //Get rid of anything outside the normal range
        if(UVs.x > 1.0 || UVs.y > 1.0  || UVs.x < 0.0 || UVs.y < 0.0)
            discard;

       //consider that we already have a normal map, if we have a displacement map
        normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
    }
    else{
        if(normalParallaxFlg == 1 && hasNormalTex == 1){
            mat3 TBN = mat3(Tangent, Bitangent, normalize(Normal));
            normal = normalize(TBN * (texture(normalTex, UVs).rgb * 2.0 - 1.0));
            }
        else
            normal = normalize(Normal);
    }

    vec3 lightDirection = normalize(light.lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), ambient);

    float specular = 0.0f;
    float angle, inten, fallbackSpecularTex;
    if(diffuse != 0.0f){
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - currentPos);
        //blinPhong
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;

        angle = dot(light.lightRot, -lightDirection);
        inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f) * light.lightInten * distInten;

        fallbackSpecularTex = (texture(specularTex, UVs).r == 0.0f) ? 1.0f : texture(specularTex, UVs).r; 
        
    }


    // reflexions (metallic texture)
    vec3 reflectionDir = reflect(-viewDirection, normal);
    vec4 reflectionColor = texture(skyboxCubeMap, reflectionDir);

    float metalness = 0.0f;
    if(hasMetallicTex == 1)
        metalness = texture(metallicTex, UVs).r;

    // shadow mapping (PCF)
    float shadow = 0.0f;
    if(availableShadowMap){
        availableShadowMap = false;

        vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
        if(lightCoords.z <= 1.0f){
            lightCoords = (lightCoords + 1.0f) / 2.0f;
            float currentDepth = lightCoords.z;
            float bias = max(0.025 * (1.0f - dot(normal, lightDirection)), 0.005f);
       
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
    }

    return (texture(diffuseTex, UVs) * (diffuse * (1.0f - shadow) * inten) + fallbackSpecularTex * specular * (1.0f - shadow) * inten + reflectionColor * metalness * inten) * light.lightColor;
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

    // transparency calculation
    if(hasAlphaTex)
        alphaVal = texture(alphaTex, texCoord).r;
    
    //ambient texture
    float fallbackAmbientTex = (texture(ambientTex, texCoord).r == 0.0f) ? 1.0f : texture(ambientTex, texCoord).r;
    
    vec4 resultColor = texture(diffuseTex, texCoord) * fallbackAmbientTex * ambient; //first add the ambient lighting (using an ambient texture if its the case)
    //add all lights influence
    for(int i = 0; i < numLights; i++) {
        Light light = lights[i];
        if(light.lightType == 1)
            resultColor += directionalLight(light);
        else if(light.lightType == 2)
            resultColor += pointLight(light);
        else if(light.lightType == 3)
            resultColor += spotLight(light);
    }
     //emissive texture
    if(hasEmissiveTex == 1)
        resultColor += texture(emissiveTex, texCoord);

    //can be used to see the depth buffer
    //vec4 zVal = vec4(vec3(liniarizeDepth(gl_FragCoord.z) / far), 1.0f);
	float depth = logisticDepth(gl_FragCoord.z, 0.125f, 20.0f);

    if(enableFog == 1){
        fogColor = texture(skyboxCubeMap, vec3(0.0, 0.0, 0.0));
        fogColor = mix(fogColor, vec4(1.0, 1.0, 1.0, 0.5), 0.1);
        FragColor = vec4(resultColor.rgb, alphaVal) * (1.0f - depth) + vec4(depth * vec3(fogColor), alphaVal);
        }
    else
        FragColor = vec4(resultColor.rgb, alphaVal);
}
