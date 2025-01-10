#include <vector>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <random>
#include <iostream>

#include "light.h"
#include "physx.h"
#include "computeShader.h"
#include "shaderClass.h"
#include <GL/gl.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

std::vector<glm::vec3> generateCubeSpherePoints(int pointsPerRow, float radius)
{
    const float DEG2RAD = acos(-1) / 180.0f;

    std::vector<glm::vec3> vertices;
    float n1[3]; // normal of longitudinal plane rotating along Y-axis
    float n2[3]; // normal of latitudinal plane rotating along Z-axis
    float v[3];  // direction vector intersecting 2 planes, n1 x n2
    float a1;    // longitudinal angle along Y-axis
    float a2;    // latitudinal angle along Z-axis

    // rotate latitudinal plane from 45 to -45 degrees along Z-axis (top-to-bottom)
    for (unsigned int i = 0; i < pointsPerRow; ++i)
    {
        // normal for latitudinal plane
        // if latitude angle is 0, then normal vector of latitude plane is n2=(0,1,0)
        // therefore, it is rotating (0,1,0) vector by latitude angle a2
        a2 = DEG2RAD * (45.0f - 90.0f * i / (pointsPerRow - 1));
        n2[0] = -sin(a2);
        n2[1] = cos(a2);
        n2[2] = 0;

        // rotate longitudinal plane from -45 to 45 along Y-axis (left-to-right)
        for (unsigned int j = 0; j < pointsPerRow; ++j)
        {
            // normal for longitudinal plane
            // if longitude angle is 0, then normal vector of longitude is n1=(0,0,-1)
            // therefore, it is rotating (0,0,-1) vector by longitude angle a1
            a1 = DEG2RAD * (-45.0f + 90.0f * j / (pointsPerRow - 1));
            n1[0] = -sin(a1);
            n1[1] = 0;
            n1[2] = -cos(a1);

            // find direction vector of intersected line, n1 x n2
            v[0] = n1[1] * n2[2] - n1[2] * n2[1];
            v[1] = n1[2] * n2[0] - n1[0] * n2[2];
            v[2] = n1[0] * n2[1] - n1[1] * n2[0];

            // normalize direction vector
            float scale = 1 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            v[0] *= scale;
            v[1] *= scale;
            v[2] *= scale;

            // add a vertex into array
            vertices.push_back(glm::vec3(v[0], v[1], v[2]) * radius);   // +X face
            vertices.push_back(glm::vec3(-v[0], v[1], -v[2]) * radius); // -X face
            vertices.push_back(glm::vec3(-v[2], v[0], -v[1]) * radius); // +Y Face
            vertices.push_back(glm::vec3(v[2], -v[0], v[1]) * radius);  // -Y Face
            vertices.push_back(glm::vec3(v[2], v[1], -v[0]) * radius);  // +Z Face
            vertices.push_back(glm::vec3(-v[2], v[1], v[0]) * radius);  // -Z Face
        }
    }

    return vertices;
}

float randomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

glm::vec3 randomVec3(float min, float max)
{
    return glm::vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
}

// settings
const unsigned int width = 1600;
const unsigned int height = 900;

std::vector<Model *> Model::models;
std::vector<Light *> Light::lights;
int Light::pointLightCount = 0;

std::vector<Particle *> Particle::particles;

struct Obj
{
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 prevPos;
    alignas(16) glm::vec3 acc;
};

int main(int argc, char *argv[])
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(width, height, "tufphysXGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);

    int windowPosX = (videoMode->width - width) / 2;
    int windowPosY = (videoMode->height - height) / 2;

    glfwSetWindowPos(window, windowPosX, windowPosY);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup compute shader
    ComputeShader computeShader("res/shaders/particle.comp");

    auto lastTime = std::chrono::high_resolution_clock::now();

    // Settings
    int numPoints = 12;
    float constraintRadius = 3.1f;
    float particleRadius = 0.1f;
    int subSteps = 8;
    float maxSpeed = 3.333f;
    float pivotDist = 10.f;

    // Setup particle data in an SSBO
    std::vector<Obj> objs;

    // Spatial Hashing Settings
    float hashSpacing = particleRadius * 2.0f;
    int hashTableSize = 5 * objs.size();

    // // Add more particles if needed
    // for (int i = 0; i < 10000; ++i)
    // {
    //     glm::vec3 randomOffset = randomVec3(-0.01f, 0.01f); // Small random perturbation
    //     objs.push_back({{randomOffset}, {glm::vec3(0.0f)}, {0.0f, 0.0f, 0.0f}, particleRadius});
    // }

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Cell Count Buffer
    GLuint cellCountBuffer;
    glGenBuffers(1, &cellCountBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellCountBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellCountBuffer); // Binding = 1
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Particle Map Buffer
    GLuint particleMapBuffer;
    glGenBuffers(1, &particleMapBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleMapBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleMapBuffer); // Binding = 2
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Query IDs Buffer
    GLuint queryIdsBuffer;
    glGenBuffers(1, &queryIdsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, queryIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, queryIdsBuffer); // Binding = 3
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // FirstAdjacency Buffer
    GLuint firstAdjIdsBuffer;
    glGenBuffers(1, &firstAdjIdsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, firstAdjIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, firstAdjIdsBuffer); // Binding = 4
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Adjacency Buffer
    GLuint adjIdsBuffer;
    glGenBuffers(1, &adjIdsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, adjIdsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, adjIdsBuffer); // Binding = 5
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Setup shaders
    Shader shader("res/shaders/particle.vert", "res/shaders/particle.frag");
    Shader icoboundsShader("res/shaders/icobounds.vert", "res/shaders/icobounds.frag");
    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 10.0f));
    // camera.Orientation = glm::vec3(-0.083f, -0.088f, 0.993f);

    glm::vec3 sunDirection(1.0f, 1.0f, 1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec3 ambient(0.02f, 0.02f, 0.02f);

    std::vector<glm::vec3>
        spherePoints = generateCubeSpherePoints(numPoints, constraintRadius);

    // Create VBO and VAO for drawing points
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, spherePoints.size() * sizeof(glm::vec3), &spherePoints[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    Model pModel("res/models/Shapes/icosphere.gltf", "particle", false);

    std::vector<Vertex> vertices = pModel.meshes[0].vertices;
    std::vector<GLuint> indices = pModel.meshes[0].indices;

    GLuint pVAO, pVBO, pEBO;

    glGenVertexArrays(1, &pVAO);
    glGenBuffers(1, &pVBO);
    glGenBuffers(1, &pEBO);

    glBindVertexArray(pVAO);

    // Set up vertex buffer (pVBO)
    glBindBuffer(GL_ARRAY_BUFFER, pVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Set up element buffer (pEBO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        float dt = elapsed.count();
        lastTime = currentTime;

        GLuint workgroupSize = 128; // This can be adjusted based on the GPU's capabilities

        // Calculate number of workgroups needed
        GLuint numWorkgroups = (std::max(int(objs.size()), hashTableSize) + workgroupSize - 1) / workgroupSize; // Ceil(particleCount / workgroupSize)

        // Compute shader
        computeShader.use();
        computeShader.setFloat("dt", dt);
        computeShader.setFloat("g", 9.81f);
        computeShader.setFloat("cr", constraintRadius + 0.25f);
        computeShader.setFloat("radius", particleRadius);
        computeShader.setInt("particleCount", objs.size());
        computeShader.setInt("subSteps", subSteps);
        bool spacePressed = false;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            spacePressed = true;

        computeShader.setBool("pullToCenter", spacePressed);
        computeShader.setFloat("maxSpeed", maxSpeed);
        computeShader.setFloat("hash.spacing", hashSpacing);
        computeShader.setInt("hash.maxObjs", objs.size());
        computeShader.setInt("hash.tableSize", hashTableSize);
        // computeShader.setVec3("offset", lorenzOffset);

        glDispatchCompute(numWorkgroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        camera.Inputs(window, pivotDist);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        // Render the cube
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        icoboundsShader.Activate();
        glm::mat4 m = glm::mat4(1.0f);                      // Identity matrix
        m = glm::translate(m, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate to center
        glUniformMatrix4fv(glGetUniformLocation(icoboundsShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(m));
        glUniformMatrix4fv(glGetUniformLocation(icoboundsShader.ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.cameraMatrix));

        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Draw points
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, spherePoints.size()); // Draw points around the center
        glBindVertexArray(0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        shader.Activate();
        glm::mat4 model = glm::mat4(1.0);
        // model = glm::scale(model, glm::vec3(particleRadius));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.cameraMatrix));
        glUniform1f(glGetUniformLocation(shader.ID, "scale"), particleRadius);
        glUniform1f(glGetUniformLocation(shader.ID, "maxSpeed"), maxSpeed);

        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(pVAO);
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, objs.size());
        glBindVertexArray(0);

        glUniform3f(glGetUniformLocation(shader.ID, "sunDirection"), sunDirection.x, sunDirection.y, sunDirection.z);
        glUniform3f(glGetUniformLocation(shader.ID, "color"), color.x, color.y, color.z);
        glUniform3f(glGetUniformLocation(shader.ID, "ambient"), ambient.x, ambient.y, ambient.z);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(128.0f, 0.0f, 128.0f, 255.0f), "Stats & Settings");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        // ImGui::DragFloat3("Camera Pos", &camera.Position[0], 0.1f);
        // ImGui::DragFloat3("Camera Orientation", &camera.Orientation[0], 0.1f);
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.0f, 128.0f, 0.0f, 255.0f), "Particle Count: %i", objs.size());

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::DragFloat3("Sun Direction", &sunDirection[0], 0.1f);
        ImGui::ColorEdit3("Particle Color", &color[0], 0.1f);
        ImGui::ColorEdit3("Ambient Lighting", &ambient[0], 0.1f);
        // ImGui::DragFloat("Particle Radius", &particleRadius, 0.1f);
        // ImGui::DragFloat("Max Speed", &maxSpeed, 0.001f);
        // ImGui::DragFloat("Pivot Dist", &pivotDist, 0.1f);

        ImGui::TextColored(ImVec4(0.0f, 128.0f, 128.0f, 255.0f), "Spatial Hashing Settings");
        ImGui::DragFloat("Spacing", &hashSpacing);

        ImGui::DragInt("Sub Steps", &subSteps);
        static int spawnCount = 1;                      // Default spawn count
        ImGui::InputInt("Particle Count", &spawnCount); // Input box to adjust count
        if (spawnCount < 1)
            spawnCount = 1; // Ensure count is at least 1

        // Button to spawn particles
        if (ImGui::Button("Spawn Particle(s)"))
        {
            for (int i = 0; i < spawnCount; i++)
            {
                // Generate a random position and velocity for the new particle
                glm::vec3 randomPos = randomVec3(-constraintRadius, constraintRadius);

                // Create the new particle object
                Obj newParticle = {{glm::vec3(0)}, {randomPos}, {0.0f, 0.0f, 0.0f}};

                // Add the new particle to the 'objs' vector
                objs.push_back(newParticle);

                hashTableSize = 5 * objs.size();

                // Resize the SSBO to accommodate the new particle
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Obj) * objs.size(), objs.data(), GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellCountBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * hashTableSize + 1, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellCountBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleMapBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * objs.size(), nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleMapBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, queryIdsBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * objs.size(), nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, queryIdsBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, firstAdjIdsBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * objs.size() + 1, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, firstAdjIdsBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, adjIdsBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * objs.size() * 10, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, adjIdsBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteProgram(computeShader.ID);

    glfwTerminate();

    return EXIT_SUCCESS;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
