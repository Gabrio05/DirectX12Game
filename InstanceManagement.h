#pragma once
#include "ModelHolderClasses.h"
#include <vector>
#include <string>
#include <memory>
class Core;
class PSOManager;
class Shaders;
class TextureManager;

class ModelManager {
public:
	std::vector<std::unique_ptr<Sphere>> spheres{};
	std::vector<std::unique_ptr<Plane>> planes{};
	std::vector<std::unique_ptr<StaticModel>> static_models{};
	std::vector<std::unique_ptr<AnimatedModel>> animated_models{};
	std::vector<AnimationInstance> animated_instances{};

	void sphereInit(Core* core, PSOManager* psos, Shaders* shaders, int rings, int segments, float radius, TextureManager* tex_man = nullptr) {
		spheres.push_back(std::make_unique<Sphere>());
		spheres.back()->init(core, psos, shaders, rings, segments, radius, tex_man);
	}
	void sphereTextureLoad(Core* core, std::string tex_filename) {
		spheres.back()->texture_manager->load(core, tex_filename);
	}

	void planeInit(Core* core, PSOManager* psos, Shaders* shaders) {
		planes.push_back(std::make_unique<Plane>());
		planes.back()->init(core, psos, shaders);
	}

	void staticModelLoad(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* tex_man = nullptr) {
		static_models.push_back(std::make_unique<StaticModel>());
		static_models.back()->load(core, filename, psos, shaders, tex_man);
	}

	void staticModelTextureLoad(Core* core, std::string tex_filename) {
		static_models.back()->texture_manager->load(core, tex_filename);
	}

	void animatedModelLoad(Core* core, std::string filename, PSOManager* psos, 
						   Shaders* shaders, TextureManager* tex_manager, std::string tex_filename) {
		animated_models.push_back(std::make_unique<AnimatedModel>());
		animated_models.back()->load(core, filename, psos, shaders, tex_manager);
		animated_models.back()->texture_manager->load(core, tex_filename);
		AnimationInstance animatedInstance;
		animatedInstance.init(&animated_models.back()->animation, 0);
		animated_instances.push_back(animatedInstance);
	}

	void instanceModelLoad(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* tex_man = nullptr, std::vector<Matrix> matrices = {}) {
		static_models.push_back(std::make_unique<StaticModel>());
		static_models.back()->load(core, filename, psos, shaders, tex_man, matrices);
	}
};

inline Quaternion MakeRotationQuaternion(Vec3 axis, float angle)
{
	axis.normalize();
	float half = 0.5f * angle;
	float s = sinf(half);
	Quaternion q;
	q.a = axis.x * s;
	q.b = axis.y * s;
	q.c = axis.z * s;
	q.d = cosf(half);
	return q;
}

class InstanceManager {
public:
	ModelManager model_manager{};
	std::vector<Vec3> static_scaling_vectors{};
	std::vector<Vec3> static_offset_vectors{};
	std::vector<Quaternion> static_rotation_quaternions{};
	Matrix animated_world_matrix{};
	Core* core;
	PSOManager* psos;
	Shaders* shaders;
	TextureManager* tex_man;
	Matrix view_perspective_matrix{};

	InstanceManager(Core* _core, PSOManager* _psos, Shaders* _shaders, TextureManager* _tex_man) : 
		core{ _core }, psos{ _psos }, shaders{ _shaders }, tex_man{ _tex_man } {}
	void sphereInit(int rings, int segments, float radius, TextureManager* tex_man = nullptr) {
		model_manager.sphereInit(core, psos, shaders, rings, segments, radius, tex_man);
	}
	void planeInit() {
		model_manager.planeInit(core, psos, shaders);
	}
	void staticModelLoad(std::string filename, Vec3 scale, Vec3 offset, bool has_textures = false, Quaternion rotation = {}) {
		if (has_textures) {
			model_manager.staticModelLoad(core, filename, psos, shaders, tex_man);
		}
		else {
			model_manager.staticModelLoad(core, filename, psos, shaders);
		}
		static_scaling_vectors.push_back(scale);
		static_offset_vectors.push_back(offset);
		static_rotation_quaternions.push_back(rotation);
	}
	void animatedModelLoad(std::string filename, std::string tex_filename, Matrix world) {
		model_manager.animatedModelLoad(core, filename, psos, shaders, tex_man, tex_filename);
		animated_world_matrix = world;
	}
	void instanceModelLoad(std::string filename, Vec3 scale, Vec3 offset, bool has_textures = false, Quaternion rotation = {}, std::vector<Matrix> matrices = {}) {
		model_manager.instanceModelLoad(core, filename, psos, shaders, tex_man, matrices);
		static_scaling_vectors.push_back(scale);
		static_offset_vectors.push_back(offset);
		static_rotation_quaternions.push_back(rotation);
	}

	void sphereDraw(float time) {
		StaticModel* the_static_model = model_manager.static_models.at(1).get();
		Matrix W;
		W = W.scaling(Vec3(-1, -1, -1));
		W = Matrix::translation(Vec3(0, 0, 0)) * MakeRotationQuaternion(Vec3(0, 1, 0), time).toMatrix() * Matrix::translation(Vec3(0, 0, 0))
			* Matrix::scaling(Vec3(-1, -1, -1)) * Matrix::translation(Vec3(0, 0, 0));
		the_static_model->updateWorld(shaders, W);
		model_manager.spheres[0]->draw(core, psos, shaders, view_perspective_matrix);
	}
	void planeDraw() {
		model_manager.planes[0]->draw(core, psos, shaders, view_perspective_matrix);
	}
	void staticModelDraw(int i) {
		StaticModel* the_static_model = model_manager.static_models.at(i).get();
		Vec3 mid_point = (the_static_model->min_point + the_static_model->max_point) / 2.0f;
		Matrix W = Matrix::translation(-mid_point) * static_rotation_quaternions[i].toMatrix() * Matrix::translation(mid_point) 
			* Matrix::scaling(static_scaling_vectors[i]) * Matrix::translation(static_offset_vectors[i]);
		the_static_model->updateWorld(shaders, W);
		the_static_model->draw(core, psos, shaders, view_perspective_matrix);
	}
	void animatedModelDraw() {
		model_manager.animated_models.at(0)->draw(core, psos, shaders, &model_manager.animated_instances.at(0), view_perspective_matrix, animated_world_matrix);
	}
	void instanceModelDraw(int i) {
		model_manager.static_models.at(i)->draw(core, psos, shaders, view_perspective_matrix);
	}
};
