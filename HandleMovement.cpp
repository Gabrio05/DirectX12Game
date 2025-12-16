#include "HandleMovement.h"

HandleMovement::HandleMovement(Window& window) {
	default_mouse_x = window.getMouseInWindowX();
	default_mouse_y = window.getMouseInWindowY();
}

Matrix HandleMovement::getView(Window& window, float dt) {
	Vec3 from = Vec3(0, 1, 15);
	mouse_rotation_x -= (window.getMouseInWindowX() - default_mouse_x) / mouse_speed_x;
	mouse_rotation_y -= (window.getMouseInWindowY() - default_mouse_y) / mouse_speed_y;
	while (mouse_rotation_x < 0.0f) { mouse_rotation_x += 2 * M_PI; }
	while (mouse_rotation_x > 2.0f * M_PI) { mouse_rotation_x -= 2 * M_PI; }
	if (mouse_rotation_y > M_PI / 2.0f) { mouse_rotation_y = M_PI / 2.0f; }
	if (mouse_rotation_y < -M_PI / 2.0f) { mouse_rotation_y = -M_PI / 2.0f; }
	Vec3 to = Vec3(cosf(mouse_rotation_x), sinf(mouse_rotation_y), sinf(mouse_rotation_x));
	if (GetFocus()) {
		SetCursorPos(1536 / 2, 960 / 2);
	}
	Vec3 up = Vec3(0, 1, 0);
	Vec3 cross = Cross(up, to).normalize();
	if (window.keys['A']) {
		lean -= dt / lean_time;
		if (lean < -1.0f) { lean = -1.0f; }
	}
	else if (window.keys['D']) {
		lean += dt / lean_time;
		if (lean > 1.0f) { lean = 1.0f; }
	}
	else {
		if (lean < dt / lean_time / 2 && lean > -dt / lean_time / 2) {
			lean = 0.0f;
		}
		else if (lean < 0) {
			lean += dt / lean_time / 2;
		}
		else {
			lean -= dt / lean_time / 2;
		}
	}
	up += cross * 0.2 * lean;
	from += cross * lean;
	return Matrix::lookAt(from, from + to, up);
}