// Microwave3D.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 // Circle Resolution = Rezolucija kruga
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

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
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/intersect.hpp>

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
    { -0.2f, 0.0f, 0.08f, 0.15f, 1 }, // Button 1
    { -0.28f, 0.0f, 0.08f, 0.15f, 2 },
    { -0.36f, 0.0f, 0.08f, 0.15f, 3 },
    { -0.2f,  -0.15f, 0.08f, 0.15f, 4 },
    { -0.28f, -0.15f, 0.08f, 0.15f, 5 },
    { -0.36f, -0.15f, 0.08f, 0.15f, 6 },
    { -0.2f,  -0.3f, 0.08f, 0.15f, 7 },
    { -0.28f, -0.3f, 0.08f, 0.15f, 8 },
    { -0.36f, -0.3f, 0.08f, 0.15f, 9 },
    { -0.2f,  -0.45f, 0.08f, 0.15f, 11 },
    { -0.28f, -0.45f, 0.08f, 0.15f, 0 },
    { -0.36f, -0.45f, 0.08f, 0.15f, 12 },
    { -0.28f, -0.7, 0.08f, 0.15f, 13}
};


struct DigitalNumber {
    float x, y, width, height;
};

DigitalNumber digitalNumbers[4] = {
    { -0.18f, 0.22f, 0.05f, 0.16f },
    { -0.23f, 0.22f, 0.05f, 0.16f },
    { -0.31f, 0.22f, 0.05f, 0.16f },
    { -0.36f, 0.22f, 0.05f, 0.16f }
};

int clockTimerNumbers[4] =
{
    0,0,0,0
};

bool isMousePressed = false;
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

bool isViewPerspective = false;
bool isCameraLocked = false;
bool isOpening = false;
bool isClosing = false;
float doorAngle = 0.0f; // Current angle of the door
float maxDoorAngle = glm::radians(90.0f); // Maximum rotation (90 degrees)
float doorSpeed = glm::radians(45.0f); // Rotation speed in radians per second

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isCameraLocked) {
        return;
    }
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

void renderButtons(unsigned int buttonShader, unsigned int buttonVAO, unsigned int* buttonTextures, glm::mat4 model) {
    glUseProgram(buttonShader);
    glBindVertexArray(buttonVAO);
    glUniformMatrix4fv(glGetUniformLocation(buttonShader, "uM"), 1, GL_FALSE, glm::value_ptr(model));

    for (int i = 0; i < 13; ++i) {
        // Bind the texture for the current button
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buttonTextures[i]);
        if (i == 11) {
            if (isPowerOn) {
                glBindTexture(GL_TEXTURE_2D, buttonTextures[i + 2]);
            }
        }
        else if (i == 12) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform1i(glGetUniformLocation(buttonShader, "uTex"), 0);

            // Draw the button
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
            glDisable(GL_BLEND);
            break;
        }
        glUniform1i(glGetUniformLocation(buttonShader, "uTex"), 0);

        // Draw the button
        glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderDigitalNumbers(unsigned int buttonShader, unsigned int digitalNumberVAO, unsigned int* digitalNumberTextures, glm::mat4 model) {
    glUseProgram(buttonShader);
    glBindVertexArray(digitalNumberVAO);

    glUniformMatrix4fv(glGetUniformLocation(buttonShader, "uM"), 1, GL_FALSE, glm::value_ptr(model));

    for (int i = 0; i < 4; ++i) {
        // Bind the texture for the current button
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, digitalNumberTextures[clockTimerNumbers[i]]);
        glUniform1i(glGetUniformLocation(buttonShader, "uTex"), 0); // Set the uniform to use texture unit 0

        // Draw the button
        glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    glBindVertexArray(0);
    glUseProgram(0);
}

void countdown() {
    while (isPowerOn) {
        // Simulate a 1-second delay
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!isPowerOn) {
            break;
        }
        // Perform the countdown
        for (int i = 3; i >= 0; --i) {
            if (clockTimerNumbers[i] > 0) {
                --clockTimerNumbers[i];
                break;
            }
            else if (i > 0) {
                if (i == 2) {
                    clockTimerNumbers[i] = 5; // Set to 5 when the third digit reaches 0
                }
                else {
                    clockTimerNumbers[i] = 9; // Default reset to 9
                }
            }
        }


        // Check if the timer is all zeros
        if (clockTimerNumbers[0] == 0 && clockTimerNumbers[1] == 0 &&
            clockTimerNumbers[2] == 0 && clockTimerNumbers[3] == 0) {
            isMicrowaveFinished = true;
            isPowerOn = false; // Turn off the timer
            lastTime = glfwGetTime();


            std::thread([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                isMicrowaveFinished = false;
                }).detach();
        }
    }
}

// Helper function to generate ray
glm::vec3 calculateMouseRay(float mouseX, float mouseY, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int screenWidth, int screenHeight, glm::vec3& rayOrigin) {
    // Convert mouse position to NDC
    float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / screenHeight; // Invert Y-axis
    glm::vec4 clipCoords = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);

    if (isViewPerspective) {
        // Perspective projection: Ray origin is at the camera position
        rayOrigin = cameraPos;

        // Unproject clip coordinates to view space
        glm::vec4 eyeCoords = glm::inverse(projectionMatrix) * clipCoords;
        eyeCoords.z = -1.0f; // Set depth
        eyeCoords.w = 0.0f; // Directional vector

        // Unproject to world space
        glm::vec3 worldRay = glm::vec3(glm::inverse(viewMatrix) * eyeCoords);
        return glm::normalize(worldRay);
    }
    else {
        // Orthogonal projection: Ray origin is at the near plane point
        glm::vec4 eyeCoords = glm::inverse(projectionMatrix) * clipCoords;
        eyeCoords.z = -1.0f; // Near plane depth
        eyeCoords.w = 1.0f; // Position vector

        rayOrigin = glm::vec3(glm::inverse(viewMatrix) * eyeCoords);

        // Direction is constant in orthogonal projection
        glm::vec3 rayDirection = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        return rayDirection;
    }
}


void handleButtonPress(Button button) {
    std::cout << "Button " << button.value << " pressed!" << std::endl;
    if (button.value == 12) {
        if (clockTimerNumbers[0] == 0 && clockTimerNumbers[1] == 0 &&
            clockTimerNumbers[2] == 0 && clockTimerNumbers[3] == 0) {
            return;
        }
        if (isPowerOn || isOpened) { return; }
        // Prevent starting if digit at index 2 is greater than 5
        if (!isPowerOn && clockTimerNumbers[2] > 5) {
            std::cout << "Cannot start: digit at index 2 is greater than 5!" << std::endl;
            return;
        }

        // Toggle power state and start countdown if valid
        if (!isPowerOn) {
            isPowerOn = true;
            lastAngle = angle;          // Save the current angle
            lastTime = glfwGetTime();   // Reset lastTime to avoid a large deltaTime
            std::thread countdownThread(countdown);
            countdownThread.detach();
        }

    }
    else if (button.value == 11) {
        isPowerOn = false;
        lastTime = glfwGetTime();
    }
    else if (button.value == 13) {
        isPowerOn = false;
        lastTime = glfwGetTime();
        clockTimerNumbers[0] = 0;
        clockTimerNumbers[1] = 0;
        clockTimerNumbers[2] = 0;
        clockTimerNumbers[3] = 0;
    }
    else {
        // Shift digits to the left and add the pressed button value
        for (int j = 0; j < 3; ++j) {
            clockTimerNumbers[j] = clockTimerNumbers[j + 1];
        }
        clockTimerNumbers[3] = button.value;
    }
    return;
}

// Function to test ray against buttons
void checkButtonPressWithRay(float mouseX, float mouseY, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int screenWidth, int screenHeight) {
    glm::vec3 rayOrigin = cameraPos; // Camera position in world space
    glm::vec3 rayDirection = calculateMouseRay(mouseX, mouseY, viewMatrix, projectionMatrix, screenWidth, screenHeight, rayOrigin);

    // The fixed depth of the button plane
    float buttonPlaneZ = -0.502f;
    
    for (int i = 0; i < 13; ++i) {
        Button button = buttons[i];

        if (isPowerOn && buttons[i].value != 13 && buttons[i].value != 12 && buttons[i].value != 11) {
            continue;
        }
        // Find where the ray intersects the button plane (z = -0.502)
        if (glm::abs(rayDirection.z) > 1e-6f) { // Avoid divide-by-zero for rays parallel to the plane
            float t = (buttonPlaneZ - rayOrigin.z) / rayDirection.z; // Intersection distance along the ray

            if (t >= 0) { // Only consider intersections in front of the ray origin
                glm::vec3 intersectionPoint = rayOrigin + t * rayDirection;

                // Check if the intersection point lies within the button's 2D boundaries
                if (intersectionPoint.x >= button.x &&
                    intersectionPoint.x <= button.x + button.width &&
                    intersectionPoint.y >= button.y &&
                    intersectionPoint.y <= button.y + button.height) {
                        handleButtonPress(buttons[i]);
                }
            }
        }
    }
}

void renderTimer(unsigned int timerShader, unsigned int timerVAO, glm::mat4 model) {
    glUseProgram(timerShader);
    glBindVertexArray(timerVAO);
    glUniformMatrix4fv(glGetUniformLocation(timerShader, "uM"), 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Black box

    glPointSize(5);
    glDrawArrays(GL_POINTS, 4, 2);

    glUseProgram(0);
    glBindVertexArray(0);
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
    unsigned int buttonShader = createShader("button.vert", "button.frag");
    unsigned int timerShader = createShader("timer.vert", "timer.frag");


    unsigned int shaders[] = { unifiedShader, doorShader, buttonShader, timerShader };
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
         0.0f, -0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
         0.9, -0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
         0.9,  0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
         0.9,  0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
        0.0f,  0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
        0.0f, -0.4f,  -0.502f,    0.0f, 0.0f, -1.0f,
        
    };
    glm::vec3 hingePosition(0.9f, 0.0f, -0.502f);

    float microwaveTimerVertices[]{
        // Black box
        -0.12, 0.20, -0.501, 0, 0, 0,
        -0.12, 0.40, -0.501, 0, 0, 0,
        -0.37, 0.20, -0.501, 0, 0, 0,
        -0.37, 0.40, -0.501, 0, 0, 0,

        // Dots on timer
        -0.245, 0.27, -0.503, 1, 0, 0,
        -0.245, 0.32, -0.503, 1, 0, 0,
    };

    float buttonVertices[13 * 16]; // 13 buttons, 16 values (4 * 4 vertices per button)

    // Loop to define the button positions
    for (int i = 0; i < 13; ++i) {
        Button& btn = buttons[i];
        float x1 = btn.x;
        float y1 = btn.y;
        float x2 = btn.x + btn.width;
        float y2 = btn.y + btn.height;

        // Assign the button's vertex data (ordered correctly for a square outline)
        buttonVertices[i * 16 + 0] = x1; buttonVertices[i * 16 + 1] = y1; buttonVertices[i * 16 + 2] = 1; buttonVertices[i * 16 + 3] = 0; // Bottom-left
        buttonVertices[i * 16 + 4] = x1; buttonVertices[i * 16 + 5] = y2; buttonVertices[i * 16 + 6] = 1; buttonVertices[i * 16 + 7] = 1; // Top-left
        buttonVertices[i * 16 + 8] = x2; buttonVertices[i * 16 + 9] = y1; buttonVertices[i * 16 + 10] = 0; buttonVertices[i * 16 + 11] = 0; // Bottom-right
        buttonVertices[i * 16 + 12] = x2; buttonVertices[i * 16 + 13] = y2; buttonVertices[i * 16 + 14] = 0; buttonVertices[i * 16 + 15] = 1; // Top-right
    }

    unsigned int buttonTextures[14]; // Array to store texture IDs for each button

    for (int i = 0; i < 14; ++i) {
        std::string texturePath = "res/button_texture_" + std::to_string(i + 1) + ".png";
        if (i == 9) {
            texturePath = "res/pause.png";
        }
        else if (i == 11) {
            texturePath = "res/unactive_start.png";
        }
        else if (i == 13) {
            texturePath = "res/active_start.png";
        }
        else if (i == 12) {
            texturePath = "res/restart.png";
        }
        buttonTextures[i] = loadImageToTexture(texturePath.c_str()); // Function to load the texture
        glBindTexture(GL_TEXTURE_2D, buttonTextures[i]); // Bind the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }

    float digitalNumVertices[4 * 16]; // 4 nums, 4 values for 1 point, 4 points total = 16

    // Loop to define the button positions
    for (int i = 0; i < 4; ++i) {
        DigitalNumber& num = digitalNumbers[i];
        float x1 = num.x;
        float y1 = num.y;
        float x2 = num.x + num.width;
        float y2 = num.y + num.height;

        // Assign the button's vertex data (ordered correctly for a square outline)
        digitalNumVertices[i * 16 + 0] = x1; digitalNumVertices[i * 16 + 1] = y1; digitalNumVertices[i * 16 + 2] = 1; digitalNumVertices[i * 16 + 3] = 0; // Bottom-left
        digitalNumVertices[i * 16 + 4] = x1; digitalNumVertices[i * 16 + 5] = y2; digitalNumVertices[i * 16 + 6] = 1; digitalNumVertices[i * 16 + 7] = 1; // Top-left
        digitalNumVertices[i * 16 + 8] = x2; digitalNumVertices[i * 16 + 9] = y1; digitalNumVertices[i * 16 + 10] = 0; digitalNumVertices[i * 16 + 11] = 0; // Bottom-right
        digitalNumVertices[i * 16 + 12] = x2; digitalNumVertices[i * 16 + 13] = y2; digitalNumVertices[i * 16 + 14] = 0; digitalNumVertices[i * 16 + 15] = 1; // Top-right
    }

    unsigned int digitalNumberTextures[10];
    for (int i = 0; i < 10; ++i) {
        std::string texturePath = "res/digital_number_" + std::to_string(i) + ".jpg";
        digitalNumberTextures[i] = loadImageToTexture(texturePath.c_str()); // Function to load the texture
        glBindTexture(GL_TEXTURE_2D, digitalNumberTextures[i]); // Bind the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }

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

    unsigned int buttonVAO, buttonVBO;
    glGenVertexArrays(1, &buttonVAO);
    glGenBuffers(1, &buttonVBO);

    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int digitalNumberVAO, digitalNumberVBO;
    glGenVertexArrays(1, &digitalNumberVAO);
    glGenBuffers(1, &digitalNumberVBO);

    glBindVertexArray(digitalNumberVAO);
    glBindBuffer(GL_ARRAY_BUFFER, digitalNumberVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(digitalNumVertices), digitalNumVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int timerVAO, timerVBO;
    glGenVertexArrays(1, &timerVAO);
    glGenBuffers(1, &timerVBO);

    glBindVertexArray(timerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, timerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(microwaveTimerVertices), microwaveTimerVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    glUseProgram(0);
    changeProjection(shaders, shaderCount, projectionO);


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
            isViewPerspective = true;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            changeProjection(shaders, shaderCount, projectionO);
            isViewPerspective = false;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            isTrasparentWindow = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        {
            isTrasparentWindow = false;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { 
            isOpening = true;
            isClosing = false;
        }   

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            isClosing = true;
            isOpening = false;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            isCameraLocked = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
            isCameraLocked = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);

            // Update lastX and lastY to prevent twitching
            lastX = width / 2.0;
            lastY = height / 2.0;
        }
       

        //Transformisanje trouglova
        float cameraSpeed = 2.5f * deltaTime;
        if (!isCameraLocked) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }
        
        glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        model = glm::mat4(1.0f);
        updateView(shaders, shaderCount, cameraPos, view);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!isMousePressed && isCameraLocked) {
                // Mouse button is newly pressed
                isMousePressed = true;

                int screenWidth, screenHeight;
                glfwGetWindowSize(window, &screenWidth, &screenHeight);

                if (isViewPerspective) {
                    checkButtonPressWithRay(mouseX, mouseY, view, projectionP, screenWidth, screenHeight);
                }
                else {
                    checkButtonPressWithRay(mouseX, mouseY, view, projectionO, screenWidth, screenHeight);
                }
            }
        }
        else {
            // Reset the press state when the button is released
            isMousePressed = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Osvjezavamo i Z bafer i bafer boje

        drawCube(unifiedShader,VAO, model);

        if (isOpening && doorAngle > -maxDoorAngle) {
            doorAngle -= doorSpeed * deltaTime; // Decrement angle to open to the left
            if (doorAngle <= -maxDoorAngle) {
                doorAngle = -maxDoorAngle; // Clamp to -90 degrees
                isOpening = false;        // Stop the opening animation
            }
        }
        if (isClosing && doorAngle < 0.0f) {
            doorAngle += doorSpeed * deltaTime; // Increment angle to close the door
            if (doorAngle >= 0.0f) {
                doorAngle = 0.0f;     // Clamp to 0 degrees (fully closed)
                isClosing = false;    // Stop the closing animation
            }
        }
        glm::mat4 doorModel = glm::mat4(1.0f);
        // Move the hinge to the origin
        doorModel = glm::translate(doorModel, hingePosition);
        // Apply rotation around the Y-axis
        doorModel = glm::rotate(doorModel, doorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        // Move the hinge back to its original position
        doorModel = glm::translate(doorModel, -hingePosition);

        if (isTrasparentWindow) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawDoor(doorShader, doorVAO, doorModel);
            glDisable(GL_BLEND);
        }
        else {
            glDisable(GL_BLEND);
            drawDoor(doorShader, doorVAO, doorModel);
        }

        renderButtons(buttonShader, buttonVAO, buttonTextures, model);
        
        renderDigitalNumbers(buttonShader, digitalNumberVAO, digitalNumberTextures, model);

        renderTimer(timerShader, timerVAO, model);

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
