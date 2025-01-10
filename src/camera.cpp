#include "camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    view = glm::lookAt(Position, Position + Orientation, Up);

    projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

    cameraMatrix = projection * view;
}

void Camera::Matrix(Shader &shader, const char *uniform)
{

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
    glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), Position.x, Position.y, Position.z);
}

void Camera::Inputs(GLFWwindow *window, float pivotDist)
{
    // Get ImGui I/O structure
    ImGuiIO &io = ImGui::GetIO();

    static bool rKeyPressedLastFrame = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (!rKeyPressedLastFrame)
        {
            isAutoRotating = !isAutoRotating;
            rKeyPressedLastFrame = true;
        }
    }
    else
    {
        rKeyPressedLastFrame = false;
    }

    if (isAutoRotating)
    {
        autoRotate(window, 0.0f, 0.0f, 0.0f, 10.0f, 0.6f); // Example center at (0,0,0), distance 5, rotation speed 0.5
    }

    // If ImGui wants to capture the mouse, don't process camera inputs
    if (io.WantCaptureMouse)
    {
        return; // Exit the function early, no camera input when interacting with ImGui
    }

    // Camera position movement (WASD)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Position += speed * -glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Position += speed * -Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Position += speed * glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Position += speed * -Up;
    }

    // Adjust speed with shift key
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 0.004f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 0.001f;
    }

    // Check if any mouse button (left or right) is pressed
    static glm::vec3 dynamicPivotCenter = glm::vec3(0.0f); // Dynamic pivot center
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (firstClick)
        {
            glfwSetCursorPos(window, (width / 2), (height / 2));
            firstClick = false;
        }

        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Handle camera orientation if left mouse button is pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

            glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

            if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                Orientation = newOrientation;
            }

            Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

            // Update the pivot center based on the new orientation
            dynamicPivotCenter = Position + Orientation * pivotDist; // Assuming a pivot distance of 5 units
        }

        // Handle camera rotation around pivotCenter if right mouse button is pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

            // Calculate the radius (distance from the pivotCenter point)
            float radius = glm::length(Position - dynamicPivotCenter);

            // Translate Position to the dynamicPivotCenter's local space
            glm::vec3 localPos = Position - dynamicPivotCenter;

            // Horizontal rotation (yaw) around the Y axis
            float yawAngle = glm::radians(-rotY);
            localPos = glm::rotateY(localPos, yawAngle);

            // Vertical rotation (pitch) around the right axis (cross of up and orientation)
            glm::vec3 right = glm::normalize(glm::cross(Up, Orientation));
            float pitchAngle = glm::radians(-rotX);
            localPos = glm::rotate(localPos, pitchAngle, right);

            // Translate back to global space
            Position = localPos + dynamicPivotCenter;

            // Maintain the same distance (radius) from the dynamicPivotCenter point
            Position = glm::normalize(Position - dynamicPivotCenter) * radius + dynamicPivotCenter;

            // Always look towards the dynamicPivotCenter point
            Orientation = glm::normalize(dynamicPivotCenter - Position);
        }

        // Reset the mouse position to the center
        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else
    {
        // Show the cursor again when neither mouse button is pressed
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}

void Camera::autoRotate(GLFWwindow *window, float centerX, float centerY, float centerZ, float distance, float rotationSpeed)
{
    // Calculate the direction vector from the camera to the center
    glm::vec3 center(centerX, centerY, centerZ);
    glm::vec3 direction = glm::normalize(Position - center);

    // Set the camera's position in a circular path around the center
    float radius = distance;

    // Calculate new position by rotating around the Y-axis
    float angle = rotationSpeed * glfwGetTime(); // Time-based angle for smooth rotation
    float x = center.x + radius * cos(angle);
    float z = center.z + radius * sin(angle);
    float y = centerY + 1.0f;

    Position = glm::vec3(x, y, z);

    // Ensure the camera looks at the center point
    Orientation = glm::normalize(center - Position);
}
