# version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

layout(location = 0) out vec3 vertex_color;

void main()
{
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(aPos, 0.f, 1.f);
    vertex_color = aColor;
}