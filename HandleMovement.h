#pragma once
#include "Window.h"
#include "Maths.h"

class HandleMovement {
public:
	float mouse_rotation_x = 4.5f;  // 0 to 2 PI, allowed to loop
	float mouse_rotation_y = 0.0f;  // - PI/2 to PI/2, not allowed to loop
	float default_mouse_x;
	float default_mouse_y;
	const float mouse_speed_x = 200.0f;
	const float mouse_speed_y = 300.0f;
	float lean = 0.0f;  // -1 for left, 1 for right, interpolated
	const float lean_time = 0.2f;

	bool free_movement = false;
	bool is_free_movement_key_pressed = false;
	const Vec3 default_position = Vec3(0, 1, 15);
	Vec3 current_position = default_position;
	const float movement_speed = 5.0f;

	HandleMovement(Window& window);

	Matrix getView(Window& window, float dt);
};

