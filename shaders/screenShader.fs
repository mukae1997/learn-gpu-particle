#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 col = texture(screenTexture, TexCoords);
//    col = clamp(col,0.0,1.0);
//    FragColor = vec4(col.rgb, col.w);
    FragColor = vec4(col.rgb, 1.0);// debug
//    FragColor = vec4(vec3(0.5), col.w);// debug
}

