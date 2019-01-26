//
//  GraphicObject.hpp
//  opengl-revise
//
//  Created by mukae on 2019/1/22.
//  Copyright © 2019年 mukae. All rights reserved.
//

#ifndef GraphicObject_hpp
#define GraphicObject_hpp

#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
 
#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
class GraphicObject {
    
public:
    
    int number_of_vertices;
    unsigned int VAO;
    unsigned int texture;
    
    
    GraphicObject(); 
    void setTexture(const char* filepath);
    void draw();
};


#endif /* GraphicObject_hpp */

