#version 330 core
in vec2 TexCoord;
in vec3 Position; // vertex coordinates of the water surface in world space

out vec4 FragColor;

uniform vec3 skyboxScaleRatio;
uniform vec3 cameraPos; // camera position in world space
uniform float reflectivity; // [0, 1] mix factor
uniform sampler2D waterTexture;
uniform samplerCube skyboxTexture;

uniform vec3  fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;
uniform float seaLevel;
uniform float fogHeight;

vec3 applyFog(vec3 sceneColor) {
    // 1) distance‐based fog factor (linear)
    float dist = length(Position - cameraPos);
    float linearF = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    // OR for exponential fog:
    // float expF = 1.0 - exp(-fogDensity * dist * dist);

    // 2) height‐based “ground fog” factor
    //    → fully fogged at seaLevel, zero fog at seaLevel+fogHeight
    float heightF = clamp((seaLevel + fogHeight - Position.y) / fogHeight, 0.0, 1.0);

    // 3) combine them (multiply so fog is strongest when both distant & low)
    float fogFactor = linearF * heightF;

    // 4) mix scene color with fog color
    return mix(sceneColor, fogColor, fogFactor);
}

void main()
{
    vec3 I = normalize(Position - cameraPos); // calculate the view direction from the camera to the fragment
    vec3 R = reflect(I, vec3(0, 1, 0)); // calculate the reflection vector
    vec3 R_scaled = normalize(R / skyboxScaleRatio);

    vec4 reflection = texture(skyboxTexture, R_scaled); // reflection color – sample the cubemap using the reflection vector as the direction
    
    vec4 water = texture(waterTexture, TexCoord); // base water color

    vec4 Color = mix(water, reflection, reflectivity);

    // vec3 final = applyFog(Color.rgb);
    FragColor = vec4(Color);
}