#version 330 core

in vec3 TexCoord;
out vec4 FragColor;

uniform samplerCube skybox; // special texture sampler for referencing a 3D cubemap texture
uniform bool weather;
uniform float fogDensity;

void main()
{
    FragColor = texture(skybox, TexCoord);

    if (weather)
        FragColor.rgb = mix(FragColor.rgb, vec3(1.0), fogDensity);
}