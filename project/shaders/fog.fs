#version 330 core

out vec4 FragColor;

uniform vec4 fogColor;
uniform sampler2D particleTexture;
 
void main()
{
    float mask = texture(particleTexture, gl_PointCoord).r;
    FragColor = vec4(fogColor.rgb, mask * fogColor.a);
}