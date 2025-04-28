#version 410 core
layout(vertices = 25) out;  // define the number of vertices passed to TES (has to match the number of control points per patch specified on the CPU side)

uniform int tessLevel;

void main()
{
    // pass the vertex data through to TES without modification, using built-in variables
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // outer tessellation levels (number of subdivisions along the edges of the patch)
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;

    // inner tessellation levels (number of subdivisions along x and z axis within the patch, i.e., the internal grid)
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
}