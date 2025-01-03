//
//  SkyBox.cpp
//  Lab10(CubeMaps)
//
//  Created by CGIS on 16/12/2016.
//  Copyright � 2016 CGIS. All rights reserved.
//

#include "SkyBox.h"

    SkyBox::SkyBox()
    {

    }

    void SkyBox::setSkybox(const char* basePath) {
        std::string base(basePath); // Convert basePath to std::string

        std::vector<std::string> facePaths = {
            base + "right.tga",
            base + "left.tga",
            base + "top.tga",
            base + "bottom.tga",
            base + "back.tga",
            base + "front.tga"
        };

        std::vector<const GLchar*> faces;
        for (const auto& path : facePaths) {
            faces.push_back(path.c_str()); // Convert std::string to const const char*
        }

        this->Load(faces);
    }


    void SkyBox::Load(std::vector<const GLchar*> cubeMapFaces)
    {
        cubemapTexture = LoadSkyBoxTextures(cubeMapFaces);
        InitSkyBox();
    }

    void SkyBox::Draw(Shader skyboxShader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
    {
        skyboxShader.Activate();

        //set the view and projection matrices
        glm::mat4 transformedView = glm::mat4(glm::mat3(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(transformedView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
    }

    GLuint SkyBox::LoadSkyBoxTextures(std::vector<const GLchar*> skyBoxFaces)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0);

        int width, height, n;
        unsigned char* image;
        int force_channels = 3;

        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        for (GLuint i = 0; i < skyBoxFaces.size(); i++)
        {
            image = stbi_load(skyBoxFaces[i], &width, &height, &n, force_channels);
            if (!image) {
                fprintf(stderr, "ERROR: could not load %s\n", skyBoxFaces[i]);
                return false;
            }
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return textureID;
    }

    void SkyBox::InitSkyBox()
    {
        GLfloat skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &(this->skyboxVAO));
        glGenBuffers(1, &skyboxVBO);

        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

        glBindVertexArray(0);
    }

    GLuint SkyBox::GetTextureId()
    {
        return cubemapTexture;
    }