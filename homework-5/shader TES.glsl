#version 410 core
layout(quads, equal_spacing, ccw) in; // specify how the tessellated patch is structured

/*
    SETTINGS that control how the tessellation is applied:

    quads: tells OpenGL to tessellate the patch into four-sided shapes
    equal_spacing: tessellated points should be evenly spaced
    ccw: vertices should be in counterclockwise order
*/

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // get the tessellated coordinates (u, v) of the current vertex (that was created by the Tessellation Primitive Generator)
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    TexCoord = vec2(u, v); // pass to fragment shader (it will be automatically interpolated)

    // Bernstein basis polynomials for degree 4 that handle the influence of control points in u and v directions (two-dimensional Bezier surface)
    float bu[5] = float[5](
        (1 - u) * (1 - u) * (1 - u) * (1 - u),
        4 * u * (1 - u) * (1 - u) * (1 - u),
        6 * u * u * (1 - u) * (1 - u),
        4 * u * u * u * (1 - u),
        u * u * u * u
    );

    float bv[5] = float[5](
        (1 - v) * (1 - v) * (1 - v) * (1 - v),
        4 * v * (1 - v) * (1 - v) * (1 - v),
        6 * v * v * (1 - v) * (1 - v),
        4 * v * v * v * (1 - v),
        v * v * v * v
    );

    vec3 pos = vec3(0.0);
    
    // loop through each control point
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            int idx = i + j * 5; // map (i, j) grid position to 1D index
            pos += bu[i] * bv[j] * vec3(gl_in[idx].gl_Position);  // Bezier surface interpolation formula (interpolate the current vertex between the control points using Bernstein polynomials as weights)
        }
    }

    gl_Position = projection * view * vec4(pos, 1.0);  // pass tessellated and interpolated vertex (generated from control points) to the fragment shader
}