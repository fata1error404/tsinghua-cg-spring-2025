#version 330 core
in vec3 TexCoord;
out vec4 FragColor;

uniform samplerCube skybox; // special texture sampler for referencing a 3D cubemap texture

void main()
{    
    FragColor = texture(skybox, TexCoord);
}