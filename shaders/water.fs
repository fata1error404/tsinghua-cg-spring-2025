#version 330 core

in vec3 PosWorldSpace;
in vec4 PosLightSpace;
in vec3 Normal;
in vec2 TexCoord;
in vec4 ReflectCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D waterTexture;
uniform sampler2D terrainReflectionTexture;
uniform samplerCube skyboxReflectionTexture;
uniform float terrainReflectionStrength;
uniform float skyboxReflectionStrength;
uniform vec3 skyboxScaleRatio;

uniform bool lighting;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform sampler2D shadowMap;

uniform bool weather;
uniform vec4 fogColor;
uniform float fogStart;
uniform float fogEnd;

//! Applies distance-based fog.
vec3 applyFog(vec3 sceneColor) {
    float dist = length(cameraPos - PosWorldSpace);
    float linearF = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0); // calculate linear fog factor

    return mix(sceneColor, fogColor.rgb, linearF);
}

//! Checks if the fragment is in shadow, returns 1.0 if true.
float checkShadow(vec4 PosLightSpace)
{
    vec3 projCoords = PosLightSpace.xyz / PosLightSpace.w; // transform the fragment position into light space to range [-1, 1] (Normalized Device Coordinates)
    projCoords = projCoords * 0.5 + 0.5;                   // convert to [0, 1] range

    float currentDepth = projCoords.z;                        // distance from the light to the current fragment
    float closestDepth = texture(shadowMap, projCoords.xy).r; // distance from the light to the nearest surface at the current fragment's position (sampled from the shadow map)

    return (currentDepth > closestDepth + 0.005) ? 1.0 : 0.0; // if the fragment is farther than the stored closest depth + bias, it's in shadow
}

void main()
{
    // base water
    vec4 baseColor = texture(waterTexture, TexCoord);
    vec3 result = baseColor.rgb;

    // planar terrain reflection
    vec3 projCoords = ReflectCoord.xyz / ReflectCoord.w;
    projCoords = projCoords * 0.5 + 0.5;
    vec4 terrainRefl = texture(terrainReflectionTexture, projCoords.xy);

    // skybox reflection
    vec3 N = normalize(Normal);
    vec3 I = normalize(cameraPos - PosWorldSpace); // view direction vector
    vec3 R = reflect(-I, N);                       // reflection vector
    vec4 skyRefl = texture(skyboxReflectionTexture, normalize(R / skyboxScaleRatio));

    vec3 reflection = terrainRefl.rgb * terrainReflectionStrength + skyRefl.rgb * skyboxReflectionStrength;

    // lighting (Phong lighting model: ambient + diffuse + specular)
    if (lighting)
    {
        vec3 L = normalize(lightPos - PosWorldSpace); // light direction vector
        float diff = max(dot(N, L), 0.0);             // measure how aligned the surface is with the light (cos = 1 means the light hits water surface directly)
        float isInShadow = checkShadow(PosLightSpace);

        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(I, R), 0.0), 64.0); // measure how aligned the view direction is with the reflected light (cos = 1 means the light reflection hits the camera)

        vec3 ambient = ambientStrength  * lightColor;
        vec3 diffuse = diffuseStrength  * lightColor * diff * (1.0 - isInShadow); // apply diffuse lighting only if the fragment is not in shadow (this allows for rendering shadowed areas)

        vec3 specular = specularStrength * lightColor * spec;

        result = (ambient + diffuse + specular) * result;
    }

    result = mix(result, reflection + result, 0.5); // reflection blending

    if (weather)
        result.rgb = applyFog(result.rgb);

    FragColor = vec4(result, 1.0);
}