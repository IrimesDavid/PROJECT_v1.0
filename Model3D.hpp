#ifndef Model3D_hpp
#define Model3D_hpp

#include "Mesh.h"

#include "tiny_obj_loader.h"
#include "Libraries\include\stb\stb_image.h"

#include <iostream>
#include <string>
#include <vector>

    class Model3D {

    public:
        ~Model3D();

		void Unload();

		void LoadModel(std::string fileName);

		void LoadModel(std::string fileName, std::string basePath);

		void Draw(Shader shaderProgram, Camera camera);

		//draws the rest of the meshes that have alpha blending. Recommended to call once, after drawing all the normal meshes.
		static void RenderAlphaMeshes(Shader shaderProgram, Camera camera);

    private:
		// Component meshes - group of objects
        std::vector<Mesh> meshes;

		// To store alpha meshes, and draw them only at the end
		static std::vector<Mesh> alphaMeshes;

		// Associated textures
        std::vector<Texture> loadedTextures;

		// Does the parsing of the .obj file and fills in the data structure
		void ReadOBJ(std::string fileName, std::string basePath);

		// Retrieves a texture associated with the object - by its name and type
		Texture LoadTexture(std::string path, std::string type);

		// Reads the pixel data from an image file and loads it into the video memory
		GLuint ReadTextureFromFile(const char* file_name);
    };

#endif /* Model3D_hpp */
