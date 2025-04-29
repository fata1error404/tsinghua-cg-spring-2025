#version 330 core

out vec4 FragColor;

uniform vec4 rainColor;
uniform sampler2D particleTexture;

void main()
{
    float mask = texture(particleTexture, gl_PointCoord).r;
    FragColor = vec4(rainColor.rgb, mask * rainColor.a); // apply transparency mask to get the desired particle shape
}