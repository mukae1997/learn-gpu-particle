
#version 330 core
layout (location = 0) in vec2 aDataLoc;

out vec2 dataLoc;

uniform sampler2D physicsData;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;



// debug
out vec4 cle;

void main()
{
    vec4 particle = texture(physicsData, aDataLoc);
    vec3 position = particle.xyz;
    
    
//    gl_Position = vec4(aDataLoc.xy, 1.0, 1.0);
    
    float perspective = 1.0 + particle.z * 5.5;
//    gl_PointSize = 2.0; // debug
    gl_Position = projection * view * model * vec4(position, 1.0f);
//    gl_Position = vec4(clamp(position,-1.0, 1.0),1.0);
    gl_PointSize = min(54.0,1.0);
    
    dataLoc = aDataLoc;
}
