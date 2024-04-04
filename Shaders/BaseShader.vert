#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 Model;
    mat4 View;
    mat4 Proj;
    vec4 Position;
} UBO;

layout(push_constant) uniform PushConstant
{
    mat4 Model;
} PC;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec2 UV;

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec2 FragTexCoord;

void main() {
    gl_Position = UBO.Proj * UBO.View * PC.Model * vec4(Position, 1.0);
    FragColor = Color;
    FragTexCoord = UV;
}