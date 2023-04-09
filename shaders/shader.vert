#version 450 //use glsl 4.5

layout(location = 0) out vec3 fragColor;

//vertex positions
vec3 positions[3] = vec3[](
    vec3(0, -0.4, 0),
    vec3(0.4, 0.4, 0),
    vec3(-0.4, 0.4, 0)
);

vec3 colors[3] = vec3[](
vec3(1, 1, 0),
vec3(0, 1, 1),
vec3(1, 0, 1)
);

void main()
{
    gl_Position = vec4( positions[gl_VertexIndex] , 1.0f );
    fragColor = colors[gl_VertexIndex];
}