
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
    
    gl_Position = vec4(particle.xy, 0.5,1.0);
    
    gl_PointSize = 3.0;
    
    dataLoc = aDataLoc;
}
