
// skybox vertex shader
#version 330 core

layout(std140) uniform CBPerObject
{
    mat4 gWorldViewProj;
};

layout(location = 0) in vec3 inPosition;

out vec3 dir;

void main()
{
    dir = inPosition;
    vec4 pos = gWorldViewProj * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;
}
