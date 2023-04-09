#version 450 //glsl version

layout(location = 0) in vec3 fragColor; //input from shader.vert

layout(location = 0) out vec4 outColor; //final output color, must have location 0. we output to the first attachment

void main()
{
    outColor = vec4(fragColor, 1.0f);
}