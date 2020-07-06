#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ccp4reader.h"
#include "shader.h"
#include "camera.h"
#include "ComputeShaderManager.hpp"
#include "openglGUI.h"
#include "marchingCubes.hpp"

// 设置
unsigned int scrWidth = 800;
unsigned int scrHeight = 600;

// 鼠标变量
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = scrWidth / 2.0f;
float lastY = scrHeight / 2.0f;
bool firstMouse = true;
bool pressingMouse = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool isPaused = false;

float valThreshold;

// 共同变量
GLFWwindow* window;
Shader* ourShader;
Shader* lampShader;
Slider *slider;

// 缓冲
unsigned int VAO;
unsigned int VBO;
unsigned int EBO;

// Marching cubes的顶点信息
float *mcVertices;
int mcVertNum;
unsigned int mcVAO;
unsigned int mcVBO;
MarchingCuber mcuber;

// 函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void initOpenGL();
void bufferMCVertices(float *vertices, int vertNum);
void updateMCVertices(float *vertices, int vertNum);
void drawBuffer(unsigned int va, int numVerts);
void draw_data_points(VolumeData *data);
void buffer_figure();
void setModelMatrix(Shader *shader, glm::vec3 position = glm::vec3(0.0f), float angle = 0.0f, bool isMoving = false);
void setViewMatrix(Shader *shader);
void setProjectionMatrix(Shader *shader);
void updateValThreshold(float newThreshold);

int main(int argc, char *argv[]) {

    // 初始化opengl基本东西
    initOpenGL();

    // test的计算着色器
    std::cout << "计算着色器test：";
    ComputeShaderManager csManager;
    csManager.setup();
    int *out;
    csManager.dispatchTest(5, 1, 1, &out);
    for(int i = 0; i < 5; i++) {
        std::cout << out[i] << ",";
    }
    std::cout << "\n";

    // 读密度数据文件（相关代码在ccp4reader.h）
    CCP4Reader reader;
    VolumeData *densityData;
    if(argc > 1) {
        densityData = reader.read(argv[1]);
    } else {
        densityData = reader.read("data/emd_10410_96.map");
    }

    slider = new Slider(densityData->minValue, densityData->maxValue);
    valThreshold = slider->read();

    mcuber.setup(densityData, valThreshold);
    mcVertNum = mcuber.polygonize(&mcVertices);

    // 着色器定义
    ourShader = new Shader("shaders/vertex/shader1.vs", "shaders/fragment/shader1.fs");

    // 缓冲一个立方体的三角形（test而已）
    //buffer_figure();
    bufferMCVertices(mcVertices, mcVertNum);

    // 渲染循环
    while(!glfwWindowShouldClose(window))
    {
        // 时间计算
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        processInput(window);

        // 背景颜色
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 给着色器的变量
        ourShader->use();
        ourShader->setVec3("viewPos", camera.Position);
        setViewMatrix(ourShader);
        setProjectionMatrix(ourShader);

        glBindVertexArray(VAO);

        //draw_data_points(densityData);

        drawBuffer(mcVAO, mcVertNum);

        slider->render();

        glBindVertexArray(0);

        // 检查并调用事情，交换缓冲
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    delete ourShader;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    delete slider;
    delete[] mcVertices;
    delete densityData;

    glfwTerminate();
    return 0;
}

void initOpenGL()
{
    // GLFW初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口对象
    window = glfwCreateWindow(scrWidth, scrHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // GLAD初始化
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    // 视口
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glEnable(GL_DEPTH_TEST);

    // 缓冲
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    Slider::setup();
}

float dataToColor(float min, float max, float data)
{
    return (data - min) / (max - min);
}

void draw_data_points(VolumeData *data)
{
    float step = 5.0f;
    float size = 0.01f;
    
    glm::mat4 modelbase = glm::mat4(1.0f);
    modelbase = glm::scale(modelbase, glm::vec3(size, size, size));

    for(int z = 0; z < data->size[2]; z++) {
        for(int y = 0; y < data->size[1]; y++) {
            for(int x = 0; x < data->size[0]; x++) {
                float curData = data->get(x, y, z);
                float color = dataToColor(data->minValue, data->maxValue, curData);
                if(curData > valThreshold) { // 只绘制出一部分的数据
                    ourShader->setFloat("datacol", color);
                    ourShader->setMat4("model", modelbase);
                    ourShader->setVec3("position", glm::vec3(x, y, z) - glm::vec3(data->size[0], data->size[1], data->size[2]));

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
    }
}

void drawBuffer(unsigned int va, int numVerts)
{
    glBindVertexArray(va);
    ourShader->use();
    ourShader->setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
    ourShader->setFloat("datacol", 0.9f);
    ourShader->setVec3("position", glm::vec3(0, 0, 0));
    glDrawArrays(GL_TRIANGLES, 0, numVerts);
}

void updateMCVertices(float *vertices, int vertNum)
{
    glBindVertexArray(mcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mcVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*vertNum*6, vertices, GL_STATIC_DRAW);
}

void bufferMCVertices(float *vertices, int vertNum)
{
    glGenVertexArrays(1, &mcVAO);

    // 顶点缓冲对象
    glBindVertexArray(mcVAO);
    glGenBuffers(1, &mcVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mcVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*vertNum*6, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3*sizeof(float)));
    glEnableVertexAttribArray(1);
}

void buffer_figure()
{
    float vertices[] = {
        // 位置               // 法向量
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    };

    // 顶点缓冲对象
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3*sizeof(float)));
    glEnableVertexAttribArray(1);
}

// 回调函数
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // 摄像移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isPaused = !isPaused;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        updateValThreshold(slider->move(true, deltaTime));
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        updateValThreshold(slider->move(false, deltaTime));
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    lastX = xpos;
    lastY = ypos;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessMouseMovement(xoffset, yoffset);
    else if(pressingMouse) {
        if(slider->click(lastX, lastY, scrWidth, scrHeight)) {
            updateValThreshold(slider->read());
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            pressingMouse = true;
            if(slider->click(lastX, lastY, scrWidth, scrHeight)) {
                updateValThreshold(slider->read());
            }
        } else if (action == GLFW_RELEASE) {
            pressingMouse = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    scrWidth = width;
    scrHeight = height;
}

void setModelMatrix(Shader *shader, glm::vec3 position, float angle, bool isMoving)
{
    float angleRad = glm::radians(angle);
    if(isMoving) {
        angleRad = (float)glfwGetTime() * glm::radians(50.0f + position.x + position.y);
    }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, angleRad, glm::vec3(0.5f, 1.0f, 0.0f));
    shader->setMat4("model", model);
}

void setViewMatrix(Shader *shader)
{
    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("view", view);
}

void setProjectionMatrix(Shader *shader)
{
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(camera.Zoom, (float) scrWidth / scrHeight , 0.1f, 100.0f);
    shader->setMat4("projection", projection);
}


void updateValThreshold(float newThreshold)
{
    valThreshold = newThreshold;
    mcuber.dataThreshold = valThreshold;
    delete[] mcVertices;
    mcVertNum = mcuber.polygonize(&mcVertices);

    updateMCVertices(mcVertices, mcVertNum);
}