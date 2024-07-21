#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Model.h"
#include <string>
#include <chrono>

// Function Headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool processInput(GLFWwindow* window, Model* model, float translationStep, float angleStep, float fovStep);

// Enums
enum class zBuffer {None = 0, ZMode = 1, ZTildeMode = 2, ZPrimeMode = 3};
enum class shading {None = 0, Flat = 1, Gouraud = 2, Phong = 3};

// Entry Point
int main() {
    // Settings
        // Screen Settings
        int SCR_WIDTH = 800;
        int SCR_HEIGHT = 600;

        // Type of Rendering
        bool useEBO = true;
        bool cpuMatrix = false;
        bool colorModifier = false;
        bool polygonMode = false;
        bool outputPerformanceTime = false;
        bool outputPosition = false;
        zBuffer zBufferRenderMode = zBuffer::None;
        shading shadingMode = shading::Flat;
        float ambientLightIntensity = 0.2f;
        float lightIntensity = 0.8f;
        float phongExponent = 16.0f;
        // Light Vec given in Eye Space Coords
        glm::vec3 lightVec = glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f));
        glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

        // Matrix Settings
        float translationScaleStep = 0.02;
        float rotatationStep = 0.02;
        float fovStep = 0.1;
        glm::vec3 translate = glm::vec3(0, 0, 10);
        float angleX = -45;
        float angleY = 0;
        float angleZ = 0;
        glm::vec3 scale = glm::vec3(0.25, 0.25, 0.25);
        float fov = 45;
        float nearClippingPlane = 0.2f;
        float farClippingPlane = 10.0f;
        float aspectRatio = 4.0/3.0;
        glm::vec3 defaultColor = glm::vec3(1, 0, 1);
        glm::vec4 backgroundColor = glm::vec4(0.54f, 0.81f, 0.94f, 1.0f);
        glm::vec3 cameraTarget = glm::vec3(0, 0, 0);
        glm::vec3 cameraPosition = glm::vec3(0, 0, -1);
        glm::vec3 upVec = glm::vec3(0, 1, 0);

        // Obj File Name
        string objFileName = "sphere.obj";

        // File Name of Shader Files
        string vertexShaderFileName = "source.vs";
        string fragmentShaderFileName = "source.fs";

        // Uniform Constants
        const char* vertexMatrixUniformName = "matrix";
        const char* zBufferRenderModeUniformName = "zBufferRenderMode";
        const char* nearClippingPlaneUniformName = "nearClippingPlane";
        const char* farClippingPlaneUniformName = "farClippingPlane";
        const char* shadingModeUniformName = "shadingMode";
        const char* ambientLightIntensityUniformName = "ambientLightIntensity";
        const char* lightIntensityUniformName = "lightIntensity";
        const char* phongExponentUniformName = "phongExponent";
        const char* lightVecUniformName = "lightVec";
        const char* specularColorUniformName = "specularColor";

    char* vertexShaderSource = nullptr;
    char* fragmentShaderSource = nullptr;
    unsigned int VBO, VAO, EBO, shaderProgram;

    // Handle contradictory settings
    if (zBufferRenderMode != zBuffer::None && shadingMode != shading::None) {
        cout << "Currently rendering Z Buffer, setting shading mode to None." << endl;
        shadingMode = shading::None;
    }
    if (useEBO && shadingMode == shading::Flat) {
        cout << "Cannot do flat shading with EBO Mode On, turning EBO Mode off." << endl;
        useEBO = false;
    }

    // Initialize
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glewInit();

    // Create Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    vertexShaderSource = Model::readShader(vertexShaderFileName);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Create Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentShaderSource = Model::readShader(fragmentShaderFileName);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Enable Depth Drawing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Create Model
    Model model = Model(objFileName);
    model.translate = translate;
    model.angleX = angleX;
    model.angleY = angleY;
    model.angleZ = angleZ;
    model.scale = scale;
    model.fov = fov;
    model.aspectRatio = SCR_WIDTH / SCR_HEIGHT;
    model.nearClippingPlane = nearClippingPlane;
    model.farClippingPlane = farClippingPlane;
    model.aspectRatio = aspectRatio;
    model.defaultColor = defaultColor;
    model.cameraTarget = cameraTarget;
    model.cameraPosition = cameraPosition;
    model.upVec = upVec;

    // Timing
    long frame = 1;
    long average = 0;

    // Calculate Vertices/Indices of Model
    float* vertices = nullptr;
    unsigned int* indices = nullptr;
    if (useEBO) {
        pair<float*, unsigned int*> result = model.generateEBOVerticesArray(cpuMatrix, colorModifier, shadingMode != shading::None);
        vertices = result.first;
        indices = result.second;
    } else {
        vertices = model.generateVBOVerticesArray(cpuMatrix, colorModifier, shadingMode == shading::Flat, shadingMode != shading::None);
    }

    // Create a Uniform Matrix
    unsigned int uniformMatrixID = glGetUniformLocation(shaderProgram, vertexMatrixUniformName);
    glm::mat4 matrixToUse = cpuMatrix ? glm::mat4(1) : model.getMatrix();
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(uniformMatrixID, 1, false, &matrixToUse[0][0]);

    // Add the z buffer mode uniform
    unsigned int uniformZBufferRenderModeID = glGetUniformLocation(shaderProgram, zBufferRenderModeUniformName);
    glUniform1i(uniformZBufferRenderModeID, (int) zBufferRenderMode);

    // Add the near/far clipping plane uniforms
    unsigned int uniformNearClippingPlaneID = glGetUniformLocation(shaderProgram, nearClippingPlaneUniformName);
    glUniform1f(uniformNearClippingPlaneID, nearClippingPlane);
    unsigned int uniformFarClippingPlaneID = glGetUniformLocation(shaderProgram, farClippingPlaneUniformName);
    glUniform1f(uniformFarClippingPlaneID, farClippingPlane);

    // Add the shading mode uniform
    unsigned int uniformShadingModeID = glGetUniformLocation(shaderProgram, shadingModeUniformName);
    glUniform1i(uniformShadingModeID, (int) shadingMode);

    // Add the ambient light intensity uniform
    unsigned int uniformAmbientLightIntensityID = glGetUniformLocation(shaderProgram, ambientLightIntensityUniformName);
    glUniform1f(uniformAmbientLightIntensityID, ambientLightIntensity);

    // Add the light intensity uniform
    unsigned int uniformLightIntensityID = glGetUniformLocation(shaderProgram, lightIntensityUniformName);
    glUniform1f(uniformLightIntensityID, lightIntensity);

    // Add the phong exponent uniform
    unsigned int uniformPhongExponentID = glGetUniformLocation(shaderProgram, phongExponentUniformName);
    glUniform1f(uniformPhongExponentID, phongExponent);

    // Add the light vec uniform
    unsigned int uniformLightVecID = glGetUniformLocation(shaderProgram, lightVecUniformName);
    glUniform3fv(uniformLightVecID, 1, &lightVec[0]);

    // Add the specular color uniform
    unsigned int uniformSpecularColorID = glGetUniformLocation(shaderProgram, specularColorUniformName);
    glUniform3fv(uniformSpecularColorID, 1, &specularColor[0]);

    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Create VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, model.getNumVertices(useEBO) * 11 * sizeof(float), vertices, GL_STATIC_DRAW);

    // Vertices for VBO
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    // Colors for VBO
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (4 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normals for VBO
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (8 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Indices for EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getNumIndices() * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Draw in wireframe polygons
    if (polygonMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // Render Loop
    bool renderFirst = true;
    while (!glfwWindowShouldClose(window)) {
        if (renderFirst || processInput(window, &model, translationScaleStep, rotatationStep, fovStep)) {
            renderFirst = false;

            auto start = chrono::high_resolution_clock::now();
            // Update the Model
            if (cpuMatrix) {
                if (vertices != nullptr) {
                    delete[] vertices;
                }

                if (indices != nullptr) {
                    delete[] indices;
                }

                // Remake the vertices/indices
                if (useEBO) {
                    pair<float*, unsigned int*> result = model.generateEBOVerticesArray(cpuMatrix, colorModifier, shadingMode != shading::None);
                    vertices = result.first;
                    indices = result.second;
                } else {
                    vertices = model.generateVBOVerticesArray(cpuMatrix, colorModifier, shadingMode == shading::Flat, shadingMode != shading::None);
                }

                // Create VBO
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, model.getNumVertices(useEBO) * 11 * sizeof(float), vertices, GL_STATIC_DRAW);

                // Vertices for VBO
                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);

                // Colors for VBO
                glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (4 * sizeof(float)));
                glEnableVertexAttribArray(1);

                // Normals for VBO
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (8 * sizeof(float)));
                glEnableVertexAttribArray(2);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                // Indices for EBO
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getNumIndices() * sizeof(unsigned int), indices, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                glBindVertexArray(0);
            }
            // Update the Uniform Matrix
            else {
                glUseProgram(shaderProgram);
                glUniformMatrix4fv(uniformMatrixID, 1, false, &model.getMatrix()[0][0]);
            }

            // Background
            glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw Triangles
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            if (useEBO) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glDrawElements(GL_TRIANGLES, model.getNumIndices(), GL_UNSIGNED_INT, 0);
            }
            else {
                glDrawArrays(GL_TRIANGLES, 0, model.getNumVertices(useEBO));
            }
            glBindVertexArray(0);

            glfwSwapBuffers(window);

            auto finish = chrono::high_resolution_clock::now();

            if (outputPerformanceTime) {
                long duration = chrono::duration_cast<chrono::microseconds>(finish - start).count();
                cout << "Frame " << frame << ": " << duration << " microseconds." << endl;
                average = (average * (frame - 1) + duration) / frame;
                cout << "Average Time: " << average << " microseconds." << endl;
            }

            if (outputPosition) {
                cout << "Frame " << frame << ": " << endl;
                cout << "Translation: " << endl;
                cout << model.translate.x << " " << model.translate.y << " " << model.translate.z << endl;
                cout << "Rotation: " << endl;
                cout << model.angleX << " " << model.angleY << " " << model.angleZ << endl;
                cout << "Scale: " << endl;
                cout << model.scale.x << " " << model.scale.y << " " << model.scale.z << endl;
                cout << "FOV: " << endl;
                cout << model.fov << endl;
            }

            frame++;
        }

        glfwPollEvents();
    }

    // Clean Up
    if (vertexShaderSource != nullptr) {
        delete[] vertexShaderSource;
    }
    if (fragmentShaderSource != nullptr) {
        delete[] fragmentShaderSource;
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    delete[] vertices;
    if (indices != nullptr) {
        delete[] indices;
    }

    return 0;
}

// Callback for OpenGL
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Process Input
bool processInput(GLFWwindow* window, Model* model, float translationStep, float angleStep, float fovStep) {
    // Exit Window on Escape
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Return True if Any Movement Occurred
    bool move = false;

    // Move Model with Arrow Keys and Page Up/Down
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        model->translate.y += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        model->translate.y -= translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        model->translate.x += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        model->translate.x -= translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        model->translate.z += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        model->translate.z -= translationStep;
        move = true;
    }

    // Rotate Model around X Axis with R and T
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        model->angleX -= angleStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        model->angleX += angleStep;
        move = true;
    }

    // Rotate Model around Y Axis with Y and U
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        model->angleY -= angleStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        model->angleY += angleStep;
        move = true;
    }

    // Rotate Model around Z Axis with I and O
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        model->angleZ -= angleStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        model->angleZ += angleStep;
        move = true;
    }

    // Adjust Scale along X Axis with F and G
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        model->scale.x += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        model->scale.x -= translationStep;
        move = true;
    }

    // Adjust Scale along Y Axis with H and J
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        model->scale.y += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        model->scale.y -= translationStep;
        move = true;
    }

    // Adjust Scale along Z Axis with K and L
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        model->scale.z += translationStep;
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        model->scale.z -= translationStep;
        move = true;
    }

    // Adjust FOV with Z and X
    if (glfwGetKey(window, GLFW_KEY_Z)) {
        model->fov = max(0.0f, model->fov - fovStep);
        move = true;
    }
    if (glfwGetKey(window, GLFW_KEY_X)) {
        model->fov = min(180.0f, model->fov + fovStep);
        move = true;
    }

    return move;
}