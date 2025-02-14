#ifndef MOVINGOBJECT_H
#define MOVINGOBJECT_H

#include "Model.h"

class MovingObject : public Model {
public:
    MovingObject(std::string const& path, bool bSmoothNormals = true, bool gamma = false);
    void SetRootTransf(glm::mat4 rootTransf);
    void Move(float deltaTime, float speed);
    void Draw(Shader& shader);

private:
    glm::mat4 _rootTransf;
    float _wheelRotationAngle;  // Unghiul de rotație pentru roți
    float _direction;  // Direcția de mișcare (stânga / dreapta)
};

#endif // MOVINGOBJECT_H
