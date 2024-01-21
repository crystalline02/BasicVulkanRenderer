# version 460 core

layout(location = 0) in vec3 vertex_color;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(vertex_color, 1.f);
}