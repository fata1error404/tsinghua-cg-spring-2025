#version 330 core
out vec4 FragColor;

uniform sampler2D particleTexture;

void main()
{
    vec4 texColor = texture(particleTexture, gl_PointCoord);
    
    // discard (make transparent) texture pixels that are nearly white, i.e. the snowflake's background
    if (length(texColor.rgb - vec3(1.0)) < 0.2)
        discard;

    FragColor = texColor;
}