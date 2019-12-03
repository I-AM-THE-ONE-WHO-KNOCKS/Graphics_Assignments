#ifndef Utils_h
#define Utils_h

#define GLEW_STATIC
#include <GL/glew.h>
#include<GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#include<string>
#include<iostream>
#include<fstream>

int width = 0;
int height = 0;
int nChannels = 0;

class Utils
{
public:
	Utils();
	static GLuint loadTexture(const char* texImagePath);
};

GLuint Utils::loadTexture(const char *texImagePath)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    BYTE* data = stbi_load(texImagePath, &width, &height, &nChannels, 0);
    if(data != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        // define filtering i.e minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        return id;
    }

    return UINT_MAX;
}

#endif

