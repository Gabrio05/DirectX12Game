#pragma once
#include "ModelHolderClasses.h"
#include <vector>
#include <string>
#include <memory>
class Core;
class PSOManager;
class Shaders;
class TextureManager;

class InstanceManagement {
public:
	std::vector<std::unique_ptr<Plane>> planes{};
	std::vector<std::unique_ptr<StaticModel>> static_models{};
	std::vector<std::unique_ptr<AnimatedModel>> animated_models{};
	std::vector<AnimationInstance> animated_instances{};

	void planeInit(Core* core, PSOManager* psos, Shaders* shaders) {
		planes.push_back(std::make_unique<Plane>());
		planes.back()->init(core, psos, shaders);
	}

	void staticModelLoad(Core* core, std::string filename, PSOManager* psos, Shaders* shaders) {
		static_models.push_back(std::make_unique<StaticModel>());
		static_models.back()->load(core, filename, psos, shaders);
	}

	void animatedModelLoad(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* texture_manager) {
		animated_models.push_back(std::make_unique<AnimatedModel>());
		animated_models.back()->load(core, filename, psos, shaders, texture_manager);
		AnimationInstance animatedInstance;
		animatedInstance.init(&animated_models.back()->animation, 0);
		animated_instances.push_back(animatedInstance);
	}
};

