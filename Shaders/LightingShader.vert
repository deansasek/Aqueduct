#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 Model;
    mat4 View;
    mat4 Proj;
} UBO;

layout(location = 0) in vec3 InPosition;

void main() {
    gl_Position = UBO.Proj * UBO.View * UBO.Model * vec4(InPosition, 1.0);
}