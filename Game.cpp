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
	instance_manager.staticModelLoad("Models/Vase_Set_66a.gem", Vec3(10.0f, 10.0f, 10.0f), Vec3(-1, 0, 5), true);
	instance_manager.model_manager.staticModelTextureLoad(&core, "Models/Textures/TX_Vase_Set_66a_ALB.png");
	instance_manager.animatedModelLoad("Models/TRex.gem", "Models/Textures/T-rex_Base_Color_alb.png", Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)));

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
		instance_manager.static_offset_vectors.at(2) = Vec3(10, 0, 10);
		instance_manager.staticModelDraw(2);

		updateAnimation(&instance_manager.model_manager.animated_instances.at(0), "roar", dt);
		instance_manager.animated_world_matrix = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
		instance_manager.animatedModelDraw();

		core.finishFrame();
	}
	core.flushGraphicsQueue();
}