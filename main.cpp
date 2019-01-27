#define RETINA_FACTOR 2

#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>
#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "GraphicObject.hpp"

//    窗口调整的时候 视口应该也被调整  对窗口注册一个回调函数每次窗口大小被调整的时候会被调用
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void processInput(GLFWwindow *window);
unsigned int setupVerticesData();
unsigned int loadFile(const char* filename, char** buffer);
void loadShaderSources();

int createShader(char** vsptr, char** fsptr, const char* vsFile, const char* fsFile);
/*
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";
 */
char*vertexShaderSource, *fragmentShaderSource;
char* screenVS, *screenFS;
char* physicsVS, *physicsFS;

int SCR_WIDTH = 800, SCR_HEIGHT = 600;

int viewportWidth = 800*RETINA_FACTOR, viewportHeight = 600*RETINA_FACTOR;
// texture IDs
unsigned int physicsInputTexture, physicsOutputTexture, pointSpriteTexture;
unsigned int canvasCopyTexture;
void createCanvasCopyTexture();
void createPhysicsTexture(unsigned int * texptr, float** data);

int PARTICLE_COUNT_SQRT = 512;
int PARTICLE_COUNT = PARTICLE_COUNT_SQRT * PARTICLE_COUNT_SQRT, DATA_SLOT_SIZE = 2;
int PARTICLE_DATA_SLOTS = 2;
int PARTICLE_DATA_WIDTH = PARTICLE_COUNT_SQRT * PARTICLE_DATA_SLOTS;
int PARTICLE_DATA_HEIGHT = PARTICLE_COUNT_SQRT;



int main(int argc, char **argv){
    using std::cout;    using std::endl;
    glfwInit();
    //    主版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //    次版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //    告诉glfw 我们使用的是core_profile 核心模块
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //    向前兼容
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //    创建一个GLFW 窗口   宽 高  窗口名字  后边两个暂时不用管
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learn GPU Particle System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, viewportWidth, viewportHeight);
    
    
    //   通过 glfwSetFramebufferSizeCallback glfw 函数 当窗口frame 变化时会调用。
    //    对于视网膜屏 Retain 屏   宽度和高度明显比原输入值更高一点。
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    GraphicObject go;
    go.setTexture("window.png");
    
    
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
//    loadShaderSources();
    
    int shaderProgram = createShader(&vertexShaderSource, &fragmentShaderSource, "basic.vs", "basic.fs");
    int screenShaderProgram = createShader(&screenVS, &screenFS, "./shaders/screenShader.vs", "./shaders/screenShader.fs");
    int physicsProgram = createShader(&physicsVS, &physicsFS, "./shaders/physics.vs", "./shaders/physics.fs");
  
    
    int VAO = setupVerticesData();
    int number_of_vertices = 36;
    
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    
    std::cout << " > Loading images ... " << std::endl;
    unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    
    
    
    glUseProgram(shaderProgram);
    
    // create transformations
    glm::mat4 model         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    glm::mat4 view          = glm::mat4(1.0f);
    glm::mat4 projection    = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 0.5f, 0.0f));
    view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.3f));
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    // retrieve the matrix uniform locations
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectLoc  = glGetUniformLocation(shaderProgram, "projection");
    // pass them to the shaders (3 different ways)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
//    ourShader.setMat4("projection", projection);
    glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    
    
    ///////////////////////////
    // set texture(why?)
    ///////////////////////////
    
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUseProgram(screenShaderProgram);
    glUniform1i(glGetUniformLocation(screenShaderProgram, "screenTexture"), 0);
    
    
    ///////////////////////////
    /////  FRAME BUFFER  //////
    ///////////////////////////
    
    
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    createCanvasCopyTexture();
    
    int PHYSICS_TEX_DATA_LENGTH = 4 * PARTICLE_COUNT * DATA_SLOT_SIZE;
    float* physicsInputData = new float[PHYSICS_TEX_DATA_LENGTH];
    // debug filling
    for (int i = 2; i < PHYSICS_TEX_DATA_LENGTH; i += 4){
        physicsInputData[i] = 1.0f;
    }
    
    createPhysicsTexture(&physicsInputTexture, &physicsInputData);
    float* physicsOutputData = new float[PHYSICS_TEX_DATA_LENGTH];
    createPhysicsTexture(&physicsOutputTexture, &physicsOutputData);
    
    
    
    
    
    
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    } else {
        cout << "> Framebuffer complete." << endl;
    }
    // unbind frameBuffer (i.e. return to default frameBuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //////////////
    //// quad ////
    //////////////
    
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    
    
    /////////////////////
    ///// PARTICLES /////
    /////////////////////
    
    glEnable(GL_PROGRAM_POINT_SIZE);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvasCopyTexture, 0);
        
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(120/255.0f, 173/255.0f, 211/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //////////////////////
        if (true) {
        
            glUseProgram(shaderProgram);
            
            // draw our first triangle
            glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            // bind Texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glActiveTexture(GL_TEXTURE0);
            
            model = glm::mat4(1.0f);
            model = glm::translate(model,  glm::vec3(0.0f,0.0f, -0.9f));
            model = glm::rotate(model, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, number_of_vertices);
            
            model = glm::translate(model,  glm::vec3(1.0f,1.0f, -0.9f));
            model = glm::rotate(model, 1.0f, glm::vec3(-5.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, number_of_vertices);
            
            model = glm::mat4(1.0f);
            model = glm::translate(model,  glm::vec3(0.0f,0.0f, -2.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            go.draw();
            
            
            model = glm::translate(model,  glm::vec3(0.0f,0.0f, 3.2f));
            model = glm::rotate(model, (float)glfwGetTime() * 1, glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            go.draw();
            
            glBindVertexArray(0);
        
        }
        
        /////////////////////////
        //////// PHYSICS ////////
        /////////////////////////
        if (!true){
        // bind output-texture to fbo
//        glBindFramebuffer(GL_FRAMEBUFFER, fbo); // render to frameBuffer
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, physicsOutputTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // render to screen
//        calculatePhysics();
        glDisable(GL_DEPTH_TEST);
        
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, PARTICLE_DATA_WIDTH, PARTICLE_DATA_HEIGHT);
        
        
        glUseProgram(physicsProgram);
        glBindVertexArray(quadVAO);
        
        
        // feed input
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, physicsInputTexture);
        glUniform1i(glGetUniformLocation(physicsProgram, "physicsInput"), 0);
        
        
        // set data texture size
        glUniform2f(glGetUniformLocation(physicsProgram, "bounds"), PARTICLE_DATA_WIDTH, PARTICLE_DATA_HEIGHT);
        
        
        
        
        glDrawArrays(GL_TRIANGLES, 0, 6); // debug
        
        glBindVertexArray(0);
        
        }
        
        
        /////////////////////////
        ////// OFF-SCREEN ///////
        /////////////////////////
    
        if (true) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            // clear all relevant buffers
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
            glClear(GL_COLOR_BUFFER_BIT);

            
            glViewport(0, 0, viewportWidth, viewportHeight);
            glUseProgram(screenShaderProgram);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, canvasCopyTexture);
            glUniform1i(glGetUniformLocation(screenShaderProgram, "screenTexture"), 0);
            
            glBindVertexArray(quadVAO); // use the color attachment texture as the texture of the quad plane
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
        
        
        glfwSwapBuffers(window);
        //        glfwPollEvents 检查函数有没有触发什么事件 键盘输入 鼠标移动 并调用对应函数
        glfwPollEvents();
    }
    
    glDeleteFramebuffers(1, &fbo);
    glfwTerminate();
    
    delete vertexShaderSource;
    delete fragmentShaderSource;
    delete screenFS;
    delete screenFS;
    delete physicsFS;
    delete physicsVS;
    
    delete[] physicsInputData;
    delete[] physicsOutputData;
    
    return 0;
}

unsigned int setupVerticesData() {
    
    
    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    return VAO;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    //
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

unsigned int loadFile(const char* filename, char** buffer) {
    
    std::ifstream t;
    int length;
    t.open(filename);      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    *buffer = new char[length]();    // allocate memory for a buffer of appropriate dimension
    t.read(*buffer, length);       // read the whole file into the buffer
    t.close();                    // close file handle
    return length;
}
void loadShaderSources() {
    
    const char *vsname = "basic.vs";
    const char *fsname = "basic.fs";
    
    loadFile(vsname, &vertexShaderSource);
//    std::cout <<vertexShaderSource<<std::endl;
    loadFile(fsname, &fragmentShaderSource);
    
    loadFile("./shaders/screenShader.vs", &screenVS);
    loadFile("./shaders/screenShader.fs", &screenFS);
}
int createShader(char** vsptr, char** fsptr, const char* vsFile, const char* fsFile) {
    
    
    GLint vsL = loadFile(vsFile, vsptr);
    //    std::cout <<vertexShaderSource<<std::endl;
    GLint fsL = loadFile(fsFile, fsptr);
    
    std::cout << vsFile << std::endl;
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    glShaderSource(vertexShader, 1, vsptr, &vsL);
    
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, fsptr, &fsL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    std::cout << fsFile<< std::endl;
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void createCanvasCopyTexture() {
    
    glGenTextures(1, &canvasCopyTexture);
    glBindTexture(GL_TEXTURE_2D, canvasCopyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void createPhysicsTexture(unsigned int * texptr, float** data) {
    
    glBindTexture(GL_TEXTURE_2D, *texptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, PARTICLE_DATA_WIDTH, PARTICLE_DATA_HEIGHT, 0, GL_RGBA, GL_FLOAT, *data);
    
}
