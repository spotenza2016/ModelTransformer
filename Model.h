#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <map>
using namespace std;

class Model {
    // 3D Vertex (for Vertices and Normals)
    struct Vertex {
        Vertex(float x, float y, float z);
        float x, y, z;
    };

    // 2D Vertex Texture
    struct VertexTexture {
        VertexTexture(float x, float y);
        float x, y;
    };

    // Triangle
    struct Triangle {
        Triangle(int p1, int p2, int p3, glm::vec3 color);
        int p1, p2, p3;
        glm::vec3 color;
    };

    // Stored vertices/triangles of model
    vector<Vertex*> vertices;
    vector<vector<Triangle*>> vertexTriangles;
    vector<glm::vec3> colors;
    vector<Triangle*> triangles;

    // Unused from OBJ File
    vector<Vertex*> vertexNormals;
    vector<VertexTexture*> vertexTextures;

    // Generate ModelViewProjection Matrix
    glm::mat4 generateModelMatrix();
    glm::mat4 generateViewMatrix();
    glm::mat4 generateProjectionMatrix();

    map<string, glm::vec3> readMaterial(string fileName);
    glm::vec3 calculateTriangleNormal(Triangle* triangle);

public:
    // Model Matrix
    glm::vec3 translate = glm::vec3(0, 0, 0);
    float angleX = 0;
    float angleY = 0;
    float angleZ = 0;
    glm::vec3 scale = glm::vec3(0, 0, 0);

    // View Matrix
    glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
    glm::vec3 cameraTarget = glm::vec3(0, 0, 0);
    glm::vec3 upVec = glm::vec3(0, 1, 0);

    // Projection Matrix
    float fov = 45;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;
    float aspectRatio = 1.0;

    glm::vec3 defaultColor = glm::vec3(1, 0, 1);

    // Constructor/Destructor
    Model(string fileName);
    ~Model();

    static char* readShader(string fileName);

    glm::mat4 getMatrix();

    float *generateVBOVerticesArray(bool cpuMatrix, bool colorModifier, bool triangleNormal, bool useNormal);
    pair<float*, unsigned int*> generateEBOVerticesArray(bool cpuMatrix, bool colorModifier, bool useNormal);
    int getNumVertices(bool useEBO);
    int getNumIndices();
    glm::vec3 getNormal(int number, bool triangleNormal);
};
