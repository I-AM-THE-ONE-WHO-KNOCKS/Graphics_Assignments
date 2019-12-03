#ifndef SPHERE_H
#define SPHERE_H

//external headers
#include<GL/glew.h>
#include<FreeImage.h>
//internal headers


class Sphere
{
public:
    //generates the vertices, colors, normals, texture coordinates of sphere
    void initSphere(GLint stacks, GLint slices, GLfloat radius, GLfloat squash);

    void drawSphere(int pLoc, int tloc);

    int mSunTextureID; //texture id for sun texture
    int mEarthTextureID; //texture id for earth texture
    int mMoonTextureID; // texture id for moon texture
    int mEarthSpecularTexID; // texture used for specular mapping

    int mSpecularTextureID; // specular texture id
};

#endif //SPHERE_H
