// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>
#include <filesystem>

namespace fs = std::filesystem;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Resources.h"
#include "Error.h"
#include "Console.h"
#include "IO.h"
#include "Material.h"
#include "Texture.h"
#include "Model.h"
#include "Scene.h"
#include "Image.h"
#include "Rasterizer.h"
#include "RayTracer.h"
#include "LightSource.h"
#include "SetupFreeNoise.h"
#include "Solid3DNoise.h"
#include "Texture2DNoise.h"

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;
static std::shared_ptr<Scene> scenePtr;
static std::shared_ptr<Rasterizer> rasterizerPtr;
static std::shared_ptr<RayTracer> rayTracerPtr;

// Camera control variables
static glm::vec3 center = glm::vec3 (0.0); // To update based on the mesh position
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static float setScale = 2.5f; 
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

// Files
static std::string basePath;
static std::string meshFilename;

// Raytraced rendering
static bool isDisplayRaytracing (false);

void clear ();
void initScene1();
void initScene2();
void initScene3();

void printHelp () {
	Console::print (std::string ("Help:\n") 
			  + "\tMouse commands:\n" 
			  + "\t* Left button: rotate camera\n" 
			  + "\t* Middle button: zoom\n" 
			  + "\t* Right button: pan camera\n" 
			  + "\tKeyboard commands:\n" 
   			  + "\t* ESC: quit the program\n"
   			  + "\t* H: print this help\n"
   			  + "\t* F12: reload GPU shaders\n"
   			  + "\t* F: decrease field of view\n"
   			  + "\t* G: increase field of view\n"
   			  + "\t* TAB: switch between rasterization and ray tracing display\n"
   			  + "\t* B: activate BVH\n"
   			  + "\t* N: deactivate BVH\n"
   			  + "\t* SPACE: execute ray tracing\n");
}

/// Adjust the ray tracer target resolution and runs it.
void raytrace() {
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	rayTracerPtr->setResolution (width, height);
	rayTracerPtr->render (scenePtr);
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_H) {
			printHelp ();
		} else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
		} else if (action == GLFW_PRESS && key == GLFW_KEY_F12) {
			rasterizerPtr->loadShaderProgram (basePath);
		} else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
			scenePtr->camera()->setFoV (std::max (5.f, scenePtr->camera()->getFoV () - 5.f));
		} else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
			scenePtr->camera()->setFoV (std::min (120.f, scenePtr->camera()->getFoV () + 5.f));
		} else if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
			isDisplayRaytracing = !isDisplayRaytracing;
		} else if (action == GLFW_PRESS && key == GLFW_KEY_B) {
			rayTracerPtr->activateBVH(true);
			Console::print("activate BVH");
		} else if (action == GLFW_PRESS && key == GLFW_KEY_N) {
			rayTracerPtr->activateBVH(false);
			Console::print("deactivate BVH");
		} else if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
			raytrace ();
		}
		else {
			printHelp ();
		}
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		scenePtr->camera()->setRotation (baseRot + dRot);
	} else if (isPanning) {
		scenePtr->camera()->setTranslation (baseTrans + 3 * setScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		scenePtr->camera()->setTranslation (baseTrans + 3 * setScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = scenePtr->camera()->getRotation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = scenePtr->camera()->getTranslation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = scenePtr->camera()->getTranslation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

/// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window. 
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
	scenePtr->camera()->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	rasterizerPtr->setResolution (width, height);
	rayTracerPtr->setResolution (width, height);
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		Console::print ("ERROR: Failed to init GLFW");
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint (GLFW_SAMPLES, 16); // Enable OpenGL multisampling (MSAA)

	// Create the window
	windowPtr = glfwCreateWindow (800, 600, BASE_WINDOW_TITLE.c_str (), nullptr, nullptr);
	if (!windowPtr) {
		Console::print ("ERROR: Failed to open window");
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	// Connect the callbacks for interactive control 
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback (windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void initScene1 () {
	Console::print("Scene1 : albedo textures generated using noise (planar uv for walls and spherical uv for others)");
	scenePtr = std::make_shared<Scene> ();
	scenePtr->setBackgroundColor (glm::vec3 (0.1f, 0.5f, 0.95f));

	// Meshes
	auto monkeyMeshPtr = IO::loadOFFMesh(meshFilename, 0, true);
	auto appleMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/Apple.off", 1, true);
	auto killerooMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/killeroo.off", 2, true);
	auto planMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/wall.off", 3, false);

	scenePtr->addMesh(monkeyMeshPtr);
	scenePtr->addMesh(appleMeshPtr);
	scenePtr->addMesh(killerooMeshPtr);
	scenePtr->addMesh(planMeshPtr);

	
	// ### Textures
	float K_ = 1.0;
	float a_ = 0.1;
	float F_0_ = 0.04;
	float omega_0_ = M_PI / 4.0;
	float number_of_impulses_per_kernel = 56.0;
	unsigned random_offset = std::time(0);
	prng gen;
	gen.seed(random_offset);
	
	Texture2Dnoise isotropic_Noise1(true, K_, a_ * 1.3f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));
	Texture2Dnoise isotropic_Noise2(true, K_, a_ * 1.3f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));
	Texture2Dnoise anisotropic_Noise1(false, K_, a_ * 1.3f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));
	Texture2Dnoise anisotropic_Noise2(false, K_, a_ * 1.3f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));
	
	std::vector<glm::vec3> colorMap1 = { glm::vec3(1.0, 1.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.1, 1.0) };
	std::vector<glm::vec3> colorMap2 = { glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.1, 0.0), glm::vec3(0.0, 0.0, 1.0) };
	std::vector<glm::vec3> colorMap3 = { glm::vec3(1.0 ,0.0, 1.0), glm::vec3(1.0 ,0.0, 0.0), glm::vec3(0.0 ,0.0, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0)};
	std::vector<glm::vec3> colorMap4 = { glm::vec3(0.99 ,0.98, 0.1), glm::vec3(0.95 ,0.93, 0.1), glm::vec3(0.89 ,0.82, 0.0), glm::vec3(0.7, 0.62, 0.1)};
	
	// Textures generated using gabor Noise
	Console::print("Generating textures and loading them to GPU using Gabor Noise");
	std::chrono::high_resolution_clock clock;
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	TextureBundle textureBundle11 = scenePtr->loadTextureBundle(256, isotropic_Noise1, colorMap1);
	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Wall Texture: resolution = 256, isotropic gabor noise generated and loaded in " + std::to_string(elapsedTime) + "ms");
	TextureBundle textureBundle12 = scenePtr->loadTextureBundle(256, anisotropic_Noise1, colorMap1);
	after = clock.now();
	elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ground Texture: resolution = 256, anisotropic gabor noise generated and loaded in " + std::to_string(elapsedTime) + "ms");
	before = clock.now();
	TextureBundle textureBundle2 = scenePtr->loadTextureBundle(256, isotropic_Noise2, colorMap2); 
	after = clock.now();
	elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Texture 2 : resolution = 256, isotropic gabor noise generated and loaded in " + std::to_string(elapsedTime) + "ms");
	before = clock.now();
	TextureBundle textureBundle3 = scenePtr->loadTextureBundle(256, anisotropic_Noise2, colorMap3);
	after = clock.now();
	elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Texture 3 : resolution = 256, anisotropic gabor noise generated and loaded in " + std::to_string(elapsedTime) + "ms");
	TextureBundle textureBundle4 = scenePtr->loadTextureBundle(256, isotropic_Noise2, colorMap4);
	after = clock.now();
	elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Texture 4 : resolution = 256, isotropic gabor noise generated and loaded in " + std::to_string(elapsedTime) + "ms");
	Console::print("Textures generation ended");

	// ---------------------------------
	
	// ### Materials
	auto materialPtr11 = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(1,1,0.0), 0.8, 0.05, textureBundle11);
	scenePtr->addMaterial(materialPtr11);
	auto materialPtr12 = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(1,1,0.0), 0.8, 0.05, textureBundle12);
	scenePtr->addMaterial(materialPtr12);
	auto materialPtr2 = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.2, 0.8), 0.1, 0.1, textureBundle2);
	scenePtr->addMaterial(materialPtr2);
	auto materialPtr3 = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(0.8, 1.0, 0.3), 0.1, 0.1, textureBundle3);
	scenePtr->addMaterial(materialPtr3);
	auto materialPtr4 = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(0.9, 0.8, 0.2), 0.8, 0.2, textureBundle4);
	scenePtr->addMaterial(materialPtr4);

	// ---------------------------------
	
	// ### Models
	auto wallPtr = std::make_shared<Model>();
	wallPtr->transform().setTranslation({ 0 , 0, -7 });
	wallPtr->transform().setScale(7);
	wallPtr->useMesh(planMeshPtr->getId());
	wallPtr->useMaterial(materialPtr11->getId());
	scenePtr->addModel(wallPtr);

	auto groundlPtr = std::make_shared<Model>();
	groundlPtr->transform().setTranslation({ 0 , -7, 0 });
	groundlPtr->transform().setRotation(glm::vec3(glm::radians(-90.0f), 0, 0));
	groundlPtr->transform().setScale(7);
	groundlPtr->useMesh(planMeshPtr->getId());
	groundlPtr->useMaterial(materialPtr12->getId());
	scenePtr->addModel(groundlPtr);
	
	for (int i = 0; i <= 4; i+=2) {
		for (int j = 0; j <= 4; j+=2) {
			auto modelPtr = std::make_shared<Model>();
			modelPtr->transform().setTranslation({ -4 + 2 * j, -4 + 2 * i, ((2 * i + j) % 3 == 0) ? -2.0f : ((2 * i + j) % 3 == 1) ? 2 : 0 });
			modelPtr->useMesh(((i * 4 + j) % 3 == 0) ? monkeyMeshPtr->getId() : (((i * 4 + j) % 3 == 1) ? appleMeshPtr->getId() : killerooMeshPtr->getId()));
			modelPtr->useMaterial(((i + j * 4) % 3 == 0)? materialPtr2->getId() : (((i*4+j) % 3 == 1)? materialPtr3->getId() : materialPtr4->getId()));
			modelPtr->transform().setScale(setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
			scenePtr->addModel(modelPtr);
		}
	}

	// ---------------------------------
	
	// ### Light
	auto pointLight1 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 70);
	pointLight1->transform().setTranslation(glm::vec3(-15, 1, 0));
	scenePtr->addLight(pointLight1);

	auto pointLight2 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 70);
	pointLight2->transform().setTranslation(glm::vec3(15, 1, 0));
	scenePtr->addLight(pointLight2);

	auto dirtectionalLight2 = std::make_shared<LightSource>(LightType::DirectionalLight, glm::vec3(1.0, 1.0, 1.0), 1.5f);
	dirtectionalLight2->transform().setRotation(glm::vec3(glm::radians(45.0f), glm::radians(180.0f), glm::radians(0.0f)));
	scenePtr->addLight(dirtectionalLight2);
	// ---------------------------------
	
	// Preprocess scene (apply model transforms & calculate AABB)  // should be called in render for dynamic scenes
	scenePtr->preprocessScene();
	// ---------------------------------

	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	auto cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 5.f * setScale));
	cameraPtr->setNear (0.1f);
	cameraPtr->setFar (100.f * setScale);
	scenePtr->set (cameraPtr);
	std::cout << "meshscale " << meshScale << std::endl;
}

void initScene2() {
	Console::print("Scene2 (Ray tracing should be activated) : texture(spherical uv) / surface / solid  ");
	scenePtr = std::make_shared<Scene>();
	scenePtr->setBackgroundColor(glm::vec3(0.1f, 0.5f, 0.95f));

	// Meshes
	auto planMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/wall.off", scenePtr->numOfMeshes(), false);
	scenePtr->addMesh(planMeshPtr);
	auto rhinoMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/rhino.off", scenePtr->numOfMeshes(), true);
	scenePtr->addMesh(rhinoMeshPtr);

	// ### Textures
	float K_ = 1.0;
	float a_ = 0.1;
	float F_0_ = 0.04;
	float omega_0_ = M_PI / 4.0;
	float number_of_impulses_per_kernel = 56.0;
	unsigned random_offset = std::time(0);
	prng gen;
	gen.seed(random_offset);


	Texture2Dnoise isotropic_Noise(true, K_, a_ * 2, F_0_ * 3, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));
	Texture2Dnoise anisotropic_Noise(false, K_, a_ * 2, F_0_ * 3, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999));

	std::vector<glm::vec3> colorMap = { glm::vec3(1.0 ,0.0, 1.0), glm::vec3(1.0 ,0.0, 0.0), glm::vec3(0.0 ,0.0, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0) };

	// Textures generated using gabor Noise
	Console::print("Generating textures and loading them to GPU using Gabor Noise");
	TextureBundle anisotropicTextureBundle = scenePtr->loadTextureBundle(256, anisotropic_Noise, colorMap);
	TextureBundle isotropicTextureBundle = scenePtr->loadTextureBundle(256, isotropic_Noise, colorMap);
	Console::print("Textures generation ended");
	// ---------------------------------

	// ### Materials
	TextureBundle textureBundleEmpty = { -1, -1, -1, -1 }; // used as a  control  sample
	auto controlMaterialPtr = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(0.7, 0.7, 0.7), 0.5, 0.5, textureBundleEmpty);
	scenePtr->addMaterial(controlMaterialPtr);
	
	auto isotropicTextureMaterialPtr = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.4, 0.4, isotropicTextureBundle);
	scenePtr->addMaterial(isotropicTextureMaterialPtr);
	auto anisotropicTextureMaterialPtr = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.4, 0.4, anisotropicTextureBundle);
	scenePtr->addMaterial(anisotropicTextureMaterialPtr);


	auto isotropicSurfaceMaterialPtr = std::make_shared<SurfaceNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.1, 0.1, textureBundleEmpty);
	isotropicSurfaceMaterialPtr->setGen(std::make_shared<SetupFreeNoise>(true, K_, a_ * 1.2f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999)));
	isotropicSurfaceMaterialPtr->addColor(colorMap);
	scenePtr->addMaterial(isotropicSurfaceMaterialPtr);
	auto anisotropicSurfaceMaterialPtr = std::make_shared<SurfaceNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.1, 0.1, textureBundleEmpty);
	anisotropicSurfaceMaterialPtr->setGen(std::make_shared<SetupFreeNoise>(false, K_, a_ * 1.2f, F_0_ * 4, omega_0_, number_of_impulses_per_kernel, gen.uniform(0, 99999999)));
	anisotropicSurfaceMaterialPtr->addColor(colorMap);
	scenePtr->addMaterial(anisotropicSurfaceMaterialPtr);
	
	auto isotropicSolidMaterialPtr = std::make_shared<SolidNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.1, 0.1, textureBundleEmpty);
	isotropicSolidMaterialPtr->setGen(std::make_shared<Solid3DNoise>(true, K_, a_ * 1.2f, F_0_ * 4, glm::vec3(1.0,0.0,0.0), number_of_impulses_per_kernel * 1.2f, gen.uniform(0, 99999999)));
	isotropicSolidMaterialPtr->addColor(colorMap);
	scenePtr->addMaterial(isotropicSolidMaterialPtr);
	auto anisotropicSolidMaterialPtr = std::make_shared<SolidNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.1, 0.1, textureBundleEmpty);
	anisotropicSolidMaterialPtr->setGen(std::make_shared<Solid3DNoise>(false, K_, a_ * 1.2f, F_0_ * 4, glm::vec3(1.0, 0.0, 0.0), number_of_impulses_per_kernel * 1.2f, gen.uniform(0, 99999999)));
	anisotropicSolidMaterialPtr->addColor(colorMap);
	scenePtr->addMaterial(anisotropicSolidMaterialPtr);

	// ---------------------------------

	// ### Models
	auto wallPtr = std::make_shared<Model>();
	wallPtr->transform().setTranslation({ 0 , 0, -6 });
	wallPtr->transform().setScale(6);
	wallPtr->useMesh(planMeshPtr->getId());
	wallPtr->useMaterial(controlMaterialPtr->getId());
	scenePtr->addModel(wallPtr);

	auto groundlPtr = std::make_shared<Model>();
	groundlPtr->transform().setTranslation({ 0 , -6, 0 });
	groundlPtr->transform().setRotation(glm::vec3(glm::radians(-90.0f), 0, 0));
	groundlPtr->transform().setScale(6);
	groundlPtr->useMesh(planMeshPtr->getId());
	groundlPtr->useMaterial(controlMaterialPtr->getId());
	scenePtr->addModel(groundlPtr);

	// Top Row : isotropic
	Console::print("(Top Left) :isotropic albedo noise texture resolution = 256, isotropic gabor noise ");
	auto modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ -3.2, 2, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4, 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(isotropicTextureMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	Console::print("(Top center) : isotropic surface noise");
	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 0, 2, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(isotropicSurfaceMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	Console::print("(Top Right) : isotropic solid noise");
	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 3.2, 2, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(isotropicSolidMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	// Bottom row : anistotropic
	
	Console::print("(Bottom Left) : anisotropic albedo noise texture resolution = 256, isotropic gabor noise ");
	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ -3.2, -2, 2 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(anisotropicTextureMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	Console::print("(Bottom center) : anisotropic surface noise");
	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 0, -2, 2 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(anisotropicSurfaceMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	Console::print("(Bottom Right) : anisotropic solid noise");
	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 3.2, -2, 2 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(anisotropicSolidMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	// ---------------------------------

	// ### Light
	auto pointLight1 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 150);
	pointLight1->transform().setTranslation(glm::vec3(-30, 0, 0));
	scenePtr->addLight(pointLight1);

	auto pointLight2 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 150);
	pointLight2->transform().setTranslation(glm::vec3(30, 0, 0));
	scenePtr->addLight(pointLight2);

	auto dirtectionalLight2 = std::make_shared<LightSource>(LightType::DirectionalLight, glm::vec3(1.0, 1.0, 1.0), 1.2f);
	dirtectionalLight2->transform().setRotation(glm::vec3(glm::radians(45.0f), glm::radians(180.0f), glm::radians(0.0f)));
	scenePtr->addLight(dirtectionalLight2);

	// Preprocess scene (apply model transforms & calculate AABB)  // should be called in render for dynamic scenes
	scenePtr->preprocessScene();
	// ---------------------------------

	// Camera
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	auto cameraPtr = std::make_shared<Camera>();
	cameraPtr->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	cameraPtr->setTranslation(center + glm::vec3(0.0, 0.0, 10.f * setScale));
	cameraPtr->setNear(0.1f);
	cameraPtr->setFar(100.f * setScale);
	scenePtr->set(cameraPtr);
	std::cout << "meshscale " << meshScale << std::endl;
}

void initScene3() {
	Console::print("Scene3 (Ray tracing should be activated) : albedo / roughness / metalicness  ");
	scenePtr = std::make_shared<Scene>();
	scenePtr->setBackgroundColor(glm::vec3(0.1f, 0.5f, 0.95f));

	// Meshes
	auto planMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/wall.off", scenePtr->numOfMeshes(), false);
	scenePtr->addMesh(planMeshPtr);
	auto rhinoMeshPtr = IO::loadOFFMesh(basePath + "/Resources/Models/rhino.off", scenePtr->numOfMeshes(), true);
	scenePtr->addMesh(rhinoMeshPtr);

	// ### Textures
	float K_ = 1.0;
	float a_ = 0.1;
	float F_0_ = 0.04;
	float omega_0_ = M_PI / 4.0;
	float number_of_impulses_per_kernel = 56.0;
	unsigned random_offset = std::time(0);
	prng gen;
	gen.seed(random_offset);


	std::vector<glm::vec3> colorMap = { glm::vec3(1.0 ,0.0, 1.0), glm::vec3(1.0 ,0.0, 0.0), glm::vec3(0.0 ,0.0, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0) };

	// ---------------------------------

	// ### Materials
	TextureBundle textureBundleEmpty = { -1, -1, -1, -1 }; // used as a  control  sample
	auto controlMaterialPtr = std::make_shared<Material>(scenePtr->numOfMaterials(), glm::vec3(0.7, 0.7, 0.7), 0.4, 0.4, textureBundleEmpty);
	scenePtr->addMaterial(controlMaterialPtr);

	auto albedo_isotropicSolidMaterialPtr = std::make_shared<SolidNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.4, 0.4, textureBundleEmpty);
	albedo_isotropicSolidMaterialPtr->setGen(std::make_shared<Solid3DNoise>(true, K_, a_ * 1.4f, F_0_ * 5, glm::vec3(1.0, 0.0, 0.0), number_of_impulses_per_kernel * 1.2f, gen.uniform(0, 99999999)));
	albedo_isotropicSolidMaterialPtr->addColor(colorMap);
	albedo_isotropicSolidMaterialPtr->setmask(true,false,false);
	scenePtr->addMaterial(albedo_isotropicSolidMaterialPtr);

	auto roughness_isotropicSolidMaterialPtr = std::make_shared<SolidNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.2, 0.1, textureBundleEmpty);
	roughness_isotropicSolidMaterialPtr->setGen(std::make_shared<Solid3DNoise>(true, K_, a_ * 1.4f, F_0_ * 5, glm::vec3(1.0, 0.0, 0.0), number_of_impulses_per_kernel * 1.2f, gen.uniform(0, 99999999)));
	roughness_isotropicSolidMaterialPtr->addColor(colorMap);
	roughness_isotropicSolidMaterialPtr->setmask(false, true, false);
	scenePtr->addMaterial(roughness_isotropicSolidMaterialPtr);

	auto metalic_isotropicSolidMaterialPtr = std::make_shared<SolidNoiseMaterial>(scenePtr->numOfMaterials(), glm::vec3(1.0, 0.5, 0.0), 0.1, 0.1, textureBundleEmpty);
	metalic_isotropicSolidMaterialPtr->setGen(std::make_shared<Solid3DNoise>(true, K_, a_ * 1.4f, F_0_ * 5, glm::vec3(1.0, 0.0, 0.0), number_of_impulses_per_kernel * 1.2f, gen.uniform(0, 99999999)));
	metalic_isotropicSolidMaterialPtr->addColor(colorMap);
	metalic_isotropicSolidMaterialPtr->setmask(false, false, true);
	scenePtr->addMaterial(metalic_isotropicSolidMaterialPtr);

	// ---------------------------------

	// ### Models
	auto wallPtr = std::make_shared<Model>();
	wallPtr->transform().setTranslation({ 0 , 0, -4 });
	wallPtr->transform().setScale(4);
	wallPtr->useMesh(planMeshPtr->getId());
	wallPtr->useMaterial(controlMaterialPtr->getId());
	scenePtr->addModel(wallPtr);

	auto groundlPtr = std::make_shared<Model>();
	groundlPtr->transform().setTranslation({ 0 , -4, 0 });
	groundlPtr->transform().setRotation(glm::vec3(glm::radians(-90.0f), 0, 0));
	groundlPtr->transform().setScale(4);
	groundlPtr->useMesh(planMeshPtr->getId());
	groundlPtr->useMaterial(controlMaterialPtr->getId());
	scenePtr->addModel(groundlPtr);



	auto modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ -2.4, 1, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(albedo_isotropicSolidMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 0, 1, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(roughness_isotropicSolidMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	modelPtr = std::make_shared<Model>();
	modelPtr->transform().setTranslation({ 2.4, 1, 0 });
	modelPtr->transform().setRotation({ 0, M_PI / 4 , 0 });
	modelPtr->useMesh(rhinoMeshPtr->getId());
	modelPtr->useMaterial(metalic_isotropicSolidMaterialPtr->getId());
	modelPtr->transform().setScale(1.5f * setScale / scenePtr->mesh(modelPtr->meshId())->getMeshScale());
	scenePtr->addModel(modelPtr);

	// ---------------------------------

	// ### Light
	auto pointLight1 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 150);
	pointLight1->transform().setTranslation(glm::vec3(-30, 0, 0));
	scenePtr->addLight(pointLight1);

	auto pointLight2 = std::make_shared<LightSource>(LightType::PointLight, glm::vec3(1, 1, 1), 150);
	pointLight2->transform().setTranslation(glm::vec3(30, 0, 0));
	scenePtr->addLight(pointLight2);

	auto dirtectionalLight2 = std::make_shared<LightSource>(LightType::DirectionalLight, glm::vec3(1.0, 1.0, 1.0), 1.2f);
	dirtectionalLight2->transform().setRotation(glm::vec3(glm::radians(45.0f), glm::radians(180.0f), glm::radians(0.0f)));
	scenePtr->addLight(dirtectionalLight2);

	// Preprocess scene (apply model transforms & calculate AABB)  // should be called in render for dynamic scenes
	scenePtr->preprocessScene();
	// ---------------------------------

	// Camera
	int width, height;
	glfwGetWindowSize(windowPtr, &width, &height);
	auto cameraPtr = std::make_shared<Camera>();
	cameraPtr->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	cameraPtr->setTranslation(center + glm::vec3(0.0, 0.0, 10.f * setScale));
	cameraPtr->setNear(0.1f);
	cameraPtr->setFar(100.f * setScale);
	scenePtr->set(cameraPtr);
	std::cout << "meshscale " << meshScale << std::endl;
}

void init () {
	initGLFW (); // Windowing system
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress)) // Load extensions for modern OpenGL
		exitOnCriticalError ("[Failed to initialize OpenGL context]");
	initScene1(); // Actual scene to render
	rasterizerPtr = make_shared<Rasterizer>();
	rasterizerPtr->init (basePath, scenePtr); // Mut be called before creating the scene, to generate an OpenGL context and allow mesh VBOs
	rayTracerPtr = make_shared<RayTracer>();
	rayTracerPtr->init (scenePtr);
}

void clear () {
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}


// The main rendering call
void render () {
	if (isDisplayRaytracing)
		rasterizerPtr->display (rayTracerPtr->image ());
	else
		rasterizerPtr->render (scenePtr);
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	static float lastTime = 0.f;
	static unsigned int frameCount = 0;
	static float fpsTime = currentTime;
	static unsigned int FPS = 0;
	float elapsedTime = currentTime - initialTime;
	float dt = currentTime - lastTime;
	if (frameCount == 99) {
		float delai = (currentTime - fpsTime)/100;
		FPS = static_cast<unsigned int> (1.f/delai);
		frameCount = 0;
		fpsTime = currentTime;
	}
	std::string titleWithFPS = BASE_WINDOW_TITLE + " - " + std::to_string (FPS) + "FPS";
	glfwSetWindowTitle (windowPtr, titleWithFPS.c_str ());
	lastTime = currentTime;
	frameCount++;
}

void usage (const char * command) {
	Console::print ("Usage : " + std::string(command) + " [<meshfile.off>]");
	std::exit (EXIT_FAILURE);
}

void parseCommandLine (int argc, char ** argv) {
	if (argc > 3)
		usage (argv[0]);
	fs::path appPath = argv[0];
	basePath = appPath.parent_path().string(); 
	meshFilename = basePath + "/" + (argc >= 2 ? argv[1] : DEFAULT_MESH_FILENAME);
}

int main (int argc, char ** argv) {
	parseCommandLine (argc, argv);
	init (); 
	while (!glfwWindowShouldClose (windowPtr)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	Console::print ("Quit");
	return EXIT_SUCCESS;
}

