#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D sampleTexture;

void main()
{
//    FragColor = texture(texture1, TexCoord);
//    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    vec4 texColor = texture(sampleTexture, TexCoord);
//    if(texColor.a < 0.1)
//        discard;
    // debug
    
    FragColor = texColor;
    if (gl_FragCoord.x<10) FragColor = vec4(1.0,0.0,0.0,1.0);
}
