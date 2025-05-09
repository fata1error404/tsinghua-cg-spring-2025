#version 330 core
layout (location = 0) in vec3 aPos;
layout (location=  1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 PosWorldSpace;
out vec4 PosLightSpace;
out vec3 Normal;
out vec2 TexCoord;
out vec4 ReflectCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 reflected_view;
uniform mat4 lightSpaceMatrix;
uniform float offset;

void main()
{
    PosWorldSpace = vec3(model * vec4(aPos, 1.0));
    PosLightSpace = lightSpaceMatrix * vec4(PosWorldSpace, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormal;         // apply normal matrix to ..
    TexCoord = aTexCoord + vec2(offset, 0.0);                   // animate water by offsetting texture coordinates horizontally
    ReflectCoord = projection * reflected_view * vec4(PosWorldSpace, 1.0); // reflect world position across a horizontal plane (planar reflection)
    gl_Position = projection * view * vec4(PosWorldSpace, 1.0);
}