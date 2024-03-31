#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 Model;
    mat4 View;
    mat4 Proj;
} UBO;

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec2 InTexCoord;

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec2 FragTexCoord;

void main() {
    gl_Position = UBO.Proj * UBO.View * UBO.Model * vec4(InPosition, 1.0);
    FragColor = InColor;
    FragTexCoord = InTexCoord;
}