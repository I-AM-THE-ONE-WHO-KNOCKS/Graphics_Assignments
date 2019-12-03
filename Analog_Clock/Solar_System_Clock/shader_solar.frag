varying vec2 v_Texcoord;
uniform sampler2D activeTexture;

void main(void)
{
    vec4 color = texture2D(activeTexture, v_Texcoord);
    gl_FragColor = color;
}
