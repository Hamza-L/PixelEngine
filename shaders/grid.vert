#version 450 //use glsl 4.5

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texUV;

layout(set = 0, binding = 0) uniform UboVP
{
    mat4 V;
    mat4 P;
    vec4 lightPos;
} uboVP;

layout(push_constant) uniform PObj
{
    mat4 M;
    mat4 MinvT;
} pushObj;

void main()
{
    gl_Position = vec4(0.0f,0.0f,0.0f,1.0f);
}