#include "Model.h"

// Open an object file as a model
Model::Model(string fileName) {
    map<string, glm::vec3> material;
    string currMaterial = "";

    // Open the file
    ifstream file(fileName);
    if (!file.is_open()) {
        cout << "File: \'" + fileName + "\' failed to open." << endl;
        return;
    }

    string currLine;

    // Read all lines
    while (getline(file, currLine)) {
        stringstream currLineStream(currLine);
        vector<string> parts;
        string currLinePart;

        // Read each part of the current line
        while (getline(currLineStream, currLinePart, ' ')) {
            parts.push_back(currLinePart);
        }

        // Use first part to decide what to do
        if (parts.size() == 0) {
            continue;
        }
        // Vertex
        else if (parts.at(0) == "v") {
            if (parts.size() < 4) {
                continue;
            }

            glm::vec3 colorVec = material.count(currMaterial) != 0 ? material.at(currMaterial) : defaultColor;
            colors.push_back(colorVec);
            vertexTriangles.push_back({});
            vertices.push_back(new Vertex(stof(parts.at(1)), stof(parts.at(2)), stof(parts.at(3))));
        }
        // Texture Vertex
        else if (parts.at(0) == "vt") {
            if (parts.size() < 3) {
                continue;
            }

            vertexTextures.push_back(new VertexTexture(stof(parts.at(1)), stof(parts.at(2))));
        }
        // Normal Vertex
        else if (parts.at(0) == "vn") {
            if (parts.size() < 4) {
                continue;
            }

            vertexNormals.push_back(new Vertex(stof(parts.at(1)), stof(parts.at(2)), stof(parts.at(3))));
        }
        // Face
        else if (parts.at(0) == "f") {
            vector<int> faceVertices;
            vector<int> faceTextures;
            vector<int> faceNormals;

            // For each part
            for (int i = 1; i < parts.size(); i++) {
                string currVertex;
                stringstream currVertexStream(parts.at(i));
                vector<string> vertexParts;

                // Read each vertex
                while (getline(currVertexStream, currVertex, '/')) {
                    vertexParts.push_back(currVertex);
                }

                // Place into proper vector
                if (vertexParts.size() > 0 && vertexParts.at(0) != "") {
                    faceVertices.push_back(stoi(vertexParts.at(0)));
                }
                if (vertexParts.size() > 1 && vertexParts.at(1) != "") {
                    faceTextures.push_back(stoi(vertexParts.at(1)));
                }
                if (vertexParts.size() > 2 && vertexParts.at(2) != "") {
                    faceNormals.push_back(stoi(vertexParts.at(2)));
                }
            }

            // Subdivide faces into triangles
            if (faceVertices.size() >= 3) {
                int numTriangles = faceVertices.size()  - 2;
                for (int i = 0; i < numTriangles; i++) {
                    int currVertex = i + 2;
                    glm::vec3 colorVec = material.count(currMaterial) != 0 ? material.at(currMaterial) : defaultColor;

                    // Resize color vector if too small
                    if (colors.size() <= faceVertices.at(0)) {
                        colors.resize(faceVertices.at(0) - 1 + 1);
                    }
                    if (colors.size() <= faceVertices.at(currVertex - 1)) {
                        colors.resize(faceVertices.at(currVertex - 1) - 1 + 1);
                    }
                    if (colors.size() <= faceVertices.at(currVertex)) {
                        colors.resize(faceVertices.at(currVertex) - 1 + 1);
                    }

                    // Set Vertex Colors
                    colors.at(faceVertices.at(0) - 1) = colorVec;
                    colors.at(faceVertices.at(currVertex - 1) - 1) = colorVec;
                    colors.at(faceVertices.at(currVertex) - 1) = colorVec;

                    Triangle* triPtr = new Triangle(faceVertices.at(0), faceVertices.at(currVertex - 1), faceVertices.at(currVertex), colorVec);

                    vertexTriangles.at(faceVertices.at(0) - 1).push_back(triPtr);
                    vertexTriangles.at(faceVertices.at(currVertex - 1) - 1).push_back(triPtr);
                    vertexTriangles.at(faceVertices.at(currVertex) - 1).push_back(triPtr);

                    triangles.push_back(triPtr);
                }
            }
        }
        // Read a material library
        else if (parts.at(0) == "mtllib") {
            if (parts.size() < 2) {
                continue;
            }

            material = readMaterial(parts.at(1));
        }
        // Change current material
        else if (parts.at(0) == "usemtl") {
            if (parts.size() < 2) {
                continue;
            }

            currMaterial = parts.at(1);
        }
    }

    file.close();
}

// Destructor
Model::~Model() {
    // Delete all vertices/triangles

    for (int i = 0; i < vertices.size(); i++) {
        delete vertices.at(i);
    }

    for (int i = 0; i < vertexNormals.size(); i++) {
        delete vertexNormals.at(i);
    }

    for (int i = 0; i < vertexTextures.size(); i++) {
        delete vertexTextures.at(i);
    }

    for (int i = 0; i < triangles.size(); i++) {
        delete triangles.at(i);
    }
}

// Generate VBO Vertices
float * Model::generateVBOVerticesArray(bool cpuMatrix, bool colorModifier, bool triangleNormal, bool useNormal) {
    // Matrix to use (either identity if using gpuMatrix, or cpuMatrix)
    glm::mat4 matrix = glm::mat4(1);
    if (cpuMatrix) {
        matrix = getMatrix();
    }

    // Generate the array
    float* vertexArray = new float[triangles.size() * 3 * 11];
    for (int i = 0; i < triangles.size(); i++) {
        int index = i * 11 * 3;
        int vertexOneIndex = triangles.at(i)->p1 - 1;
        glm::vec3 color = triangles.at(i)->color;
        float colorModifierVal = colorModifier ? (float) i / (float) (triangles.size() - 1) : 1;
        glm::vec4 vertexOne = matrix * glm::vec4(vertices.at(vertexOneIndex)->x, vertices.at(vertexOneIndex)->y, vertices.at(vertexOneIndex)->z, 1);
        vertexArray[index] = vertexOne.x;
        vertexArray[index + 1] = vertexOne.y;
        vertexArray[index + 2] = vertexOne.z;
        vertexArray[index + 3] = vertexOne.w;
        vertexArray[index + 4] = color.x * colorModifierVal;
        vertexArray[index + 5] = color.y * colorModifierVal;
        vertexArray[index + 6] = color.z * colorModifierVal;
        vertexArray[index + 7] = 1.0f;
        glm::vec3 normal = useNormal ? glm::normalize(glm::vec3(matrix * glm::vec4(getNormal(triangleNormal ? i : vertexOneIndex, triangleNormal), 0))) : glm::vec3(0, 0, 0);
        vertexArray[index + 8] = normal.x;
        vertexArray[index + 9] = normal.y;
        vertexArray[index + 10] = normal.z;

        int vertexTwoIndex = triangles.at(i)->p2 - 1;
        glm::vec4 vertexTwo = matrix * glm::vec4(vertices.at(vertexTwoIndex)->x, vertices.at(vertexTwoIndex)->y, vertices.at(vertexTwoIndex)->z, 1);
        vertexArray[index + 11] = vertexTwo.x;
        vertexArray[index + 12] = vertexTwo.y;
        vertexArray[index + 13] = vertexTwo.z;
        vertexArray[index + 14] = vertexTwo.w;
        vertexArray[index + 15] = color.x * colorModifierVal;
        vertexArray[index + 16] = color.y * colorModifierVal;
        vertexArray[index + 17] = color.z * colorModifierVal;
        vertexArray[index + 18] = 1.0f;
        normal = useNormal ? glm::normalize(glm::vec3(matrix * glm::vec4(getNormal(triangleNormal ? i : vertexTwoIndex, triangleNormal), 0))) : glm::vec3(0, 0, 0);
        vertexArray[index + 19] = normal.x;
        vertexArray[index + 20] = normal.y;
        vertexArray[index + 21] = normal.z;

        int vertexThreeIndex = triangles.at(i)->p3 - 1;
        glm::vec4 vertexThree = matrix * glm::vec4(vertices.at(vertexThreeIndex)->x, vertices.at(vertexThreeIndex)->y, vertices.at(vertexThreeIndex)->z, 1);
        vertexArray[index + 22] = vertexThree.x;
        vertexArray[index + 23] = vertexThree.y;
        vertexArray[index + 24] = vertexThree.z;
        vertexArray[index + 25] = vertexThree.w;
        vertexArray[index + 26] = color.x * colorModifierVal;
        vertexArray[index + 27] = color.y * colorModifierVal;
        vertexArray[index + 28] = color.z * colorModifierVal;
        vertexArray[index + 29] = 1.0f;
        normal = useNormal ? glm::normalize(glm::vec3(matrix * glm::vec4(getNormal(triangleNormal ? i : vertexThreeIndex, triangleNormal), 0))) : glm::vec3(0, 0, 0);
        vertexArray[index + 30] = normal.x;
        vertexArray[index + 31] = normal.y;
        vertexArray[index + 32] = normal.z;
    }

    return vertexArray;
}

// Number of vertices based on mode
int Model::getNumVertices(bool useEBO) {
    return useEBO ? vertices.size() : triangles.size() * 3;
}

// Generate model view projection matrix
glm::mat4 Model::generateModelMatrix() {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1), translate);
    glm::mat4 rotationMatrixX = glm::rotate(angleX, glm::vec3(1, 0, 0));
    glm::mat4 rotationMatrixY = glm::rotate(angleY, glm::vec3(0, 1, 0));
    glm::mat4 rotationMatrixZ = glm::rotate(angleZ, glm::vec3(0, 0, 1));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1), scale);

    return translationMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * scaleMatrix;
}

// Use GLM Library to make View Matrix
glm::mat4 Model::generateViewMatrix() {
    return glm::lookAt(cameraPosition, cameraTarget, upVec);
}

// Use GLM Library to make Projection Matrix
glm::mat4 Model::generateProjectionMatrix() {
    glm::mat4 mat = glm::perspective(glm::radians(fov), aspectRatio, nearClippingPlane, farClippingPlane);
    return glm::perspective(glm::radians(fov), aspectRatio, nearClippingPlane, farClippingPlane);
}

// Get the matrix
glm::mat4 Model::getMatrix() {
    return generateProjectionMatrix() * generateViewMatrix() * generateModelMatrix();
}

// Read a material file
map<string, glm::vec3> Model::readMaterial(string fileName) {
    map<string, glm::vec3> material;
    string currentMat = "";

    // Open the file
    ifstream file(fileName);
    if (!file.is_open()) {
        cout << "File: \'" + fileName + "\' failed to open." << endl;
        return material;
    }

    string currLine;

    // Read each line
    while (getline(file, currLine)) {
        stringstream currLineStream(currLine);
        vector<string> parts;
        string currLinePart;

        // Read each part
        while (getline(currLineStream, currLinePart, ' ')) {
            parts.push_back(currLinePart);
        }

        // Based on first part in line
        if (parts.size() == 0) {
            continue;
        }
        // Change current material
        else if (parts.at(0) == "newmtl") {
            if (parts.size() != 2) {
                continue;
            }

            currentMat = parts.at(1);
        }
        // Use Diffusion Constant as material
        else if (parts.at(0) == "Kd") {
            if (parts.size() != 4) {
                continue;
            }

            material.emplace(currentMat, glm::vec3(stof(parts.at(1)), stof(parts.at(2)), stof(parts.at(3))));
        }
    }

    file.close();

    return material;
}

// Generate EBO Vertices
pair<float*, unsigned int*> Model::generateEBOVerticesArray(bool cpuMatrix, bool colorModifier, bool useNormal) {
    // Matrix to use (either identity if using gpuMatrix, or cpuMatrix)
    glm::mat4 matrix = glm::mat4(1);
    if (cpuMatrix) {
        matrix = getMatrix();
    }

    // Generate Vertices Array
    float* vertexArray = new float[vertices.size() * 11];
    for (int i = 0; i < vertices.size(); i++) {
        int index = i * 11;
        glm::vec3 color = colors.at(i);
        float colorModifierVal = colorModifier ? (float) i / (float) (vertices.size() - 1) : 1;
        glm::vec4 vertex = matrix * glm::vec4(vertices.at(i)->x, vertices.at(i)->y, vertices.at(i)->z, 1);
        vertexArray[index] = vertex.x;
        vertexArray[index + 1] = vertex.y;
        vertexArray[index + 2] = vertex.z;
        vertexArray[index + 3] = vertex.w;
        vertexArray[index + 4] = color.x * colorModifierVal;
        vertexArray[index + 5] = color.y * colorModifierVal;
        vertexArray[index + 6] = color.z * colorModifierVal;
        vertexArray[index + 7] = 1.0f;
        glm::vec3 normal = useNormal ? glm::normalize(glm::vec3(matrix * glm::vec4(getNormal(i, false), 0))) : glm::vec3(0, 0, 0);
        vertexArray[index + 8] = normal.x;
        vertexArray[index + 9] = normal.y;
        vertexArray[index + 10] = normal.z;
    }

    // Generate Indices Array
    unsigned int* indexArray = new unsigned int[triangles.size() * 3];
    for (int i = 0; i < triangles.size(); i++) {
        int index = i * 3;
        indexArray[index] = triangles.at(i)->p1 - 1;
        indexArray[index + 1] = triangles.at(i)->p2 - 1;
        indexArray[index + 2] = triangles.at(i)->p3 - 1;
    }

    return {vertexArray, indexArray};
}

// Constructors
Model::Vertex::Vertex(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
Model::VertexTexture::VertexTexture(float x, float y) {
    this->x = x;
    this->y = y;
}
Model::Triangle::Triangle(int p1, int p2, int p3, glm::vec3 color) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->color = color;
}

// Read a Shader File
char* Model::readShader(string fileName) {
    // Open the File
    ifstream shader(fileName);
    if (!shader.is_open()) {
        cout << "File: \'" << fileName << "\' failed to open." << endl;
    }

    // Convert to a string
    stringstream stream;
    stream << shader.rdbuf();
    string shaderString = stream.str();
    char* shaderArray = new char[shaderString.size() + 1];
    for (int i = 0; i < shaderString.size(); i++) {
        shaderArray[i] = shaderString.at(i);
    }
    shaderArray[shaderString.size()] = '\0';

    shader.close();

    return shaderArray;
}

// Number of indices
int Model::getNumIndices() {
    return triangles.size() * 3;
}

glm::vec3 Model::calculateTriangleNormal(Triangle* triangle) {
    Vertex* pointOneVertex = vertices.at(triangle->p1 - 1);
    glm::vec3 pointOne = glm::vec3(pointOneVertex->x, pointOneVertex->y, pointOneVertex->z);
    Vertex* pointTwoVertex = vertices.at(triangle->p2 - 1);
    glm::vec3 pointTwo = glm::vec3(pointTwoVertex->x, pointTwoVertex->y, pointTwoVertex->z);
    Vertex* pointThreeVertex = vertices.at(triangle->p3 - 1);
    glm::vec3 pointThree = glm::vec3(pointThreeVertex->x, pointThreeVertex->y, pointThreeVertex->z);

    glm::vec3 u = pointTwo - pointOne;
    glm::vec3 w = pointThree - pointTwo;

    glm::vec3 normal = glm::normalize(glm::cross(u, w));
    return normal;
}

glm::vec3 Model::getNormal(int number, bool triangleNormal) {
    if (triangleNormal) {
        Vertex* pointOneVertex = vertices.at(triangles.at(number)->p1 - 1);
        glm::vec3 pointOne = glm::vec3(pointOneVertex->x, pointOneVertex->y, pointOneVertex->z);
        Vertex* pointTwoVertex = vertices.at(triangles.at(number)->p2 - 1);
        glm::vec3 pointTwo = glm::vec3(pointTwoVertex->x, pointTwoVertex->y, pointTwoVertex->z);
        Vertex* pointThreeVertex = vertices.at(triangles.at(number)->p3 - 1);
        glm::vec3 pointThree = glm::vec3(pointThreeVertex->x, pointThreeVertex->y, pointThreeVertex->z);

        glm::vec3 u = pointTwo - pointOne;
        glm::vec3 w = pointThree - pointTwo;

        glm::vec3 normal = glm::normalize(glm::cross(u, w));
        return calculateTriangleNormal(triangles.at(number));
    }
    else {
        glm::vec3 average(0, 0, 0);

        for (int i = 0; i < vertexTriangles.at(number).size(); i++) {
            average += calculateTriangleNormal(vertexTriangles.at(number).at(i));
        }

        average /= vertexTriangles.at(number).size();

        return average;
    }
}
