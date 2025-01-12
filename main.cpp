#include <vector>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <random>

#include "light.h"
#include "physx.h"
#include "hash.h"

const unsigned int width = 1600;
const unsigned int height = 900;

std::vector<Model *> Model::models;
std::vector<Light *> Light::lights;
int Light::pointLightCount = 0;

std::vector<Particle *> Particle::particles;

float randomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

std::vector<glm::vec3> generateSpherePoints(int numPoints, float radius)
{
    std::vector<glm::vec3> points;
    for (int i = 0; i < numPoints; ++i)
    {
        // Generate random spherical coordinates
        float theta = randomFloat(0.0f, 2.0f * glm::pi<float>());
        float phi = randomFloat(0.0f, glm::pi<float>());

        // Convert spherical coordinates to Cartesian coordinates
        float x = radius * sin(phi) * cos(theta);
        float y = radius * sin(phi) * sin(theta);
        float z = radius * cos(phi);

        // Store the point in a vector
        points.push_back(glm::vec3(x, y, z));
    }
    return points;
}

std::vector<glm::vec3> generateIcoSpherePoints(float radius)
{
    std::vector<glm::vec3> points;

    float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
    float a = 1.0f;
    float b = 1.0f / phi;

    points.push_back(radius * glm::vec3(0, b, -a));
    points.push_back(radius * glm::vec3(b, a, 0));
    points.push_back(radius * glm::vec3(-b, a, 0));
    points.push_back(radius * glm::vec3(0, b, a));
    points.push_back(radius * glm::vec3(0, -b, a));
    points.push_back(radius * glm::vec3(-a, 0, b));
    points.push_back(radius * glm::vec3(0, -b, -a));
    points.push_back(radius * glm::vec3(a, 0, -b));
    points.push_back(radius * glm::vec3(a, 0, b));
    points.push_back(radius * glm::vec3(-a, 0, -b));
    points.push_back(radius * glm::vec3(b, -a, 0));
    points.push_back(radius * glm::vec3(-b, -a, 0));

    return points;
}

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

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    gladLoadGL();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, width, height);

    Shader shader("res/shaders/default.vert", "res/shaders/default.frag");

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 10.0f));

    glm::vec3 sunDirection(1.0f, 1.0f, 1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec3 ambient(0.15f, 0.15f, 0.15f);

    auto lastTime = std::chrono::high_resolution_clock::now();

    Physx physx;

    int numPoints = 12;
    float constraintRadius = 3.1f;
    float particleRadius = 0.1f;

    Hash hash(2 * particleRadius, 1200);

    Particle p(particleRadius, constraintRadius);
    Particle p1(particleRadius, constraintRadius);
    Particle p2(particleRadius, constraintRadius);
    Particle p3(particleRadius, constraintRadius);

    std::vector<glm::vec3> spherePoints = generateCubeSpherePoints(numPoints, constraintRadius);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        float dt = elapsed.count();
        lastTime = currentTime;

        glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        camera.Inputs(window);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);

        physx.update(particleRadius, dt, 9.81, constraintRadius, 8, &hash);

        shader.Activate();

        glm::mat4 model = glm::mat4(1.0f);                          // Identity matrix
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate to center
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Draw the points (fixed position around the center)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Draw points
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, spherePoints.size()); // Draw points around the center
        glBindVertexArray(0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Draw other models
        for (Model *model : Model::models)
        {
            model->Draw(shader, camera);
        }

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

        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.0f, 128.0f, 0.0f, 255.0f), "Particle Count: %i", Particle::particles.size());

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::DragFloat3("Sun Direction", &sunDirection[0], 0.1f);
        ImGui::ColorEdit3("Particle Color", &color[0], 0.1f);
        ImGui::ColorEdit3("Ambient Lighting", &ambient[0], 0.1f);
        ImGui::DragFloat("Particle Radius", &particleRadius, 0.1f);

        static int spawnCount = 1; // Default spawn count

        ImGui::InputInt("Particle Count", &spawnCount); // Input box to adjust count
        if (spawnCount < 1)
            spawnCount = 1; // Ensure count is at least 1

        // Button to spawn particles
        if (ImGui::Button("Spawn Particle(s)"))
        {
            for (int i = 0; i < spawnCount; i++)
            {
                new Particle(particleRadius, constraintRadius); // Spawn particles

                // hash.~Hash(); // Explicitly call the destructor
                // new (&hash) Hash(2 * particleRadius, Particle::particles.size());
            }
        }
        ImGui::End();

        ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        // int count = 0;
        // for (Model *model : Model::models)
        // {
        //     model->name = "p(" + std::to_string(count) + ")";
        //     model->UI();
        //     count++;
        // }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    for (Model *model : Model::models)
    {
        model->SaveImGuiData("saveData/transforms.json");
        delete model;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
