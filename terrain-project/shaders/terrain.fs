#version 330 core
in vec2 TexCoord;
in vec3 Position;

out vec4 FragColor;

uniform vec3 skyboxScaleRatio;
uniform vec3 cameraPos; 
uniform float reflectivity;
uniform sampler2D terrainTexture;
uniform sampler2D detailTexture;
uniform samplerCube skyboxTexture;

void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, vec3(0, 1, 0));
    vec3 R_scaled = normalize(R / skyboxScaleRatio);

    vec4 reflection = texture(skyboxTexture, R_scaled);

    vec4 terrain = texture(terrainTexture, TexCoord);
    vec4 detail = texture(detailTexture, TexCoord * 50.0); // tile detail texture
    vec4 result = terrain + detail - 0.5; // GL_ADD_SIGNED behavior

    FragColor = mix(result, reflection, reflectivity);
}