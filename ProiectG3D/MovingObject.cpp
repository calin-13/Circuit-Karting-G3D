#include "MovingObject.h"
#include <glfw3.h>

MovingObject::MovingObject(std::string const& path, bool bSmoothNormals, bool gamma)
    : Model(path, bSmoothNormals, gamma), _wheelRotationAngle(0.0f), _direction(0.0f) {
}

void MovingObject::SetRootTransf(glm::mat4 rootTransf) {
    _rootTransf = rootTransf;
}

void MovingObject::Move(float deltaTime, float speed) {
    // Mișcare pe axa X (stânga/dreapta)
    _direction += speed * deltaTime;

    // Rotește roțile pe măsură ce se mișcă obiectul
    _wheelRotationAngle += speed * deltaTime * 100.0f;  // Reglează viteza de rotație a roților
}

void MovingObject::Draw(Shader& shader) {
    shader.setMat4("model", _rootTransf);

    double currentFrame = glfwGetTime();
    glm::mat4 rotation = glm::rotate(_rootTransf, (float)(20.0f * currentFrame), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(_direction, 0.0f, 0.0f));

    // Aplică mișcarea pe axa X (stânga/dreapta)
    glm::mat4 modelMatrix = translation * rotation;

    for (unsigned int i = 0; i < meshes.size(); i++) {
        if (meshes[i].name == "Wheel") {
            // Rotește roțile pe axa Z
            glm::mat4 wheelRotation = glm::rotate(glm::mat4(1.0f), glm::radians(_wheelRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.setMat4("model", wheelRotation * modelMatrix);
        }
        else {
            shader.setMat4("model", modelMatrix);
        }
        meshes[i].Draw(shader);
    }
}
