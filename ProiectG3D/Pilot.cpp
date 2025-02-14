#include "Pilot.h"




// Constructorul clasei Pilot
Pilot::Pilot(std::string const& path, bool bSmoothNormals, bool gamma)
    : Model(path, bSmoothNormals, gamma), _offset(3.3f, -0.2f, -1.0f) // Ajustează înălțimea pentru poziție
{
}

// Metodă pentru a seta poziția pilotului în funcție de poziția kart-ului
void Pilot::UpdatePosition(const glm::vec3& kartPosition, float kartRotationAngle) {
    // Resetăm matricea de transformare
    _rootTransf = glm::mat4(1.0f);

    // Aplicăm translația kartului
    _rootTransf = glm::translate(_rootTransf, kartPosition);

    // Aplicăm rotația kartului
    _rootTransf = glm::rotate(_rootTransf, glm::radians(kartRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Aplicăm offset-ul pentru poziționarea pilotului față de centrul kartului
    _rootTransf = glm::translate(_rootTransf, _offset);

    // Ajustăm rotația pilotului (dacă este necesar)
    _rootTransf = glm::rotate(_rootTransf, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Aplicăm scalarea pentru a micșora pilotul mai mult
    _rootTransf = glm::scale(_rootTransf, glm::vec3(0.1f));
}

//void Pilot::UpdatePosition(const glm::vec3& kartPosition, float kartRotationAngle) {
//    // Resetăm matricea de transformare
//    _rootTransf = glm::mat4(1.0f);
//
//    // Aplicăm translația bazată pe poziția kart-ului și pe offset
//    _rootTransf = glm::translate(_rootTransf, kartPosition + _offset);
//
//    // Aplicăm rotația pentru a alinia pilotul drept în kart  
//    _rootTransf = glm::rotate(_rootTransf, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//
//    // Aplicăm rotația kartului pentru a sincroniza pilotul
//    _rootTransf = glm::rotate(_rootTransf, glm::radians(kartRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
//
//    // Aplicăm scalarea pentru a micșora pilotul mai mult
//    _rootTransf = glm::scale(_rootTransf, glm::vec3(0.1f));
//}
void Pilot::Draw(Shader& shader)
{
    shader.setMat4("model", _rootTransf);
    Model::Draw(shader);
}

