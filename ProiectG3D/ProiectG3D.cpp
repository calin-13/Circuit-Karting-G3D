#include <Windows.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>
#include <xmemory>
#include <array>
#include <algorithm>
#include <random>
#include <ctime>
#include <map>


#include <SFML/Audio.hpp>

#include "ShaderManager.h"
#include "Model.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Pilot.h"

#define M_PI 3.14159265358979323846

//#include "CameraMovementType.h"

// settings
//c
std::shared_ptr<Camera> pCamera = nullptr;
std::shared_ptr<SkyBox> skybox = nullptr;

struct Kart {
	glm::vec3 position;
	glm::vec3 kartDirection;
	float rotationAngle;
	float kartAcceleration;
	Model kartModel;
	std::shared_ptr<Pilot> pilotModel;
	bool isCurrentPlayer = false;
	bool kartAccelerationChanged = false;
	int index;
	float mass;
	float kartSteeringAngle = 0.0f;

	Kart() = default;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RenderShadowPass(Shader& depthShader,unsigned int depthMapFBO,unsigned int SHADOW_WIDTH, unsigned int SHADOW_HEIGHT,const glm::mat4& lightSpaceMatrix,std::unordered_map<std::string, Model>& models,std::array<Kart, 12>& karts);
void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string,Model>& models, std::array<Kart, 12>& karts, std::shared_ptr<SkyBox> skyBoxInstance);
void RenderSkybox(Shader& shader,std::shared_ptr<SkyBox> skyBoxInstance);
void RenderTrack(Shader& shader, Model& model);
void RenderKarts(Shader& shader, std::array<Kart, 12>& karts);
void RenderTerrain(Shader& shader, Model& model);
void RenderSun(Shader& shader, Model& sunModel, const glm::vec3& sunPos);
void LoadMultipleKarts(std::array<Kart, 12>& karts, std::unordered_map<std::string, Model>& models, std::shared_ptr<Pilot> pilot);
bool checkCollision(const Kart& kart1, const Kart& kart2);
void handleCollision(Kart& kart1, Kart& kart2);
float random_floatant_number();
void loadSounds();
void playSound(const std::string& soundName);
void stopSound(const std::string& soundName);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

float maxDeltaTime = 0.1f;
float minDeltaTime = 0.01f;

glm::vec3 kartPos(39.0f, 0.0f, 320.0f);  // Poziția inițială a kart-ului
float kartSpeed = 15.0f;               // Viteza de mișcare a kart-ului

glm::vec3 lightPos(0.0f, 2.0f, 1.0f); // Pozitia luminii
glm::vec3 sunPos(20.0f, 25.0f, -10.0f);//Soarele

float kartRotationAngle = 0.0f; // Unghiul curent al kart-ului
float kartRotationSpeed = 90.0f; // Viteza de rotație în grade pe secundă

float kartAcceleration = 0.0f; // Accelerarea curentă
float kartMaxSpeed = 50.0f; // Viteza maximă
float kartAccelerationRate = 20.0f; // Rata de accelerare
float kartDecelerationRate = 5.0f; // Rata de decelerare
float maxCollisionDistance = 4.2f; // Distanța maximă de coliziune



int currentKartIndex = 5; // Indexul următorului kart
int lastKartIndex = -1; // Indexul kart-ului anterior
int hitKartIndex = 1; // Indexul kart-ului lovit

std::array<Kart, 12> karts; // Vectorul de kart-uri

unsigned int depthMapFBO = 0;
unsigned int depthMap = 0;
unsigned int cubeVAO = 0;

std::map<std::string, sf::SoundBuffer> soundBuffers;
std::map<std::string, sf::Sound> sounds;

std::string currentPath;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	ShaderManager shaderManager;
	std::unordered_map<std::string, Model> models;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Lab 7", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	glViewport(0, 0, mode->width, mode->height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	pCamera = std::make_shared<Camera>(mode->width, mode->height, glm::vec3(0.0, 2.0, 6.0));

	glm::vec3 cubePos(0.0f, 5.0f, 1.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	currentPath = converter.to_bytes(wscurrentPath);

	skybox = std::make_shared<SkyBox>(currentPath + "\\Textures\\SkyBox\\", "back.jpg", "front.jpg", "top.jpg", "bottom.jpg", "left.jpg", "right.jpg");

	shaderManager.LoadShader("lightingShader", currentPath + "\\Shaders\\PhongLight.vs", currentPath + "\\Shaders\\PhongLight.fs");
	shaderManager.LoadShader("lightingWithTextureShader", currentPath + "\\Shaders\\PhongLightWithTexture.vs", currentPath + "\\Shaders\\PhongLightWithTexture.fs");
	shaderManager.LoadShader("lampShader", currentPath + "\\Shaders\\Lamp.vs", currentPath + "\\Shaders\\Lamp.fs");
	shaderManager.LoadShader("skyboxShader", currentPath + "\\Shaders\\SkyBox.vs", currentPath + "\\Shaders\\SkyBox.fs");
	shaderManager.LoadShader("shadowMappingDepthShader", currentPath + "\\Shaders\\ShadowMappingDepth.vs", currentPath + "\\Shaders\\ShadowMappingDepth.fs");
	shaderManager.LoadShader("sunShader",currentPath + "\\Shaders\\PhongLight.vs",currentPath + "\\Shaders\\PhongLight.fs");

	std::string go_kartObjFileName = (currentPath + "\\Models\\Kart\\go_kart.obj");
	Model go_kartObjModel(go_kartObjFileName, false);
	models["go_kart"] = go_kartObjModel;

	std::string pilotObjFileName = (currentPath + "\\Models\\Pilot\\pilot_good2.obj");
	std::shared_ptr<Pilot> PilotModel = std::make_shared<Pilot>(pilotObjFileName, false);

	LoadMultipleKarts(karts, models, PilotModel);
	loadSounds();

	std::string TrackObjFileName = (currentPath + "\\Models\\Track\\track.obj");
	Model TrackModel(TrackObjFileName, false);
	models["track"] = TrackModel;

	std::string terrainObjFileName = (currentPath + "\\Models\\Terrain\\terrain.obj");
	Model TerrainModel(terrainObjFileName, false);
	models["terrain"] = TerrainModel;

	std::string sunObjFileName = (currentPath + "\\Models\\Sun\\sun.obj");
	Model sunObjModel(sunObjFileName, false);
	models["sun"] = sunObjModel;
	
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glfwSwapInterval(0);

	float maxDeltaTime = 0.1f;

	while (!glfwWindowShouldClose(window))
	{
		// 1) Actualizare timp și intrări
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		deltaTime = (deltaTime > maxDeltaTime) ? maxDeltaTime : deltaTime;
		lastFrame = currentFrame;

		processInput(window); // Controlul utilizatorului

		// 2) Poziția dinamică a luminii
		lightPos.x = 5.0 * cos(glfwGetTime());
		lightPos.z = 5.0 * sin(glfwGetTime());

		// 3) Calcul matrice lumină
		glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
		glm::mat4 lightView = glm::lookAt(sunPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// 4) Pasul de umbrire (Shadow Pass)
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		Shader& depthShader = shaderManager.GetShader("shadowMappingDepthShader");
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		RenderShadowPass(depthShader, depthMapFBO, SHADOW_WIDTH, SHADOW_HEIGHT, lightSpaceMatrix, models, karts);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Dezactivare FBO pentru randare normală

		// 5) Pasul final (randare normală)
		glViewport(0, 0, mode->width, mode->height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Shader& finalShader = shaderManager.GetShader("lightingWithTextureShader");
		finalShader.use();
		finalShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		finalShader.setInt("shadowMap", 1);

		RenderScene(shaderManager, models, karts, skybox);

		// 6) Actualizare buffer și evenimente
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::DOWN, (float)deltaTime);

	if (sounds["kart_acceleration"].getStatus() != sf::Sound::Playing)
		if (sounds["kart_running_engine"].getStatus() != sf::Sound::Playing)
			playSound("kart_running_engine");

	//Parcurgem toate kart-urile pentru a aplica decelerarea
	for (size_t i = 0; i < karts.size(); ++i) {
		auto& kart = karts[i]; 

		if (kart.kartAcceleration != 0)
		{
			kart.kartAccelerationChanged = true;
			if (kart.kartAcceleration > 0.0f)
			{
				kart.kartAcceleration -= kartDecelerationRate * static_cast<float>(deltaTime);
				kart.kartAcceleration = std::max(kart.kartAcceleration, 0.0f);
			}
			else
			{
				kart.kartAcceleration += kartDecelerationRate * static_cast<float>(deltaTime);
				kart.kartAcceleration = std::min(kart.kartAcceleration, 0.0f);
			}
		}
		else
		{
			kart.kartAccelerationChanged = false;
		}	

		if (!kart.kartAccelerationChanged)
			continue;

		glm::vec3 kartDirection = glm::vec3(
			sin(glm::radians(kart.rotationAngle)),
			0.0f,
			cos(glm::radians(kart.rotationAngle))
		);

		kart.position += kartDirection * kart.kartAcceleration * static_cast<float>(deltaTime);
	}

	for (size_t i = 0; i < karts.size(); ++i) {
		for (size_t j = i + 1; j < karts.size(); ++j) { // Iterăm doar peste perechile de karturi neverificate
			if (checkCollision(karts[i], karts[j])) {

				if (sounds["kart_crash"].getStatus() != sf::Sound::Playing)
					playSound("kart_crash");

				handleCollision(karts[i], karts[j]);
				if (karts[i].kartDirection.z - karts[j].kartDirection.z < 0 || karts[i].kartDirection.x - karts[j].kartDirection.x < 0)
				{
					karts[i].position += karts[i].kartDirection * karts[i].kartAcceleration * static_cast<float>(deltaTime);
					karts[j].position -= karts[j].kartDirection * karts[j].kartAcceleration * static_cast<float>(deltaTime);
				}
				else
				{
					karts[i].position -= karts[i].kartDirection * karts[i].kartAcceleration * static_cast<float>(deltaTime);
					karts[j].position += karts[j].kartDirection * karts[j].kartAcceleration * static_cast<float>(deltaTime);
				}
				
			}
		}
	}

	// Aplicați mișcările doar pentru kart-ul curent
	glm::vec3 kartDirection(
		sin(glm::radians(karts[currentKartIndex].rotationAngle)),
		0.0f,
		cos(glm::radians(karts[currentKartIndex].rotationAngle))
	);

	// Mișcare înainte
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		karts[currentKartIndex].kartAcceleration += kartAccelerationRate * static_cast<float>(deltaTime);
		karts[currentKartIndex].kartAcceleration = std::min(karts[currentKartIndex].kartAcceleration, kartMaxSpeed);

		if (sounds["kart_running_engine"].getStatus() == sf::Sound::Playing)
			stopSound("kart_running_engine");

		if (sounds["kart_acceleration"].getStatus() != sf::Sound::Playing)
			playSound("kart_acceleration");
	}

	// Mișcare înapoi
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		karts[currentKartIndex].kartAcceleration -= kartAccelerationRate * static_cast<float>(deltaTime);
		karts[currentKartIndex].kartAcceleration = std::max(karts[currentKartIndex].kartAcceleration, -kartMaxSpeed / 2.0f);
	}

	// Actualizează poziția kart-ului curent
	karts[currentKartIndex].position += kartDirection * karts[currentKartIndex].kartAcceleration * static_cast<float>(deltaTime);
	karts[currentKartIndex].position.y = karts[currentKartIndex].position.y > 0 ? karts[currentKartIndex].position.y - 0.1f : 0.0f;
	karts[hitKartIndex].position.y = karts[hitKartIndex].position.y < 0 ? karts[hitKartIndex].position.y + 0.1f : 0.0f;

	// Rotație la stânga
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		float rotationFactor = (karts[currentKartIndex].kartAcceleration >= 0.0f) ? 1.0f : -1.0f; // Schimbă sensul rotației
		karts[currentKartIndex].rotationAngle += kartRotationSpeed * rotationFactor * static_cast<float>(deltaTime);
		karts[currentKartIndex].kartSteeringAngle += rotationFactor;
		if (karts[currentKartIndex].kartSteeringAngle > 10.0f) {
			karts[currentKartIndex].kartSteeringAngle = 10.0f;
		}
	}

	// Rotație la dreapta
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		float rotationFactor = (karts[currentKartIndex].kartAcceleration >= 0.0f) ? 1.0f : -1.0f; // Schimbă sensul rotației
		karts[currentKartIndex].rotationAngle -= kartRotationSpeed * rotationFactor * static_cast<float>(deltaTime);
		karts[currentKartIndex].kartSteeringAngle -= rotationFactor;
		if (karts[currentKartIndex].kartSteeringAngle < -10.0f) {
			karts[currentKartIndex].kartSteeringAngle = -10.0f;
		}
	}
	
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		pCamera->SetCameraMode(CameraMode::FREE);  // Mișcare libera a camerei

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		pCamera->SetCameraMode(CameraMode::FIRST_PERSON);  // Modul first person
	}

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		pCamera->SetCameraMode(CameraMode::THIRD_PERSON);  // Modul third person
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) // Schimbă kart-ul "înainte"
	{
		lastKartIndex = currentKartIndex;
		if (currentKartIndex > 0)
			currentKartIndex--;
		else
			currentKartIndex = karts.size() - 1;

		karts[currentKartIndex].mass += .4f;
		karts[lastKartIndex].mass -= .4f;
		glfwWaitEventsTimeout(1);
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // Schimbă kart-ul "înapoi"
	{
		lastKartIndex = currentKartIndex;
		if (currentKartIndex < karts.size() - 1)
			currentKartIndex++;
		else
			currentKartIndex = 0;

		karts[currentKartIndex].mass += .4f;
		karts[lastKartIndex].mass -= .4f;
		glfwWaitEventsTimeout(1);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) // Resetează camera
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}

	if (karts[currentKartIndex].kartSteeringAngle > 0.0f) {
		karts[currentKartIndex].kartSteeringAngle -= 10.0f * (float)deltaTime;
		if (karts[currentKartIndex].kartSteeringAngle < 0.0f) {
			karts[currentKartIndex].kartSteeringAngle = 0.0f;
		}
	}
	else if (karts[currentKartIndex].kartSteeringAngle < 0.0f) {
		karts[currentKartIndex].kartSteeringAngle += 10.0f * (float)deltaTime;
		if (karts[currentKartIndex].kartSteeringAngle > 0.0f) {
			karts[currentKartIndex].kartSteeringAngle = 0.0f;
		}
	}
}

void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string, Model>& models, std::array<Kart, 12>& karts, std::shared_ptr<SkyBox> skyBoxInstance)
{
	RenderSkybox(shaderManager.GetShader("skyboxShader"), skyBoxInstance);
	RenderKarts(shaderManager.GetShader("lightingWithTextureShader"), karts);
	RenderTrack(shaderManager.GetShader("lightingWithTextureShader"), models.at("track"));
	RenderTerrain(shaderManager.GetShader("lightingWithTextureShader"), models.at("terrain"));
	RenderSun(shaderManager.GetShader("sunShader"), models.at("sun"), sunPos);
}

void RenderSkybox(Shader& shader, std::shared_ptr<SkyBox> skyBoxInstance)
{
	shader.use();
	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
	skyBoxInstance->Render();
}

void RenderKarts(Shader& shader, std::array<Kart, 12>& karts)
{
	// Setează shader-ul o singură dată
	shader.use();

	// Setează valori care nu se schimbă
	shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("lightPos", lightPos);
	shader.SetVec3("viewPos", pCamera->GetPosition());
	shader.setInt("texture_diffuse1", 0);

	// Setează matricea de proiecție și vizualizare doar dacă este necesar
	static glm::mat4 lastProjectionMatrix;
	static glm::mat4 lastViewMatrix;
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glm::mat4 view = pCamera->GetViewMatrix();

	if (projection != lastProjectionMatrix) {
		shader.setMat4("projection", projection);
		lastProjectionMatrix = projection;
	}

	if (view != lastViewMatrix) {
		shader.setMat4("view", view);
		lastViewMatrix = view;
	}

	// Setează kart-ul curent
	karts[currentKartIndex].isCurrentPlayer = true;

	// Resetează kart-urile anterioare
	if (currentKartIndex != lastKartIndex && lastKartIndex >= 0)
		karts[lastKartIndex].isCurrentPlayer = false;

	// Calculează transformările pentru fiecare kart
	for (Kart& kart : karts) 
	{
		glm::mat4 kartModel = glm::mat4(1.0f);
		kartModel = glm::translate(kartModel, kart.position);
		kartModel = glm::rotate(kartModel, glm::radians(kart.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		kartModel = glm::scale(kartModel, glm::vec3(0.06f));

		// Actualizează poziția camerei pentru kart-ul curent
		if (kart.isCurrentPlayer)
		{
			glm::vec3 kartForwardVector = glm::normalize(glm::vec3(kartModel[2]));
			pCamera->UpdateKartPosition(kart.position, kartForwardVector);
		}
		shader.use();

		glm::mat4 frontWheelSteer = glm::rotate(kartModel, glm::radians(kart.kartSteeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		kart.kartModel.setNodeTransforms("obj5", frontWheelSteer);

		// Desenează kart-ul
		shader.setMat4("model", kartModel);
		kart.kartModel.Draw(shader);

		// Desenează pilotul pentru kart-ul curent
		if (kart.isCurrentPlayer) 
		{
			kart.pilotModel->UpdatePosition(kart.position, kart.rotationAngle);
			glm::mat4 pilotModel = glm::mat4(1.0f);
			pilotModel = glm::translate(pilotModel, glm::vec3(0.0f, 1.2f, 0.0f)); // Poziționează pilotul
			pilotModel = kartModel * pilotModel; // Aplică rotația kart-ului la pilot
			pilotModel = glm::scale(pilotModel, glm::vec3(0.02f)); // Ajustează scala pilotului
			shader.setMat4("model", pilotModel);
			kart.pilotModel->Draw(shader);
		}
	}
}

void RenderTerrain(Shader& shader, Model& model)
{
	// Verificăm dacă matricele de proiecție și vizualizare s-au schimbat
	static glm::mat4 lastProjectionMatrix;
	static glm::mat4 lastViewMatrix;
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glm::mat4 view = pCamera->GetViewMatrix();

	if (projection != lastProjectionMatrix) {
		shader.setMat4("projection", projection);
		lastProjectionMatrix = projection;
	}

	if (view != lastViewMatrix) {
		shader.setMat4("view", view);
		lastViewMatrix = view;
	}

	// Setările care nu se schimbă frecvent
	static bool isShaderUsed = false;
	if (!isShaderUsed) {
		shader.use();  // Folosește shader-ul o singură dată
		shader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f); // Culoare fixă pentru teren
		shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.SetVec3("lightPos", lightPos);
		shader.SetVec3("viewPos", pCamera->GetPosition());
		shader.setInt("texture_diffuse1", 0); // Textura (presupunând că aceasta este constantă)
		isShaderUsed = true;
	}

	// Modelul terenului
	glm::mat4 terrainModel = glm::mat4(1.0f);
	terrainModel = glm::scale(terrainModel, glm::vec3(0.1f)); // Ajustează scala terenului
	terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -3800.0f, 0.0f)); // Poziționează terenul
	shader.setMat4("model", terrainModel);

	// Desenează terenul
	model.Draw(shader);
}

void LoadMultipleKarts(std::array<Kart, 12>& karts, std::unordered_map<std::string, Model>& models, std::shared_ptr<Pilot> pilot)
{
	std::vector<std::pair<glm::vec3, float>> kartConfigs = {
		{{0.0f, 0.0f, 0.0f}, 0.0f},  {{34.0f, 0.0f, 0.0f}, 0.0f},  {{0.0f, 0.0f, 50.0f}, 180.0f},
		{{34.0f, 0.0f, 50.0f}, 180.0f}, {{116.0f, 0.0f, 0.0f}, 0.0f}, {{150.0f, 0.0f, 0.0f}, 0.0f},
		{{116.0f, 0.0f, 50.0f}, 180.0f}, {{150.0f, 0.0f, 50.0f}, 180.0f}, {{215.0f, 0.0f, 0.0f}, 0.0f},
		{{249.0f, 0.0f, 0.0f}, 0.0f}, {{215.0f, 0.0f, 50.0f}, 180.0f}, {{249.0f, 0.0f, 50.0f}, 180.0f}
	};

	for (size_t i = 0; i < kartConfigs.size(); ++i) {
		Kart kart;
		kart.position = kartPos + kartConfigs[i].first;
		kart.rotationAngle = kartRotationAngle + kartConfigs[i].second;
		kart.kartModel = models.at("go_kart");
		kart.pilotModel = pilot;
		kart.kartAcceleration = kartAcceleration;
		kart.index = i;
		kart.mass = .8f + random_floatant_number();
		karts[i] = kart;
	}
}

bool checkCollision(const Kart& kart1, const Kart& kart2)
{
	// Calculăm centrul sferei de coliziune pentru fiecare kart
	glm::vec3 kart1Center = kart1.position + kart1.kartModel.boundingSphereCenter;
	glm::vec3 kart2Center = kart2.position + kart2.kartModel.boundingSphereCenter;

	// Ajustăm razele (dacă este necesar)
	float kart1Radius = std::min(kart1.kartModel.boundingSphereRadius, maxCollisionDistance);
	float kart2Radius = std::min(kart2.kartModel.boundingSphereRadius, maxCollisionDistance);

	// Calculăm distanța dintre centrele celor două sfere
	float distance = glm::distance(kart1Center, kart2Center);

	// Verificăm coliziunea
	return distance <= (kart1Radius + kart2Radius);
}

void handleCollision(Kart& kart1, Kart& kart2) {
	// Presupunem că fiecare kart are o masă constantă (de exemplu 1.0f)
	float mass1 = kart1.mass;
	float mass2 = kart2.mass;

	// factor de frânare 
	float brakingFactor = 0.6f;  // Ajustează această valoare pentru un efect mai puternic
	kart1.kartDirection *= brakingFactor;
	kart2.kartDirection *= brakingFactor;

	// Calculăm viteza curentă pe baza accelerației și direcției
	glm::vec3 velocity1 = kart1.kartDirection * kart1.kartAcceleration;
	glm::vec3 velocity2 = kart2.kartDirection * kart2.kartAcceleration;

	// Momentul fiecărui kart
	glm::vec3 momentum1 = mass1 * velocity1;
	glm::vec3 momentum2 = mass2 * velocity2;

	// Conservarea momentului: calculăm viteza finală combinată
	glm::vec3 totalMomentum = momentum1 + momentum2;

	// Coeficient de coliziune elastică
	float restitution = 0.8f; // Coeficient de restaurare (elasticitate)

	// Aplicați efectul coliziunii asupra fiecărei viteze
	glm::vec3 newVelocity1 = totalMomentum / mass1 * restitution;
	glm::vec3 newVelocity2 = totalMomentum / mass2 * restitution;

	// Actualizează direcțiile karturilor
	kart1.kartDirection = glm::normalize(newVelocity1);
	kart2.kartDirection = glm::normalize(newVelocity2);

	// Actualizează accelerațiile (viteza)
	kart1.kartAcceleration = std::min(glm::length(newVelocity1), kartMaxSpeed);
	kart2.kartAcceleration = std::min(glm::length(newVelocity2), kartMaxSpeed);

	// Adăugarea distanțării pentru a evita suprapunerea completă
	glm::vec3 separationVector = kart1.position - kart2.position;
	float distance = glm::length(separationVector);
	float maxCollisionDistance1 = 2.1 * maxCollisionDistance;
	float minDistance = std::min(kart1.kartModel.boundingSphereRadius + kart2.kartModel.boundingSphereRadius, maxCollisionDistance1);

	if (distance < minDistance) {
		glm::vec3 separation = glm::normalize(separationVector);
		glm::vec3 combinedDirection = glm::normalize(kart1.kartDirection + kart2.kartDirection);
		separation = glm::normalize(separation + combinedDirection) * (minDistance - distance);

		// Separă karturile pentru a evita suprapunerea
		kart1.position += separation * 0.5f; // Separă kartul 1
		kart2.position -= separation * 0.5f; // Separă kartul 2

		float rotationFactor = 5.0f;  // Factor de rotație

		// Rotirea kartului care a suferit coliziunea
		if (!kart1.isCurrentPlayer) {
			float dotProduct1 = glm::dot(kart1.kartDirection, separation);
			float dotProduct2 = glm::dot(kart2.kartDirection, separation);
			if (dotProduct1 > 0.0f) {
				kart1.rotationAngle += glm::abs(dotProduct1) * rotationFactor;
				kart2.rotationAngle -= glm::abs(dotProduct1) * rotationFactor / 2;
			}
			else {
				kart1.rotationAngle -= glm::abs(dotProduct1) * rotationFactor;
				kart2.rotationAngle += glm::abs(dotProduct1) * rotationFactor / 2;
			}

			glm::vec3 kartDirection = glm::vec3(
				sin(glm::radians(kart1.rotationAngle)),
				0.0f,
				cos(glm::radians(kart1.rotationAngle))
			);
			kart1.kartDirection = glm::normalize(kartDirection);
			hitKartIndex = kart1.index;

			kartDirection = glm::vec3(
				sin(glm::radians(kart2.rotationAngle)),
				0.0f,
				cos(glm::radians(kart2.rotationAngle))
			);

			kart2.kartDirection = glm::normalize(kartDirection);
		}
		else
		{
			float dotProduct2 = glm::dot(kart2.kartDirection, separation);
			float dotProduct1 = glm::dot(kart1.kartDirection, separation);
			if (dotProduct2 > 0.0f) {
				kart2.rotationAngle += glm::abs(dotProduct2) * rotationFactor;
				kart1.rotationAngle -= glm::abs(dotProduct2) * rotationFactor / 2;
			}
			else {
				kart2.rotationAngle -= glm::abs(dotProduct2) * rotationFactor;
				kart1.rotationAngle += glm::abs(dotProduct2) * rotationFactor / 2;
			}

			glm::vec3 kartDirection = glm::vec3(
				sin(glm::radians(kart2.rotationAngle)),
				0.0f,
				cos(glm::radians(kart2.rotationAngle))
			);

			kart2.kartDirection = glm::normalize(kartDirection);
			hitKartIndex = kart2.index;

			kartDirection = glm::vec3(
				sin(glm::radians(kart1.rotationAngle)),
				0.0f,
				cos(glm::radians(kart1.rotationAngle))
			);

			kart1.kartDirection = glm::normalize(kartDirection);
		}
	}
}

float random_floatant_number()
{
	std::random_device rd;
	std::mt19937 gen(rd());  
	std::uniform_real_distribution<> dis(0.0, 2.0);

	return dis(gen);
}

void loadSounds() {
	// Listează fișierele de sunet împreună cu numele asociate
	std::map<std::string, std::string> soundFiles = {
		{"kart_acceleration",currentPath + "\\assets\\kart_acceleration.wav"},
		{"kart_running_engine",currentPath + "\\assets\\kart_running_engine.wav"},
		{"kart_crash",currentPath + "\\assets\\kart_crash.wav"}
	};

	// Încarcă fiecare fișier de sunet în hărți
	for (const auto& it : soundFiles) {
		sf::SoundBuffer buffer;
		if (!buffer.loadFromFile(it.second)) {
			std::cerr << "Nu s-a putut incarca " << it.second<< "!" << std::endl;
			continue; // Treci la următorul fișier
		}

		// Adaugă buffer-ul în map
		soundBuffers[it.first] = buffer;

		// Creează sunetul și setează buffer-ul
		sf::Sound sound;
		sound.setBuffer(soundBuffers[it.first]);

		// Adaugă sunetul în map
		sounds[it.first] = sound;
	}

	sounds["kart_crash"].setVolume(50.0f);
	sounds["kart_running_engine"].setVolume(190.0f);
	sounds["kart_acceleration"].setVolume(50.0f);
}

void playSound(const std::string& soundName) {
	// Verifică dacă sunetul există în map și îl redă
	if (sounds.find(soundName) != sounds.end()) {
		sounds[soundName].play();
	}
	else {
		std::cerr << "Sunetul \"" << soundName << "\" nu exista!" << std::endl;
	}
}

void stopSound(const std::string& soundName) {
	// Oprește sunetul dacă există
	if (sounds.find(soundName) != sounds.end()) {
		sounds[soundName].stop();
	}
	else {
		std::cerr << "Sunetul \"" << soundName << "\" nu exista!" << std::endl;
	}
}
void RenderTrack(Shader& shader, Model& model)
{
	// Verificăm dacă matricele de proiecție și vizualizare s-au schimbat
	static glm::mat4 lastProjectionMatrix;
	static glm::mat4 lastViewMatrix;
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glm::mat4 view = pCamera->GetViewMatrix();

	if (projection != lastProjectionMatrix) {
		shader.setMat4("projection", projection);
		lastProjectionMatrix = projection;
	}

	if (view != lastViewMatrix) {
		shader.setMat4("view", view);
		lastViewMatrix = view;
	}

	// Setările care nu se schimbă frecvent
	static bool isShaderUsed = false;
	if (!isShaderUsed) {
		shader.use();  // Folosește shader-ul o singură dată
		shader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f); // Culoare fixă pentru pistă
		shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.SetVec3("lightPos", lightPos);
		shader.SetVec3("viewPos", pCamera->GetPosition());
		shader.setInt("texture_diffuse1", 0); // Textura (presupunând că aceasta este constantă)
		isShaderUsed = true;
	}

	// Modelul pistei
	glm::mat4 trackModel = glm::mat4(1.0f);
	trackModel = glm::scale(trackModel, glm::vec3(200.0f)); // Ajustează scala pistei
	trackModel = glm::translate(trackModel, glm::vec3(0.0f, -0.008f, 0.0f)); // Poziționează pista
	shader.setMat4("model", trackModel);

	// Desenează pista
	model.Draw(shader);
}

void RenderSunDepth(Shader& depthShader)
{
	// Poziția soarelui (ce vrei tu)
	glm::vec3 sunPosition(15.0f, 25.0f, -15.0f);

	// Matrice model
	glm::mat4 model(1.0f);
	model = glm::translate(model, sunPosition);
	model = glm::scale(model, glm::vec3(2.0f)); // mărimea soarelui

	// Trimitem către shader
	depthShader.setMat4("model", model);

	// Desenăm sfera/cubul soarelui (vezi pasul 3 pentru mesh)
	glBindVertexArray(cubeVAO); // sau sphereVAO
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void RenderSun(Shader& shader, Model& sunModel, const glm::vec3& sunPos)
{
	// Make sure we use the shader!
	shader.use();

	// Projection & view:
	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", pCamera->GetViewMatrix());

	// Basic uniform setup:
	shader.SetVec3("objectColor", 1.0f, 1.0f, 0.0f); // bright yellow
	shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("lightPos", lightPos);
	shader.SetVec3("viewPos", pCamera->GetPosition());

	// If you have a diffuse texture, set it to 0:
	shader.setInt("texture_diffuse1", 0);

	// Model matrix for the sun
	glm::mat4 modelSun(1.0f);
	// Place it where you want, e.g. at sunPos:
	modelSun = glm::translate(modelSun, sunPos);
	// Scale it to something visible
	modelSun = glm::scale(modelSun, glm::vec3(0.02f));
	// or bigger if you want...
	shader.setMat4("model", modelSun);

	// Finally draw the sun
	sunModel.Draw(shader);
}

void RenderShadowPass(Shader& depthShader,
	unsigned int depthMapFBO,
	unsigned int SHADOW_WIDTH,
	unsigned int SHADOW_HEIGHT,
	const glm::mat4& lightSpaceMatrix,
	std::unordered_map<std::string, Model>& models,
	std::array<Kart, 12>& karts)
{
	// 1) Bind the depth FBO and set viewport to shadow-map size
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// 2) Use the depth shader
	depthShader.use();
	// Send the lightSpaceMatrix uniform
	depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	// (a) Render the track
	{
		// Model matrix for the track
		glm::mat4 trackModel = glm::mat4(1.0f);
		trackModel = glm::scale(trackModel, glm::vec3(200.0f));
		trackModel = glm::translate(trackModel, glm::vec3(0.0f, -0.008f, 0.0f));

		depthShader.setMat4("model", trackModel);
		models.at("track").Draw(depthShader);
	}

	// (b) Render the terrain
	{
		glm::mat4 terrainModel = glm::mat4(1.0f);
		terrainModel = glm::scale(terrainModel, glm::vec3(0.1f));
		terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -3800.0f, 0.0f));

		depthShader.setMat4("model", terrainModel);
		models.at("terrain").Draw(depthShader);
	}

	// (c) Render the sun itself (if you want it to cast shadow)
	// Usually we don't cast shadows from the sun sphere, but up to you:
	{
		glm::mat4 sunModelMat = glm::mat4(1.0f);
		// Put the sun at (sunPos) and scale it up if needed
		// For example:
		// sunModelMat = glm::translate(sunModelMat, sunPos);
		// sunModelMat = glm::scale(sunModelMat, glm::vec3(5.0f));
		//
		// But often we skip the sun geometry in the depth pass.
	}

	// (d) Render the karts
	for (size_t i = 0; i < karts.size(); ++i)
	{
		auto& kart = karts[i];

		// Build the same model matrix you use in your main pass
		glm::mat4 kartModel = glm::mat4(1.0f);
		kartModel = glm::translate(kartModel, kart.position);
		kartModel = glm::rotate(kartModel, glm::radians(kart.rotationAngle),
			glm::vec3(0.0f, 1.0f, 0.0f));
		kartModel = glm::scale(kartModel, glm::vec3(0.06f));

		depthShader.setMat4("model", kartModel);
		kart.kartModel.Draw(depthShader);

		// If you want the pilot to cast a shadow:
		glm::mat4 pilotModelMat = glm::mat4(1.0f);
		pilotModelMat = glm::translate(pilotModelMat, glm::vec3(0.0f, 1.2f, 0.0f));
		pilotModelMat = kartModel * pilotModelMat;
		pilotModelMat = glm::scale(pilotModelMat, glm::vec3(0.02f));
		depthShader.setMat4("model", pilotModelMat);
		kart.pilotModel->Draw(depthShader);
	}

	// 3) Unbind so we can go back to our normal pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

