
#version 330 core
layout (location = 0) in vec2 aDataLoc;

out vec2 dataLoc;

uniform sampler2D physicsData;

// debug
out vec4 cle;

void main()
{
    vec4 particle = texture(physicsData, aDataLoc);
    cle = particle;
    
    gl_Position = vec4(clamp(particle.xyz,-1.0, 1.0),1.0);
//    gl_Position = vec4(aDataLoc.xy, 1.0, 1.0);
    
    float perspective =  particle.z * 0.15;
//    gl_PointSize = 5.0;
    gl_PointSize = min(64.0, (2.0 * perspective));
    
    dataLoc = aDataLoc;
}
