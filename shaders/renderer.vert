
// renderer vertex shader
#version 330 core

layout(std140) uniform CBPerObject
{
    mat4 gWorldViewProj;
};

layout(location = 0) in vec3 inPosition; // layout(location = <n>) in <datatype> <변수명>
layout(location = 1) in vec3 inNormal; 
layout(location = 2) in vec2 inUv; 

out vec2 uv;

void main()
{
    // gl_Position이 hlsl의 SV_POSITION 같은 것
    gl_Position = gWorldViewProj * vec4(inPosition, 1.0);
    uv = inUv;
}