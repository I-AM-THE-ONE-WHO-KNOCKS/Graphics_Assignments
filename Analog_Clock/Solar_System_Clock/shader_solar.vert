
attribute vec4 a_Position;
attribute vec2 a_Texcoord;
varying vec2 v_Texcoord;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

void main(void)
{
    gl_Position =
            u_ProjectionMatrix *
            u_ViewMatrix *
            u_ModelMatrix *
            a_Position;

    v_Texcoord = a_Texcoord;
}
