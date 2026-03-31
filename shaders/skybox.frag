// skybox fragment shader
#version 330 core

in vec3 dir;
out vec4 fragColor;

uniform samplerCube gTex;


void main()
{
    fragColor = texture(gTex, normalize(dir));
}
