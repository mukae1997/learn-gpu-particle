
#version 330 core
in vec2 dataLoc;
in vec4 cle;

out vec4 FragColor;
uniform sampler2D physicsData;

void main()
{
    
    
    FragColor = vec4(1.0);
    
//    FragColor = vec4(dataLoc, 1.0, 1.0); // debug
}
