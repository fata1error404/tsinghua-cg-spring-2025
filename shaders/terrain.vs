#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

out vec3 PosWorldSpace;
out vec4 PosLightSpace;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix; 

void main()
{
    PosWorldSpace = vec3(model * vec4(aPos, 1.0));
    PosLightSpace = lightSpaceMatrix * vec4(PosWorldSpace, 1.0);
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(PosWorldSpace, 1.0); 
}