#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 Model;
    mat4 View;
    mat4 Proj;
} UBO;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = UBO.Proj * UBO.View * UBO.Model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}