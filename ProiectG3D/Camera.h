#pragma once

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>
#include "CameraMovementType.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

enum class CameraMode {
    FREE,
	FIRST_PERSON,
	THIRD_PERSON
};

class Camera
{
    const float zNEAR = 0.1f;
    const float zFAR = 2500.f;
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float FOV = 45.0f;
    glm::vec3 startPosition;
	glm::vec3 targetKartPosition;
	glm::vec3 kartForward;
	CameraMode currentMode;

protected:
    const float cameraSpeedFactor = 29.5f;
    const float mouseSensitivity = 0.2f;

    // Perspective properties
    float zNear;
    float zFar;
    float FoVy;
    int width;
    int height;
    bool isPerspective;

    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;
    float pitch;

    bool bFirstMouseMove = true;
    float lastX = 0.f, lastY = 0.f;

public:

    Camera(const int width, const int height, const glm::vec3& position, CameraMode mode = CameraMode::THIRD_PERSON);
    void Set(const int width, const int height, const glm::vec3& position);
	void SetCameraMode(CameraMode mode);
    void Reset(int width, int height);
    void Reshape(int width, int height);
    const glm::mat4 GetViewMatrix() const;
    const glm::vec3 GetPosition() const;
    const glm::mat4 GetProjectionMatrix() const;
    void ProcessKeyboard(CameraMovementType direction, float deltaTime);
    void MouseControl(float xpos, float ypos);
    void ProcessMouseScroll(float yOffset);
    void UpdateKartPosition(const glm::vec3& position, const glm::vec3& forwardVector);

    CameraMode GetCameraMode() const;

private:

    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void UpdateCameraVectors();
};

