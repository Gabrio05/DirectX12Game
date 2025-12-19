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
#include <random>
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
			const int vase_index = 0;
			Vec3 target = target_position + object.target_offset;
			Vec3 end = (target - object.starting_position) * 2 + object.starting_position;
			manager->static_offset_vectors.at(vase_index) = interpolate(object.starting_position, end, (object.current_time + dt) / object.time_to_player / 2);
			manager->static_rotation_quaternions.at(vase_index) = object.update(dt);
			manager->staticModelDraw(vase_index);
		}
		std::erase_if(thrown_objects, [](ThrownObject object) { return object.shouldDespawn(); });
	}
};

int readIntNumber(std::string the_string) {
	int number = 0;
	bool negative = false;
	for (const char& c : the_string) {
		if (c == ',') {
			break;
		}
		else if (c == '-') {
			negative = true;
		}
		else {
			number *= 10;
			number += c - '0';
		}
	}
	if (negative) {
		return -number;
	}
	return number;
}

float readNumber(std::string the_string) {
	int whole_part = 0;
	bool negative = false;
	int decimal_part = 0;
	bool on_decimals = false;
	int decimal_digits = 0;
	bool exponential = false;
	bool negative_exponent = false;
	int exponent = 0;
	for (const char& c : the_string) {
		if (c == ',') {
			break;
		}
		else if (c == '.') {
			on_decimals = true;
		}
		else if (c == 'e') {
			exponential = true;
		}
		else if (c == '-') {
			if (exponential) {
				negative_exponent = true;
			}
			else {
				negative = true;
			}
		}
		else {
			if (!on_decimals) {
				whole_part *= 10;
				whole_part += c - '0';
			}
			else if (!exponential) {
				decimal_part *= 10;
				decimal_part += c - '0';
				decimal_digits++;
			}
			else {
				exponent *= 10;
				exponent += c - '0';
			}
		}
	}
	float number = whole_part + decimal_part / pow(10, decimal_digits);
	if (exponential) {
		if (negative_exponent) {
			exponent = -exponent;
		}
		number = number * pow(10, exponent);
	}
	if (negative) {
		return -number;
	}
	return number;
}

std::string getNextString(std::ifstream* file) {
	std::string to_return = "";
	char c = file->get();
	while (!file->eof() && c != ',' && c != '\n') {
		to_return += c;
		c = file->get();
	}
	return to_return;
}

void loadAllLevel(Core* core, TextureManager* tex_man, InstanceManager* instance_manager, std::string dataFilename) {
	std::ifstream file;
	file.open(dataFilename);
	while (!file.eof()) {
		std::vector<std::string> next_command{};
		while (!file.eof() && (next_command.size() == 0 || next_command.back() != "")) {
			next_command.push_back(getNextString(&file));
		}
		if (next_command.at(0) == "plane") {
			instance_manager->planeInit();
		}
		else if (next_command.at(0) == "static") {
			bool has_textures = false;
			if (next_command.size() > 8 && next_command.at(8)[0] == 'M') {
				has_textures = true;
			}
			instance_manager->staticModelLoad(next_command.at(1),
				Vec3(readNumber(next_command.at(2)), readNumber(next_command.at(3)), readNumber(next_command.at(4))),
				Vec3(readNumber(next_command.at(5)), readNumber(next_command.at(6)), readNumber(next_command.at(7))),
				has_textures);
			int i = 8;
			while (next_command.size() > i && next_command.at(i)[0] == 'M') {
				instance_manager->model_manager.staticModelTextureLoad(core, next_command.at(i));
				i++;
			}
		}
		else if (next_command.at(0) == "staticRepeat") {
			instance_manager->addStaticCPUInstance(Vec3(readNumber(next_command.at(1)), readNumber(next_command.at(2)), readNumber(next_command.at(3))));
		}
		else if (next_command.at(0) == "animated") {
			instance_manager->animatedModelLoad("Models/TRex.gem", "Models/Textures/T-rex_Base_Color_alb.png", Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f)));
		}
		else if (next_command.at(0) == "sphere") {
			instance_manager->sphereInit(readNumber(next_command.at(1)), readNumber(next_command.at(2)), readNumber(next_command.at(3)), tex_man);
			int i = 4;
			while (next_command.size() > i && next_command.at(i)[0] == 'M') {
				instance_manager->model_manager.staticModelTextureLoad(core, next_command.at(i));
				i++;
			}
		}
		else if (next_command.at(0) == "instance") {
			if (next_command.at(1) == "line") {
				std::vector<Matrix> instanceMatrices{};
				int instanceNumber = readIntNumber(next_command.at(2));
				Vec3 offset = Vec3(readNumber(next_command.at(6)), readNumber(next_command.at(7)), readNumber(next_command.at(8)));
				Vec3 multiply = Vec3(readNumber(next_command.at(3)), readNumber(next_command.at(4)), readNumber(next_command.at(5)));
				Vec3 scale = Vec3(readNumber(next_command.at(9)), readNumber(next_command.at(10)), readNumber(next_command.at(11)));
				for (int i = 0; i < instanceNumber; i++) {
					Matrix inst{};
					inst = inst.translation(multiply * i + offset);
					Matrix mult = Matrix::scaling(scale);
					instanceMatrices.push_back(mult * inst);
				}
				instance_manager->instanceModelLoad(next_command.at(12), {}, {}, true, {}, instanceMatrices);
				int i = 13;
				while (next_command.size() > i && next_command.at(i)[0] == 'M') {
					instance_manager->model_manager.staticModelTextureLoad(core, next_command.at(i));
					i++;
				}
			}
			else if (next_command.at(1) == "grid") {
				std::random_device rd;
				std::uniform_real_distribution slight_offset{ 0.6, 1.4 };
				std::uniform_real_distribution random_rotation{ 0.0, 360.0 };
				std::unique_ptr<std::default_random_engine> engine = std::make_unique<std::default_random_engine>(rd());

				std::vector<Matrix> instanceMatrices{};
				int instanceNumber = readIntNumber(next_command.at(2));
				Vec3 offset = Vec3(readNumber(next_command.at(3)), readNumber(next_command.at(4)), readNumber(next_command.at(5)));
				Vec3 multiply_x = Vec3(readNumber(next_command.at(6)), readNumber(next_command.at(7)), readNumber(next_command.at(8)));
				Vec3 multiply_y = Vec3(readNumber(next_command.at(9)), readNumber(next_command.at(10)), readNumber(next_command.at(11)));
				Vec3 scale = Vec3(readNumber(next_command.at(12)), readNumber(next_command.at(13)), readNumber(next_command.at(14)));
				for (int i = 0; i < sqrt(instanceNumber); i++) {
					for (int j = 0; j < sqrt(instanceNumber); j++) {
						Matrix inst{};
						inst = inst.translation(multiply_x * i * slight_offset(*engine) + multiply_y * j * slight_offset(*engine) + offset);
						Matrix mult = Matrix::scaling(scale);
						instanceMatrices.push_back(mult * inst);
					}
				}
				instance_manager->instanceModelLoad(next_command.at(15), {}, {}, true, {}, instanceMatrices);
				int i = 16;
				while (next_command.size() > i && next_command.at(i)[0] == 'M') {
					instance_manager->model_manager.staticModelTextureLoad(core, next_command.at(i));
					i++;
				}
			}
			
		}
	}
	file.close();
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
	loadAllLevel(&core, &texture_manager, &instance_manager, "levelData.txt");

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

		if (t > thrown_vases * 5.0f) {
			vase_manager.throwVase(Vec3(0, 0, 0), 4, Vec3(thrown_vases, 0, 0));
			thrown_vases++;
		}
		vase_manager.updateThrownObjects(&instance_manager, dt);

		updateAnimation(&instance_manager.model_manager.animated_instances.at(0), "roar", dt);

		instance_manager.drawAll(t);

		core.finishFrame();
	}
	core.flushGraphicsQueue();
}