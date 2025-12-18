#pragma once

#include "Core.h"
#include "Mesh.h"
#include "PSO.h"
#include "Shaders.h"
#include "Animation.h"
#include "TextureData.h"


class Sphere {
public:
	Mesh mesh;
	std::string shaderName;
	TextureManager* texture_manager;
	STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv)
	{
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		Frame frame;
		frame.fromVector(n);
		v.tangent = frame.u;
		v.tu = tu;
		v.tv = tv;
		return v;
	}
	int rings;
	int segments;
	float radius;
	void init(Core* core, PSOManager* psos, Shaders* shaders, int rings, int segments, float radius, TextureManager* tex_man = nullptr) {
		std::vector<STATIC_VERTEX> vertices;
		for (int lat = 0; lat <= rings; lat++) {
			float theta = lat * M_PI / rings;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			for (int lon = 0; lon <= segments; lon++) {
				float phi = lon * 2.0f * M_PI / segments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta,
					radius * sinTheta * sinPhi);
				Vec3 normal = position.normalize();
				float tu = 1.0f - (float)lon / segments;
				float tv = 1.0f - (float)lat / rings;
				vertices.push_back(addVertex(position, normal, tu, tv));
			}
		}
		std::vector<unsigned int> indices;
		for (int lat = 0; lat < rings; lat++)
		{
			for (int lon = 0; lon < segments; lon++)
			{
				int current = lat * (segments + 1) + lon;
				int next = current + segments + 1;
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}
		mesh.init(core, vertices, indices);
		if (true) {
			shaders->load(core, "StaticModelTextured", "VS.txt", "PS.txt");
		}
		else {
			shaders->load(core, "StaticModelTextured", "VS.txt", "PSUntextured.txt");
		}
		texture_manager = tex_man;
		shaderName = "StaticModelTextured";
		psos->createPSO(core, "StaticModelTexturedPSO", shaders->find("StaticModelTextured")->vs, shaders->find("StaticModelTextured")->ps, VertexLayoutCache::getStaticLayout());
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, Matrix& vp)
	{
		shaders->updateConstantVS("StaticModelTextured", "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, shaderName);
		psos->bind(core, "StaticModelTexturedPSO");
		if (texture_manager) {
			shaders->updateTexturePS(core, "StaticModelTextured", "tex", texture_manager->find("Models/Textures/qwantani_moon_noon_puresky.jpg"));
		}
		mesh.draw(core);
	}
};

class Plane
{
public:
	Mesh mesh;
	std::string shaderName;
	STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv)
	{
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		Frame frame;
		frame.fromVector(n);
		v.tangent = frame.u;
		v.tu = tu;
		v.tv = tv;
		return v;
	}
	void init(Core* core, PSOManager* psos, Shaders* shaders/*, Textures* textures*/)
	{
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(Vec3(-10000, -1, -10000), Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(Vec3(10000, -1, -10000), Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(Vec3(-10000, -1, 10000), Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(Vec3(10000, -1, 10000), Vec3(0, 1, 0), 1, 1));
		std::vector<unsigned int> indices;
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(2);
		mesh.init(core, vertices, indices);
		shaders->load(core, "StaticModelUntextured", "VS.txt", "PSUntextured.txt");
		shaderName = "StaticModelUntextured";
		psos->createPSO(core, "StaticModelUntexturedPSO", shaders->find("StaticModelUntextured")->vs, shaders->find("StaticModelUntextured")->ps, VertexLayoutCache::getStaticLayout());
		//textures->load(core, "Textures/Ground.jpg", "Textures/Ground.jpg");
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, Matrix& vp)
	{
		Matrix planeWorld;
		shaders->updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "VP", &vp);
		shaders->updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "W", &planeWorld);
		shaders->apply(core, shaderName);
		psos->bind(core, "StaticModelUntexturedPSO");
		mesh.draw(core);
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders/*, Textures* textures*/)
	{
		//shaders->updateTexturePS(core, "StaticModel", "tex", textures->find("Textures/Ground.jpg"));
		shaders->apply(core, shaderName);
		psos->bind(core, "StaticModelUntexturedPSO");
		mesh.draw(core);
	}
};

class StaticModel
{
public:
	std::vector<Mesh*> meshes;
	std::vector<std::string> textureFilenames;
	TextureManager* texture_manager;
	bool is_instanced = false;
	Vec3 min_point{ INFINITY, INFINITY, INFINITY };
	Vec3 max_point{ -INFINITY, -INFINITY, -INFINITY };
	void load(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* tex_man = nullptr, std::vector<Matrix> matrices = {})
	{
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		loader.load(filename, gemmeshes);
		for (int i = 0; i < gemmeshes.size(); i++)
		{
			Mesh* mesh = new Mesh();
			std::vector<STATIC_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++)
			{
				STATIC_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
				vertices.push_back(v);

				for (int k = 0; k < 3; k++) {
					if (v.pos.coords[k] < min_point.coords[k]) {
						min_point.coords[k] = v.pos.coords[k];
					}
					if (v.pos.coords[k] > max_point.coords[k]) {
						max_point.coords[k] = v.pos.coords[k];
					}
				}
			}
			textureFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
			//textureFilenames.push_back(gemmeshes[i].material.find("normal").getValue());
			if (matrices.size() == 0) {
				mesh->init(core, vertices, gemmeshes[i].indices);
			}
			else {
				mesh->init(core, vertices, gemmeshes[i].indices, matrices);
			}
			meshes.push_back(mesh);
		}
		if (matrices.size() != 0) {
			shaders->load(core, "StaticModelInstanced", "VSInstancing.txt", "PS.txt");
			psos->createPSO(core, "StaticModelInstancedPSO", shaders->find("StaticModelInstanced")->vs, shaders->find("StaticModelInstanced")->ps, VertexLayoutCache::getInstancedLayout());
			texture_manager = tex_man;
			is_instanced = true;
			return;
		}
		if (true) {
			shaders->load(core, "StaticModelTextured", "VS.txt", "PS.txt");
		}
		else {
			shaders->load(core, "StaticModelTextured", "VS.txt", "PSUntextured.txt");
		}
		psos->createPSO(core, "StaticModelPSO", shaders->find("StaticModelTextured")->vs, shaders->find("StaticModelTextured")->ps, VertexLayoutCache::getStaticLayout());
		texture_manager = tex_man;
	}
	void updateWorld(Shaders* shaders, Matrix& w)
	{
		shaders->updateConstantVS("StaticModelTextured", "staticMeshBuffer", "W", &w);
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, Matrix& vp)
	{
		if (is_instanced) {
			shaders->updateConstantVS("StaticModelInstanced", "staticMeshBuffer", "VP", &vp);
			Matrix W{};
			shaders->updateConstantVS("StaticModelInstanced", "staticMeshBuffer", "W", &W);
			shaders->apply(core, "StaticModelInstanced");
			psos->bind(core, "StaticModelInstancedPSO");
			for (int i = 0; i < meshes.size(); i++)
			{
				if (texture_manager) {
					shaders->updateTexturePS(core, "StaticModelInstanced", "tex", texture_manager->find(textureFilenames[i]));
				}
				meshes[i]->draw(core);
			}
		}
		else {
			shaders->updateConstantVS("StaticModelTextured", "staticMeshBuffer", "VP", &vp);
			shaders->apply(core, "StaticModelTextured");
			psos->bind(core, "StaticModelPSO");
			for (int i = 0; i < meshes.size(); i++)
			{
				if (texture_manager) {
					shaders->updateTexturePS(core, "StaticModelTextured", "tex", texture_manager->find(textureFilenames[i]));
				}
				meshes[i]->draw(core);
			}
		}
	}
};

class AnimatedModel
{
public:
	std::vector<Mesh*> meshes;
	Animation animation;
	TextureManager* texture_manager;
	std::vector<std::string> textureFilenames;
	void load(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* tex_man)
	{
		GEMLoader::GEMModelLoader loader;
		std::vector<GEMLoader::GEMMesh> gemmeshes;
		GEMLoader::GEMAnimation gemanimation;
		loader.load(filename, gemmeshes, gemanimation);
		for (int i = 0; i < gemmeshes.size(); i++)
		{
			Mesh* mesh = new Mesh();
			std::vector<ANIMATED_VERTEX> vertices;
			for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++)
			{
				ANIMATED_VERTEX v;
				memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
				vertices.push_back(v);
			}
			textureFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
			mesh->init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->load(core, "AnimatedUntextured", "VSAnim.txt", "PSAbnormal.txt");
		psos->createPSO(core, "AnimatedModelPSO", shaders->find("AnimatedUntextured")->vs, shaders->find("AnimatedUntextured")->ps, VertexLayoutCache::getAnimatedLayout());
		memcpy(&animation.skeleton.globalInverse, &gemanimation.globalInverse, 16 * sizeof(float));
		for (int i = 0; i < gemanimation.bones.size(); i++)
		{
			Bone bone;
			bone.name = gemanimation.bones[i].name;
			memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
			bone.parentIndex = gemanimation.bones[i].parentIndex;
			animation.skeleton.bones.push_back(bone);
		}
		for (int i = 0; i < gemanimation.animations.size(); i++)
		{
			std::string name = gemanimation.animations[i].name;
			AnimationSequence aseq;
			aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
			for (int j = 0; j < gemanimation.animations[i].frames.size(); j++)
			{
				AnimationFrame frame;
				for (int index = 0; index < gemanimation.animations[i].frames[j].positions.size(); index++)
				{
					Vec3 p;
					Quaternion q;
					Vec3 s;
					memcpy(&p, &gemanimation.animations[i].frames[j].positions[index], sizeof(Vec3));
					frame.positions.push_back(p);
					memcpy(&q, &gemanimation.animations[i].frames[j].rotations[index], sizeof(Quaternion));
					frame.rotations.push_back(q);
					memcpy(&s, &gemanimation.animations[i].frames[j].scales[index], sizeof(Vec3));
					frame.scales.push_back(s);
				}
				aseq.frames.push_back(frame);
			}
			animation.animations.insert({ name, aseq });
		}
		texture_manager = tex_man;
	}
	void updateWorld(Shaders* shaders, Matrix& w)
	{
		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "W", &w);
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, AnimationInstance* instance, Matrix& vp, Matrix& w)
	{
		psos->bind(core, "AnimatedModelPSO");
		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "W", &w);
		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "VP", &vp);
		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "bones", instance->matrices);
		shaders->apply(core, "AnimatedUntextured");
		for (int i = 0; i < meshes.size(); i++)
		{
			shaders->updateTexturePS(core, "AnimatedModel", "tex", texture_manager->find(textureFilenames[i]));
			meshes[i]->draw(core);
		}
	}
};