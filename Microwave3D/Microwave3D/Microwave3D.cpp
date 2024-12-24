// Microwave3D.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 // Circle Resolution = Rezolucija kruga
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>

#include <GL/glew.h>   
#include <GLFW/glfw3.h>
#include "ShaderUtils.h" 
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

static unsigned loadImageToTexture(const char* filePath);

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}


struct Button {
    float x, y, width, height;
    int value;
};

Button buttons[13] = {
    { 0.43f, 0.0f, 0.08f, 0.15f, 1 }, // Button 1
    { 0.51f, 0.0f, 0.08f, 0.15f, 2 },
    { 0.59f, 0.0f, 0.08f, 0.15f, 3 },
    { 0.43f, -0.15f, 0.08f, 0.15f, 4 },
    { 0.51f, -0.15f, 0.08f, 0.15f, 5 },
    { 0.59f, -0.15f, 0.08f, 0.15f, 6 },
    { 0.43f, -0.3f, 0.08f, 0.15f, 7 },
    { 0.51f, -0.3f, 0.08f, 0.15f, 8 },
    { 0.59f, -0.3f, 0.08f, 0.15f, 9 },
    { 0.43f, -0.45f, 0.08f, 0.15f, 11 },
    { 0.51f, -0.45f, 0.08f, 0.15f, 0 },
    { 0.59f, -0.45f, 0.08f, 0.15f, 12 },
    { 0.51f, -0.7, 0.08f, 0.15f, 13}
};


struct DigitalNumber {
    float x, y, width, height;
};

DigitalNumber digitalNumbers[4] = {
    { 0.44f, 0.22f, 0.05f, 0.16f },
    { 0.49f, 0.22f, 0.05f, 0.16f },
    { 0.56f, 0.22f, 0.05f, 0.16f },
    { 0.61f, 0.22f, 0.05f, 0.16f }
};

int clockTimerNumbers[4] =
{
    0,0,0,0
};
bool isPowerOn = false;
bool isOpened = false;
bool isTrasparentWindow = true;
bool isError = false;
bool isXPressed = false;
bool isMicrowaveFinished = false;
float angle = 0.0f;
float darkness = 1.0f;
float lastAngle = 0.0f;
float lastTime = 0.0f;
float rotationSpeed = 15.0f;
float yaw = 90.0f;
float pitch = 0;
float lastX, lastY; // No initialization here
bool firstMouse = true;
glm::vec3 cameraPos = glm::vec3(0.2f, 0.0f, -2.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightPos(3.0f, 1.0f, -2.0f);
float fov = 90.0f;

float microwaveRGB = 130 / 255.0;
float microwaveGlassR = 173 / 255.0;
float microwaveGlassG = 216 / 255.0;
float microwaveGlassB = 230 / 255.0;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

void drawCube(unsigned int shader, unsigned int VAO, glm::mat4 model) {
    glUseProgram(shader); //Slanje default vrijednosti uniformi
    glBindVertexArray(VAO);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uM"), 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_TRIANGLES, 0, 54);
}

void drawDoor(unsigned int shader, unsigned int VAO, glm::mat4 model) {
    glUseProgram(shader); //Slanje default vrijednosti uniformi
    glBindVertexArray(VAO);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uM"), 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void changeProjection(unsigned int shaders[], size_t shaderCount, glm::mat4 projection) {
    for (size_t i = 0; i < shaderCount; ++i) { // Loop through the array by index
        unsigned int shader = shaders[i]; // Access each shader using the index
        glUseProgram(shader); //Slanje default vrijednosti uniformi
        glUniformMatrix4fv(glGetUniformLocation(shader, "uP"), 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void updateView(unsigned int shaders[], size_t shaderCount, glm::vec3 cameraPos, glm::mat4 view) {
    for (size_t i = 0; i < shaderCount; ++i) { // Loop through the array by index
        unsigned int shader = shaders[i]; // Access each shader using the index
        glUseProgram(shader); //Slanje default vrijednosti uniformi
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cameraPos));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uV"), 1, GL_FALSE, glm::value_ptr(view));

    }
}


int main(void)
{


    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    const char wTitle[] = "Microwave";
    window = glfwCreateWindow(mode->width, mode->height, wTitle, monitor, NULL);

    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    lastX = xpos;
    lastY = ypos;

    void mouse_callback(GLFWwindow * window, double xpos, double ypos);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetScrollCallback(window, scroll_callback);



    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int doorShader = createShader("door.vert", "door.frag");
    unsigned int shaders[] = { unifiedShader, doorShader };
    // Get the shader count (in this case, 2 shaders)
    size_t shaderCount = sizeof(shaders) / sizeof(shaders[0]);
    float microwaveVertices[] = {
        // positions         // normal coords
       // Front face (with a hole)
        // RIGHT of the hole
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,
        -0.0f,  -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.0f,  -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.0f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        // Top-right of the hole
         1.0,  0.4f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

         -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         -0.5f,  0.4f, -0.5f, 0.0f,  0.0f, -1.0f,
         1.0,  0.4f, -0.5f,  0.0f,  0.0f, -1.0f,

         // Bottom of the hole
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0, -0.4f, -0.5f,  0.0f,  0.0f, -1.0f,

         1.0, -0.4f, -0.5f,  0.0f,  0.0f, -1.0f,
         -0.5f, -0.4f, -0.5f,  0.0f,  0.0f, -1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

         // LEFT of the hole
          1.0, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          1.0, 0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
          0.9f, 0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

          0.9f, 0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
          0.9f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          1.0, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, -1.0f,
         1.0, -0.5f,  0.5f,    0.0f, 0.0f, -1.0f,
         1.0,  0.5f,  0.5f,    0.0f, 0.0f, -1.0f,
         1.0,  0.5f,  0.5f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, -1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

         1.0,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         1.0,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         1.0, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         1.0, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         1.0, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         1.0,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,
         1.0, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,
         1.0, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,
         1.0, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         1.0,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         1.0,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         1.0,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    };

    float microwaveDoorVertices[] = {
         0.0f, -0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
         0.9, -0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
         0.9,  0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
         0.9,  0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
        0.0f,  0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
        0.0f, -0.4f,  -0.501f,    0.0f, 0.0f, -1.0f,
        
    };

    unsigned int stride = 6 * sizeof(float);

    unsigned int VAO, VBO, doorVAO, doorVBO;

    // Generate and bind the Vertex Array Object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind the Vertex Buffer Object
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(microwaveVertices), microwaveVertices, GL_STATIC_DRAW);

    // Define the vertex attribute for position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO (optional)
    glBindVertexArray(0);

    glGenVertexArrays(1, &doorVAO);
    glBindVertexArray(doorVAO);

    // Generate and bind the Vertex Buffer Object
    glGenBuffers(1, &doorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(microwaveDoorVertices), microwaveDoorVertices, GL_STATIC_DRAW);

    // Define the vertex attribute for position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO (optional)
    glBindVertexArray(0);


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++
    glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, glm::value_ptr(model));


    glm::mat4 view; //Matrica pogleda (kamere)

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));

    float aspectRatio = (float)mode->width / (float)mode->height;  // Aspect ratio
    glm::mat4 projectionP = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    glm::mat4 projectionO = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)
    glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uP"), 1, GL_FALSE, glm::value_ptr(projectionO));
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    unsigned int lightColor = glGetUniformLocation(unifiedShader, "lightColor");
    unsigned int viewPosLoc = glGetUniformLocation(unifiedShader, "viewPos");

    glUniform3f(glGetUniformLocation(unifiedShader, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    glUniform3f(glGetUniformLocation(unifiedShader, "material.ambient"), microwaveRGB, microwaveRGB, microwaveRGB);
    glUniform3f(glGetUniformLocation(unifiedShader, "material.diffuse"), microwaveRGB, microwaveRGB, microwaveRGB);
    glUniform3f(glGetUniformLocation(unifiedShader, "material.specular"), 0.5f, 0.5f, 0.5f);
    glUniform1f(glGetUniformLocation(unifiedShader, "material.shininess"), 32.0f);
    
    glUniform3fv(glGetUniformLocation(unifiedShader, "light.position"), 1, glm::value_ptr(lightPos));
    glUniform3f(glGetUniformLocation(unifiedShader, "light.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(unifiedShader, "light.diffuse"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(unifiedShader, "light.specular"), 1.0f, 1.0f, 1.0f);

    glUseProgram(doorShader);
    glUniform3f(glGetUniformLocation(doorShader, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(doorShader, "viewPos"), 1, glm::value_ptr(cameraPos));

    glUniform3f(glGetUniformLocation(doorShader, "material.ambient"), microwaveGlassR, microwaveGlassG, microwaveGlassB);
    glUniform3f(glGetUniformLocation(doorShader, "material.diffuse"), microwaveGlassR, microwaveGlassG, microwaveGlassB);
    glUniform3f(glGetUniformLocation(doorShader, "material.specular"), 0.5f, 0.5f, 0.5f);
    glUniform1f(glGetUniformLocation(doorShader, "material.shininess"), 32.0f);

    glUniform3fv(glGetUniformLocation(doorShader, "light.position"), 1, glm::value_ptr(lightPos));
    glUniform3f(glGetUniformLocation(doorShader, "light.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(doorShader, "light.diffuse"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(doorShader, "light.specular"), 1.0f, 1.0f, 1.0f);
    glUseProgram(doorShader);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)
                glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame
    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        //Testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }

        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }

        //Mijenjanje projekcija
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            changeProjection(shaders, shaderCount, projectionP);
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            changeProjection(shaders, shaderCount, projectionO);
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            isTrasparentWindow = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        {
            isTrasparentWindow = false;
        }
        //Transformisanje trouglova
        float cameraSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        model = glm::mat4(1.0f);
        updateView(shaders, shaderCount, cameraPos, view);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Osvjezavamo i Z bafer i bafer boje

        drawCube(unifiedShader,VAO, model);
        if (isTrasparentWindow) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawDoor(doorShader, doorVAO, model);
            glDisable(GL_BLEND);
        }
        else {
            glDisable(GL_BLEND);
            drawDoor(doorShader, doorVAO, model);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}
