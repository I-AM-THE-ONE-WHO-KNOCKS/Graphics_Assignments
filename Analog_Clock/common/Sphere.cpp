
//external headers
#include <GL/glew.h>
#include <stdlib.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <stdio.h>
#include "iostream"
//internal headers
#include "Sphere.h"

#define M_PI 3.14159265358979323846


GLfloat			*m_VertexData;
GLubyte			*m_ColorData;
GLfloat			*m_NormalData;
GLfloat         *m_TexCoordsData;
GLint			m_Stacks, m_Slices;
GLfloat			m_Scale;						
GLfloat			m_Squash;
GLfloat			m_Angle;
GLfloat			m_Pos[3];


void Sphere::initSphere(GLint stacks, GLint slices, GLfloat radius, GLfloat squash)
{
    unsigned int colorIncrment=0;
    unsigned int blue=10;
    unsigned int red=10;
    int numVertices=0;
    
    m_Scale=radius;
    m_Squash=squash;
    
    colorIncrment=255/stacks;
    
        m_Stacks = stacks;
        m_Slices = slices;
        m_VertexData = NULL;
        m_TexCoordsData = NULL;
        
        //vertices
        
        GLfloat *vPtr = m_VertexData =
        (GLfloat*)malloc(sizeof(GLfloat) * 3 * ((m_Slices*2+2) *
                                                (m_Stacks)));
        
        //color data
        
        GLubyte *cPtr = m_ColorData =
        (GLubyte*)malloc(sizeof(GLubyte) * 4 * ((m_Slices*2+2) *
                                                (m_Stacks)));
        
        //normal pointers for lighting
        
        GLfloat *nPtr = m_NormalData = (GLfloat*)
        malloc(sizeof(GLfloat) * 3 * ((m_Slices*2+2) * (m_Stacks)));
        
        GLfloat *tPtr=NULL;                                          //3
        
            tPtr=m_TexCoordsData =
            (GLfloat *)malloc(sizeof(GLfloat) * 2 * ((m_Slices*2+2) *
                                                     (m_Stacks)));
        
        unsigned int phiIdx, thetaIdx;
        
        //latitude
        
        for(phiIdx=0; phiIdx < m_Stacks; phiIdx++)
        {
            //starts at -1.57 goes up to +1.57 radians
            
            //the first circle
            
            float phi0 = M_PI * ((float)(phiIdx+0) * (1.0/(float)
                                                      (m_Stacks)) - 0.5);
            
            //the next, or second one.
            
            float phi1 = M_PI * ((float)(phiIdx+1) * (1.0/(float)
                                                      (m_Stacks)) - 0.5);
            float cosPhi0 = cos(phi0);
            float sinPhi0 = sin(phi0);
            float cosPhi1 = cos(phi1);
            float sinPhi1 = sin(phi1);
            
            float cosTheta, sinTheta;
            
            //longitude
            
            for(thetaIdx=0; thetaIdx < m_Slices; thetaIdx++)
            {
                //Increment along the longitude circle each "slice."
                
                float theta = -2.0*M_PI * ((float)thetaIdx) *
                (1.0/(float)(m_Slices-1));
                cosTheta = cos(theta);
                sinTheta = sin(theta);
                
                //We're generating a vertical pair of points, such
                //as the first point of stack 0 and the first point
                //of stack 1
                //above it. This is how TRIANGLE_STRIPS work,
                //taking a set of 4 vertices and essentially drawing
                //two triangles
                //at a time. The first is v0-v1-v2 and the next is
                //v2-v1-v3. Etc.
                
                //Get x-y-z for the first vertex of stack.
                
                vPtr[0] = m_Scale*cosPhi0 * cosTheta;
                vPtr[1] = m_Scale*sinPhi0*m_Squash;
                vPtr[2] = m_Scale*(cosPhi0 * sinTheta);
                
                //the same but for the vertex immediately above the
                //previous one
                
                vPtr[3] = m_Scale*cosPhi1 * cosTheta;
                vPtr[4] = m_Scale*sinPhi1*m_Squash;
                vPtr[5] = m_Scale*(cosPhi1 * sinTheta);
                
                //normal pointers for lighting
                
                nPtr[0] = cosPhi0 * cosTheta;
                nPtr[2] = cosPhi0 * sinTheta;
                nPtr[1] = sinPhi0;
                
                nPtr[3] = cosPhi1 * cosTheta;
                nPtr[5] = cosPhi1 * sinTheta;
                nPtr[4] = sinPhi1;
                
                if(tPtr!=NULL)                               //4
                {
                    GLfloat texX = (float)thetaIdx *
                    (1.0f/(float)(m_Slices-1));
                    tPtr[0] = texX;
                    tPtr[1] = (float)(phiIdx+0) *
                    (1.0f/(float)(m_Stacks));
                    tPtr[2] = texX;
                    tPtr[3] = (float)(phiIdx+1) *
                    (1.0f/(float)(m_Stacks));
                }
                
                cPtr[0] = red;
                cPtr[1] = 0;
                cPtr[2] = blue;
                cPtr[4] = red;
                cPtr[5] = 0;
                cPtr[6] = blue;
                cPtr[3] = cPtr[7] = 255;
                
                cPtr += 2*4;
                vPtr += 2*3;
                nPtr += 2*3;
                
                
                if(tPtr!=NULL)                               //5
                    tPtr += 2*2;
            }
            
            blue+=colorIncrment;
            red-=colorIncrment;
            
            // Degenerate triangle to connect stacks and maintain
            //winding order.
            
            vPtr[0] = vPtr[3] = vPtr[-3];
            vPtr[1] = vPtr[4] = vPtr[-2];
            vPtr[2] = vPtr[5] = vPtr[-1];
            
            nPtr[0] = nPtr[3] = nPtr[-3];
            nPtr[1] = nPtr[4] = nPtr[-2];
            nPtr[2] = nPtr[5] = nPtr[-1];
            
            if(tPtr!=NULL)
            {
                tPtr[0] = tPtr[2] = tPtr[-2];         //6
                tPtr[1] = tPtr[3] = tPtr[-1];
            }
            
        }
        
        numVertices=(vPtr-m_VertexData)/6;
}

void Sphere::drawSphere(int pLoc, int tloc)
{
    glEnableVertexAttribArray(pLoc);
    glEnableVertexAttribArray(tloc);

    glVertexAttribPointer(pLoc, 3, GL_FLOAT, false, 0, m_VertexData);
    glVertexAttribPointer(tloc, 2, GL_FLOAT, false, 0, m_TexCoordsData);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, (m_Slices+1)*2*(m_Stacks-1)+2);
    glDisableVertexAttribArray(pLoc);
    glDisableVertexAttribArray(tloc);
}
