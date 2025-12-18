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
#include "InstanceManagement.h"
// Properties -> Linker -> System -> Windows
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void updateAnimation(AnimationInstance* animated, std::string current_animation, float dt) {
	animated->update(current_animation, dt);
	if (animated->animationFinished()) {
		animated->resetAnimationTime();
	}
}

Vec3 interpolate(Vec3 p1, Vec3 p2, float t)
{
	return ((p1 * (1.0f - t)) + (p2 * t));
}

class rotationHolder {
public:
	inline static const float rotation_time = 2.0f;
	float current_t = 0.0f;
	Vec3 rotation_axis = Vec3(1, 0, 0);

	Quaternion updateRotation(float dt) {
		current_t += dt / rotation_time;
		if (current_t >= 1.0f) { current_t = 0; }
		return MakeRotationQuaternion(rotation_axis, M_PI * 2 * current_t);
	}
};

class ThrownObject {
public:
	rotationHolder rotation{};
	Vec3 starting_position;
	Vec3 target_offset;
	float time_to_player;
	float current_time = 0;

	ThrownObject(Vec3 start, float time, Vec3 offset) {
		starting_position = start;
		target_offset = offset;
		time_to_player = time;
	}

	Quaternion update(float dt) {
		current_time += dt;
		return rotation.updateRotation(dt);
	}

	bool hasPastPlayer() { return current_time > time_to_player; }
	bool shouldDespawn() { return current_time > 2 * time_to_player; }
};

class ObjectThrowing {
public:
	std::vector<ThrownObject> thrown_objects;
	const Vec3 target_position = Vec3(0, 1, 15);  // base target position is the same for all (add offset to it)
	void throwVase(Vec3 start, float time, Vec3 offset) {
		thrown_objects.push_back({ start, time, offset });
	}

	void updateThrownObjects(InstanceManager* manager, float dt) {
		for (ThrownObject& object : thrown_objects) {
			const int vase_index = 2;
			Vec3 target = target_position + object.target_offset;
			Vec3 end = (target - object.starting_position) * 2 + object.starting_position;
			manager->static_offset_vectors.at(vase_index) = interpolate(object.starting_position, end, (object.current_time + dt) / object.time_to_player / 2);
			manager->static_rotation_quaternions.at(vase_index) = object.update(dt);
			manager->staticModelDraw(vase_index);
		}
		std::erase_if(thrown_objects, [](ThrownObject object) { return object.shouldDespawn(); });
	}
};

#define WIDTH 1280.0f
#define HEIGHT 720.0f

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
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

	InstanceManager instance_manager{ &core, &psos, &shaders, &texture_manager };
	instance_manager.planeInit();
	instance_manager.staticModelLoad("Models/acacia_003.gem", Vec3(0.01f, 0.01f, 0.01f), Vec3(5, 0, 0));
	instance_manager.staticModelLoad("Models/Scraggly_Bush_01a.gem", Vec3(2.0f, 2.0f, 2.0f), Vec3(0, 0, 18), true);
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Scraggly_Bushes_01a_ALB.png");
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Scraggly_Bushes_01a_NH.png");
	instance_manager.staticModelLoad("Models/Vase_Set_66a.gem", Vec3(10.0f, 10.0f, 10.0f), Vec3(-1, 0, 5), true);
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Vase_Set_66a_ALB.png");
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Vase_Set_66a_NH.png");
	instance_manager.animatedModelLoad("Models/TRex.gem", "Models/Textures/T-rex_Base_Color_alb.png", Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)));
	instance_manager.sphereInit(200, 500, 1000, &texture_manager);
	instance_manager.model_manager.sphereTextureLoad(&core, "Models/Textures/qwantani_moon_noon_puresky.jpg");
	instance_manager.model_manager.sphereTextureLoad(&core, "Models/Textures/black-image-8192x4096-rectangle.png");
	std::vector<Matrix> instanceMatrices{};
	for (int i = 0; i < 100; i++) {
		Matrix inst{};
		inst = inst.translation(Vec3(i * 3, 1, 0));
		instanceMatrices.push_back(inst);
	}
	instance_manager.instanceModelLoad("Models/Takeout_Food_01a.gem", Vec3(10, 10, 10), Vec3(), true, {}, instanceMatrices);
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Takeout_Food_01a_ALB.png");

	ObjectThrowing vase_manager{};
	int thrown_vases = 0;

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
		instance_manager.view_perspective_matrix = vp;

		//shaders.updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "VP", &vp);
		//shaders.updateConstantVS("StaticModelTextured", "staticMeshBuffer", "VP", &vp);
		core.beginRenderPass();

		instance_manager.planeDraw();

		instance_manager.static_offset_vectors.at(0) = Vec3(5, 0, 0);
		instance_manager.staticModelDraw(0);
		instance_manager.static_offset_vectors.at(0) = Vec3(10, 0, 0);
		instance_manager.staticModelDraw(0);
		instance_manager.staticModelDraw(1);
		if (t > thrown_vases * 5.0f) {
			vase_manager.throwVase(Vec3(0, 0, 0), 4, Vec3(thrown_vases, 0, 0));
			thrown_vases++;
		}
		vase_manager.updateThrownObjects(&instance_manager, dt);

		updateAnimation(&instance_manager.model_manager.animated_instances.at(0), "roar", dt);
		instance_manager.animated_world_matrix = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
		instance_manager.animatedModelDraw();
		instance_manager.sphereDraw(t);
		instance_manager.instanceModelDraw(3);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
}