#include "Core.h"
#include "Window.h"
#include "Timer.h"
#include "Maths.h"
#include "Mesh.h"
#include "PSO.h"
#include "Shaders.h"
#include "Animation.h"
#include "TextureData.h"
#include "HandleMovement.h"

// Properties -> Linker -> System -> Windows

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


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
	void init(Core* core, PSOManager *psos, Shaders* shaders/*, Textures* textures*/)
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
	void load(Core* core, std::string filename, Shaders* shaders, PSOManager* psos)
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
			}
			textureFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
			mesh->init(core, vertices, gemmeshes[i].indices);
			meshes.push_back(mesh);
		}
		shaders->load(core, "StaticModelUntextured", "VS.txt", "PS.txt");
		psos->createPSO(core, "StaticModelPSO", shaders->find("StaticModelUntextured")->vs, shaders->find("StaticModelUntextured")->ps, VertexLayoutCache::getStaticLayout());
	}
	void updateWorld(Shaders* shaders, Matrix& w)
	{
		shaders->updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "W", &w);
	}
	void draw(Core* core, PSOManager* psos, Shaders* shaders, Matrix& vp)
	{
		shaders->updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "VP", &vp);
		shaders->apply(core, "StaticModelUntextured");
		psos->bind(core, "StaticModelPSO");
		for (int i = 0; i < meshes.size(); i++)
		{
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
		texture_manager->load(core, "Models/Textures/T-rex_Base_Color_alb.png");
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

void updateAnimation(AnimationInstance* animated, std::string current_animation, float dt) {
	animated->update(current_animation, dt);
	if (animated->animationFinished()) {
		animated->resetAnimationTime();
	}
}



#define WIDTH 1280.0f
#define HEIGHT 720.0f

// Takeout Food, Tool kit, tableware


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	Window window;
	while (ShowCursor(FALSE) >= -1);
	window.create(WIDTH, HEIGHT, "My Window");
	window.clipMouseToWindow();
	if (GetFocus()) {
		SetCursorPos(1536 / 2, 960 / 2);
	}
	HandleMovement movement_handler{ window };
	
	Core core;
	core.init(window.hwnd, WIDTH, HEIGHT);
	Shaders shaders;
	PSOManager psos;
	TextureManager texture_manager;

	Plane plane;
	plane.init(&core, &psos, &shaders);

	StaticModel staticModel;
	staticModel.load(&core, "Models/acacia_003.gem", &shaders, &psos);

	AnimatedModel animatedModel;
	animatedModel.load(&core, "Models/TRex.gem", &psos, &shaders, &texture_manager);
	AnimationInstance animatedInstance;
	animatedInstance.init(&animatedModel.animation, 0);

	Timer timer;
	float t = 0;
	while (true)
	{
		core.beginFrame();
		float dt = timer.dt();
		window.checkInput();
		if (window.keys[VK_ESCAPE] == 1)
		{
			break;
		}

		t += dt;
		Matrix vp;
		Matrix p = Matrix::perspective(0.01f, 10000.0f, WIDTH / HEIGHT, 60.0f);
		Matrix v = movement_handler.getView(window, dt);
		vp = v * p;


		shaders.updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "VP", &vp);
		core.beginRenderPass();

		plane.draw(&core, &psos, &shaders, vp);

		Matrix W;
		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::translation(Vec3(5, 0, 0));
		staticModel.updateWorld(&shaders, W);
		staticModel.draw(&core, &psos, &shaders, vp);

		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::translation(Vec3(10, 0, 0));
		staticModel.updateWorld(&shaders, W);
		staticModel.draw(&core, &psos, &shaders, vp);

		updateAnimation(&animatedInstance, "roar", dt);
		shaders.updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "VP", &vp);
		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
		animatedModel.draw(&core, &psos, &shaders, &animatedInstance, vp, W);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
}