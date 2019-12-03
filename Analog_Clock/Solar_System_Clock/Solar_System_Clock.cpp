#include <GL/glew.h>
#include <GL/freeglut.h>
#include "shader.h"
#include <stdio.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Sphere.h"
#include <time.h>
#include <sys/timeb.h>
#include "Utils.h"

GLint programID = -1;
Sphere* mSphere;

GLint positionLocation;
GLint colorLocation;
GLint modelMatrixLocation;
GLint projectionMatrixLocation;
GLint viewMatrixLocation;
GLint textureLocation;
GLint activeTextureLocation;

float zPos = 0.0;

int sunTextureId;
int earthTextureId;
int moonTextureId;
int jupitorTextureId;
int marsTextureId;
int mercuryTextureId;
int neptuneTextureId;
int saturnTextureId;
int uranusTextureId;
int venusTextureId;
int redTexture;
int whiteTexture;
int greenTexture;

void basicinit()
{
    mSphere = new Sphere();

    sunTextureId = Utils::loadTexture("texture/sun.tga");
    earthTextureId = Utils::loadTexture("texture/earth.tga");
    moonTextureId = Utils::loadTexture("texture/moon.tga");
    jupitorTextureId = Utils::loadTexture("texture/jupiter.tga");
    marsTextureId = Utils::loadTexture("texture/mars.tga");
    mercuryTextureId = Utils::loadTexture("texture/mercury.tga");
    neptuneTextureId = Utils::loadTexture("texture/neptune.tga");
    saturnTextureId = Utils::loadTexture("texture/saturn.tga");
    uranusTextureId = Utils::loadTexture("texture/uranus.tga");
    venusTextureId = Utils::loadTexture("texture/venus.tga");
    redTexture = Utils::loadTexture("texture/red.jpg");
    whiteTexture = Utils::loadTexture("texture/white.png");
    greenTexture = Utils::loadTexture("texture/green.png");
}

const float clockR = 36.0f,
clockVol = 100.0f,

angle1min = M_PI / 30.0f,

minStart = 0.9f,
minEnd = 1.0f,

stepStart = 0.8f,
stepEnd = 1.0f;

float angleHour = 0,
angleMin = 0,
angleSec = 0;

void newLine(float rStart, float rEnd, float angle) {
    float c = cos(angle), s = sin(angle);
    glVertex2f(clockR*rStart*c, clockR*rStart*s);
    glVertex2f(clockR*rEnd*c, clockR*rEnd*s);
}



void RenderScene(void) {
    int i;

    glBindTexture(GL_TEXTURE_2D, greenTexture);
    glLineWidth(2.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glBegin(GL_LINES);
    for (i = 0; i<60; i++) {
        if (i % 5) { // normal minute
            if (i % 5 == 1)
            glBindTexture(GL_TEXTURE_2D, greenTexture);
            newLine(minStart, minEnd, i*angle1min);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, greenTexture);
            newLine(stepStart, stepEnd, i*angle1min);
        }
    }
    glEnd();

    glLineWidth(3.0f);
    glBindTexture(GL_TEXTURE_2D, redTexture);
    glBegin(GL_LINES);
    newLine(0.0f, 0.3f, -angleHour + M_PI / 2);
    newLine(0.0f, 0.5f, -angleMin + M_PI / 2);
    glEnd();

    glLineWidth(1.0f);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glBegin(GL_LINES);
    newLine(0.0f, 0.5f, -angleSec + M_PI / 2);
    glEnd();
}



GLdouble g_eye[3] = { 0, 0, 64 };
const GLdouble LOOK_DISTANCE = 64;
GLdouble g_look[3] = { 0, 0, -1 };
GLdouble g_up[3] = { 0, 1, 0 };
const GLdouble MOVE_FACTOR = 1;
const GLdouble ZOOM_FACTOR = 2;

GLUquadricObj *Cylinder;
GLUquadricObj *Disk;

struct tm *newtime;
time_t ltime;


GLdouble vector_length(GLdouble vector[3]) {
    return sqrt(vector[0] * vector[0] + vector[1] * vector[1]
                + vector[2] * vector[2]);
}
void normalize_vector(GLdouble vector[3]) {
    GLdouble length = vector_length(vector);
    vector[0] = vector[0] / length;
    vector[1] = vector[1] / length;
    vector[2] = vector[2] / length;
}

void add_multiplied_vector(GLdouble vector[3], GLdouble factor,
                           GLdouble term[3]) {
    vector[0] += factor * term[0];
    vector[1] += factor * term[1];
    vector[2] += factor * term[2];
}

void assign_vector(GLdouble *destination, GLdouble source[3]) {
    *(destination+0) = source[0];
    *(destination+1) = source[1];
    *(destination+2) = source[2];
}

void assign_vector_normalized(GLdouble destination[3], GLdouble source[3]) {
    assign_vector(destination, source);
    normalize_vector(destination);
}

void cross_vector(GLdouble vector[3], GLdouble term[3]) {

    GLdouble temp[3];
    temp[0] = vector[1] * term[2] - vector[2] * term[1];
    temp[1] = vector[2] * term[0] - vector[0] * term[2];
    temp[2] = vector[0] * term[1] - vector[1] * term[0];
    assign_vector(vector, temp);
}

void init() {

    Shader shader("shader_solar.vert",
                   "shader_solar.frag");

    programID = shader.Program;

    //load the program object to the GPU
    shader.Use();

    positionLocation =
            glGetAttribLocation(programID, "a_Position");
    if(positionLocation < 0) {
        printf("Invalid location for a_Position");
    }

    textureLocation =
            glGetAttribLocation(programID, "a_Texcoord");
    if(textureLocation < 0) {
        printf("Invalid location for a_Texcoord");
    }

    activeTextureLocation =
            glGetUniformLocation(programID, "activeTexture");
    if(activeTextureLocation < 0) {
        printf("Invalid location for activeTexture");
    }

    modelMatrixLocation =
            glGetUniformLocation(programID, "u_ModelMatrix");

    projectionMatrixLocation =
            glGetUniformLocation(programID, "u_ProjectionMatrix");

    viewMatrixLocation =
            glGetUniformLocation(programID, "u_ViewMatrix");

}

float angle = 0.0;

void drawScene() {

    init();

    angle += 1.0;
    if( angle >= 360.0) angle = 0.0;

    zPos += 1.0;
    if(zPos >= 150.0) zPos = 0.0;

    glm::mat4 projMatrix = glm::mat4(1.0);
    projMatrix = glm::perspective(45.0, 1.0, 0.1, 100.0);

    glUniformMatrix4fv(projectionMatrixLocation,
                       1,
                       false,
                       &projMatrix[0][0]);

    GLdouble center[3];
    assign_vector(center, g_eye);
    add_multiplied_vector(center, LOOK_DISTANCE, g_look);

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(g_eye[0], g_eye[1], g_eye[2]),  //pos
                                       glm::vec3(center[0], center[1], center[2]),  //looking at
                                       glm::vec3(g_up[0], g_up[1], g_up[2])); //up vector
    glUniformMatrix4fv(viewMatrixLocation, 1, false, &viewMatrix[0][0]);



    mSphere->initSphere(50, 50, 3.0, 1.0);
    glm::mat4 modelMatrix;

    modelMatrix = glm::mat4(1.0);
           // glm::translate(glm::vec3(0, 0.0, -30.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));

    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTextureId);
    glUniform1i(activeTextureLocation, 0);

    //draw the sun
    mSphere->drawSphere(positionLocation, textureLocation);


    modelMatrix = glm::translate(modelMatrix, glm::vec3(17.0, 0.0, 5.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75, 0.75, 0.75));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, earthTextureId);
    //draw the earth
    mSphere->drawSphere(positionLocation, textureLocation);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0, 0.0, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3, 0.3, 0.3));
    //draw the moon
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, moonTextureId);

    mSphere->drawSphere(positionLocation, textureLocation);


    //draw mercury
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(6.0, 0.0, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3, 0.3, 0.3));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, mercuryTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);


    //draw venus
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 15.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.35, 0.35, 0.35));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, venusTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);

    //draw mars
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(10.0, 0.0, 10.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3, 0.3, 0.3));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, marsTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);


    //draw jupitor
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(15.0, 0.0, 25.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8, 0.8, 0.8));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, jupitorTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);

    //draw saturn
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(25.0, 0.0, -10.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7, 0.7, 0.7));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, saturnTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);


    //draw uranus
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(30.0, 0.0, -20.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4, 0.4, 0.4));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, uranusTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);

    //draw neptune
    modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle),
                              glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(30.0, 0.0, -30.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(angle+30), glm::vec3(0.0, 1.0, 0.0));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4, 0.4, 0.4));
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);

    glBindTexture(GL_TEXTURE_2D, neptuneTextureId);
    mSphere->drawSphere(positionLocation, textureLocation);

    modelMatrix = glm::mat4(1.0);
    glUniformMatrix4fv(modelMatrixLocation,
                       1,
                       false,
                       &modelMatrix[0][0]);
}

void draw() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    RenderScene();
    drawScene();

    glutSwapBuffers();
}

void timer(int val) {

    struct timeb tb;
    time_t tim = time(0);
    struct tm* t;
    t = localtime(&tim);
    ftime(&tb);

    angleSec = (float)(t->tm_sec + (float)tb.millitm / 1000.0f) / 30.0f * M_PI;
    angleMin = (float)(t->tm_min) / 30.0f * M_PI + angleSec / 60.0f;
    angleHour = (float)(t->tm_hour > 12 ? t->tm_hour - 12 : t->tm_hour) / 6.0f * M_PI +
        angleMin / 12.0f;

    draw();
    glutTimerFunc(33, timer, 1);
}

void resize(int w, int h) {
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
            if (state == GLUT_UP) {
                return;
            }
            GLdouble direction_factor = button == 3 ? 1 : -1;
            add_multiplied_vector(g_eye, direction_factor * ZOOM_FACTOR,
                                  g_look);
}

void keyboard(unsigned char key, int x, int y) {
    GLdouble direction_factor = 1;
    switch (key) {
        case 'I':
        case 'i':
        {
            GLdouble temp1[3] = { 0, 0, 64 };
            assign_vector(g_eye, temp1) ;
            GLdouble temp2[3] = { 0, 0, -1 };
            assign_vector(g_look, temp2);
            GLdouble temp3[3] = { 0, 1, 0 };
            assign_vector(g_up, temp3);
        }
            break;
        case 'j':
        case 'J':
        {
            GLdouble temp1[3] = { 0, 0, 24 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = { 0, 0, -1 };
            assign_vector(g_look, temp2);
            GLdouble temp3[3] = { 0, 1, 0 };
            assign_vector(g_up, temp3);
        }
            break;
        case 'N':
        case 'n':
    {
            GLdouble temp1[3] = { 0, -64, 16 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = { 0, 64, -16 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = { 0, 16, 64 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'm':
        case 'M':
    {
            GLdouble temp1[3] = { 0, -24, 3 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = { 0, 24, -3 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = { 0, 3, 24 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'K':
        case 'k':
    {
            GLdouble temp1[3] = { 0, -64, 0 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = { 0, 64, 0 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = { 0, 0, 64 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'o':
        case 'O':
    {
            GLdouble temp1[3] = { 0, -24, 0 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = { 0, 24, 0 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = { 0, 0, 24 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'p':
        case 'P':
    {
            GLdouble temp1[3] = { -64, 0, 16 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = {64, 0, -16 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = {16, 0, 64 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'L':
        case 'l':
    {
            GLdouble temp1[3] = { -24, 0, 3 };
            assign_vector(g_eye, temp1);
            GLdouble temp2[3] = {24, 0, -3 };
            assign_vector_normalized(g_look, temp2);
            GLdouble temp3[3] = {3, 0, 24 };
            assign_vector_normalized(g_up, temp3);
    }
            break;
        case 'd':
        case 'D':
            direction_factor = -1;
        case 'a':
        case 'A': {
            GLdouble direction[3];
            assign_vector(direction, g_look);
            cross_vector(direction, g_up);
            add_multiplied_vector(g_eye, direction_factor * MOVE_FACTOR,
                                  direction);
            break;
        }
        case 'x':
        case 'X':
            direction_factor = -1;
        case 'Z':
        case 'z':
            add_multiplied_vector(g_eye, direction_factor * MOVE_FACTOR,
                                  g_look);
            break;
        case 'w':
        case 'W':
            direction_factor = -1;
        case 's':
        case 'S':
            add_multiplied_vector(g_eye, direction_factor * MOVE_FACTOR, g_up);
            break;
        default:
            // Do nothing.
            break;
    }
}

int main(int argc, char** argv)
{
    //initialize the glut library
    glutInit(&argc, argv);

    //set the appropriate display mode
    //this configures the frame buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

    //define the size of the window
    glutInitWindowSize(1200, 720);

    glutCreateWindow("Solar System clock");

     //initialize the glew library
    glewInit();

    basicinit();

    glutDisplayFunc(draw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glutTimerFunc(10, timer, 1);
    //start the glut event loop
    glutMainLoop();


    return 0;
}
