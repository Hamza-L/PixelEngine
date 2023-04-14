#version 450 //glsl version

layout(location = 0) out vec4 outColor; //final output color, must have location 0. we output to the first attachment

void main()
{
    outColor = vec4(1.0f,0.0f,0.0f, 1.0f);
}