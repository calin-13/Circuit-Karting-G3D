#pragma once

#include "Model.h"
#include <glfw3.h>

class Pilot : public Model {
public:

	Pilot() = default;
    // Constructorul clasei Pilot
    Pilot(std::string const& path, bool bSmoothNormals = false, bool gamma = false);

    // Metodă pentru a seta poziția pilotului în funcție de poziția kart-ului
    void UpdatePosition(const glm::vec3& kartPosition, float kartRotationAngle);

    // Suprascrie metoda Draw pentru a desena pilotul
    void Draw(Shader& shader) override;

private:
    glm::mat4 _rootTransf;     // Transformarea rădăcină pentru pilot
    glm::vec3 _offset;         // Offset-ul pilotului față de kart
};

