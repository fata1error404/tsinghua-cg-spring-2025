#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

uniform int renderMode;  // 0 = WIREFRAME, 1 = TEXTURE
uniform sampler2D ourTexture;

void main()
{
    if (renderMode == 0)
        FragColor = vec4(0.5f, 0.7f, 1.0f, 1.0f);
    else
        FragColor = texture(ourTexture, TexCoord);
}