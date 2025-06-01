#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main()
{
    float dist = distance(aPos, cameraPos);
    float size = clamp(100.0 / dist, 2.0, 500.0); // vary raindrop size based on distance from camera, clamped to keep it in range [2, 500]

    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = size;
}