#version 450

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 fragColor;

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1);
	fragColor = colors[gl_VertexIndex];
}