#pragma once

#include "Core.h"
#include "Mesh.h"
#include "PSO.h"
#include "Shaders.h"
#include "Animation.h"
#include "TextureData.h"


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
	Vec3 min_point{ INFINITY, INFINITY, INFINITY };
	Vec3 max_point{ -INFINITY, -INFINITY, -INFINITY };
	void load(Core* core, std::string filename, PSOManager* psos, Shaders* shaders, TextureManager* tex_man = nullptr)
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
			//textureFilenames.push_back(gemmeshes[i].material.find("normal").getValue());  Why is this not necessary?
			mesh->init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->load(core, "StaticModelTextured", "VS.txt", "PS.txt");
		psos->createPSO(core, "StaticModelPSO", shaders->find("StaticModelTextured")->vs, shaders->find("StaticModelTextured")->ps, VertexLayoutCache::getStaticLayout());
		texture_manager = tex_man;
	}
	void updateWorld(Shaders* shaders, Matrix& w)
	{
		shaders->updateConstantVS("StaticModelTextured", "staticMeshBuffer", "W", &w);
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, Matrix& vp)
	{
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
		shaders->load(core, "AnimatedUntextured", "VSAnim.txt", "PS.txt");
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