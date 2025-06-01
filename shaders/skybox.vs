#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoord = aPos; // bind to cube vertices (texture coordinate is just a position on the surface of the unit cube)
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // a trick to force z = w so that after perspective division, depth is always 1.0, which is the max depth value at the far plane
}