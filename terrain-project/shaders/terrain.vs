#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoord = aTexCoord;
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(Position, 1.0); 
}