#version 330 core
out vec4 FragColor;

uniform int renderMode;  // 1 = WIREFRAME, 2 = VERTEX, 3 = FACE, 4 = FACE_EDGE
uniform vec3 wireframeColor;

// function to generate a pseudo-random color per triangle (only in FACE mode)
vec3 getRandomColor(int id) {
    return vec3(
        fract(sin(float(id) * 127.1) * 43758.5453),
        fract(sin(float(id) * 269.5) * 43758.5453),
        fract(sin(float(id) * 419.2) * 43758.5453)
    );
}

void main()
{
    if (renderMode == 1)
        FragColor = vec4(wireframeColor, 1.0);
    else if (renderMode == 2)
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else if (renderMode == 3 || renderMode == 4)
    {
        int primitiveID = gl_PrimitiveID;
        FragColor = vec4(getRandomColor(primitiveID), 1.0);
    }
}
