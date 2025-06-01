#version 330 core

in vec3 PosWorldSpace;
in vec4 PosLightSpace;
in vec2 TexCoord; 

out vec4 FragColor;

uniform float clipPlane;
uniform float detailLevel;
uniform sampler2D mainTexture;
uniform sampler2D detailTexture;
uniform samplerCube skyboxReflectionTexture;
uniform vec3 skyboxScaleRatio;

uniform bool lighting;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform sampler2D shadowMap;

//! Checks if the fragment is in shadow, returns 1.0 if true.
float checkShadow(vec4 PosLightSpace)
{
    vec3 projCoords = PosLightSpace.xyz / PosLightSpace.w; // transform the fragment position into light space
    projCoords = projCoords * 0.5 + 0.5;                   // convert to [0, 1] range

    float currentDepth = projCoords.z;                        // distance from the light to the current fragment
    float closestDepth = texture(shadowMap, projCoords.xy).r; // distance from the light to the nearest surface at the current fragment's position (sampled from the shadow map)

    return (currentDepth > closestDepth + 0.005) ? 1.0 : 0.0; // if the fragment is farther than the stored closest depth + bias, it's in shadow
}

void main()
{    
    if (PosWorldSpace.y < clipPlane)
        discard;

    // base terrain + detail
    vec4 terrain = texture(mainTexture, TexCoord);
    vec4 detail = texture(detailTexture, TexCoord * detailLevel);
    vec4 baseColor = terrain + detail - 0.5;

    // lighting (ambient + diffuse)
    if (lighting)
    {
        vec3 N = normalize(cross(dFdx(PosWorldSpace), dFdy(PosWorldSpace))); // compute the fragment's normal vector as the cross product of partial derivatives (obtained using built-in functions) of its world space position
        vec3 L = normalize(lightPos - PosWorldSpace);
        float diff = max(dot(N, L), 0.0);
        float isInShadow = checkShadow(PosLightSpace);

        vec3 ambient = ambientStrength * lightColor;
        vec3 diffuse = diffuseStrength * lightColor * diff * (1.0 - isInShadow); // apply diffuse lighting only if the fragment is not in shadow (this allows for rendering shadowed areas)

        vec3 result = (ambient + diffuse) * baseColor.rgb;

        FragColor = vec4(result, 1.0);
    }
    else
        FragColor = baseColor;
}