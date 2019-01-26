#version 330 core
out vec4 FragColor;

uniform vec2 bounds;
uniform sampler2D physicsInput;


vec4 texel(vec2 offset) {
    vec2 coord = (gl_FragCoord.xy + offset) / bounds;
    return texture(physicsInput, coord);
}

void main()
{
    if (gl_FragCoord.x < 0.5 * bounds.x)
        FragColor = vec4(0.0,gl_FragCoord.x/bounds.x,0.0,1.0);
    else
        FragColor = texel(vec2(0.0, 0.0));
    
    
//    FragColor = vec4(0.0,0.0,max(0,gl_FragCoord.x-0.5),1.0);
}
