#version 410 core
layout(location = 0) in vec3 aPos;

uniform int isRenderControlPoints;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    if (isRenderControlPoints == 1) // not used
        gl_Position = projection * view * vec4(aPos, 1.0);
    else
        gl_Position = vec4(aPos, 1.0);
}