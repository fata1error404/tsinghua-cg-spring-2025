#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in float aSize;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = aSize;
}