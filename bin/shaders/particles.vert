#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

layout(location = 0) out vec4 particleColor;

layout(set = 0, binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

void main()
{
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(aPos, 1.f);
    particleColor = aColor;

    gl_PointSize = 4.0f;
}