#version 450 //use glsl 4.5

layout(location = 0) in vec4 position;

void main()
{
    gl_Position = position;
}