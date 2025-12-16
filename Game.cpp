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

	InstanceManagement instance_manager;
	instance_manager.planeInit(&core, &psos, &shaders);
	instance_manager.staticModelLoad(&core, "Models/acacia_003.gem", &psos, &shaders);
	instance_manager.animatedModelLoad(&core, "Models/TRex.gem", &psos, &shaders, &texture_manager);

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


		instance_manager.planes[0]->draw(&core, &psos, &shaders, vp);

		Matrix W;
		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::translation(Vec3(5, 0, 0));
		instance_manager.static_models.at(0)->updateWorld(&shaders, W);
		instance_manager.static_models.at(0)->draw(&core, &psos, &shaders, vp);

		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)) * Matrix::translation(Vec3(10, 0, 0));
		instance_manager.static_models.at(0)->updateWorld(&shaders, W);
		instance_manager.static_models.at(0)->draw(&core, &psos, &shaders, vp);

		updateAnimation(&instance_manager.animated_instances.at(0), "roar", dt);
		shaders.updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "VP", &vp);
		W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
		instance_manager.animated_models.at(0)->draw(&core, &psos, &shaders, &instance_manager.animated_instances.at(0), vp, W);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
}