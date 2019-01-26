#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
//    FragColor = texture(texture1, TexCoord);
//    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    vec4 texColor = texture(texture1, TexCoord);
//    if(texColor.a < 0.1)
//        discard;
    FragColor = texColor;
}
