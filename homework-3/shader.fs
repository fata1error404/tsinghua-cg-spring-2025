#version 330 core
in vec4 particleColor;
out vec4 FragColor;

uniform sampler2D particleTexture; // texture is one for all particles, so it's applied globally via uniform and sent directly to the fragment shader 

void main()
{
    FragColor = texture(particleTexture, gl_PointCoord) * particleColor;
}