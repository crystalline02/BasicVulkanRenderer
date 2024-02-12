# version 460 core

layout(location = 0) in vec3 vertex_color;
layout(location = 1) in vec2 texture_coord;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 1) uniform sampler2D image;

void main()
{
    vec3 color = texture(image, texture_coord).rgb * vertex_color;
    FragColor = vec4(color, 1.f);
}