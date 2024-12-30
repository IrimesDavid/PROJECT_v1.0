//
//  SkyBox.hpp
//  Lab10(CubeMaps)
//
//  Created by CGIS on 16/12/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef SkyBox_hpp
#define SkyBox_hpp


#include "ShaderClass.h"
#include "Libraries\include\stb\stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <stdio.h>

class SkyBox
{
public:
    SkyBox();
    void Load(std::vector<const GLchar*> cubeMapFaces);
    void Draw(Shader shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void setSkybox(const char* basePath);
    GLuint GetTextureId();
private:
    GLuint skyboxVAO;
    GLuint skyboxVBO;
    GLuint textureSkyboxID;
    GLuint cubemapTexture;
    GLuint LoadSkyBoxTextures(std::vector<const GLchar*> cubeMapFaces);
    void InitSkyBox();
};

#endif /* SkyBox_hpp */
