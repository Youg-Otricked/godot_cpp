// meta-description: Classic movement for gravity games (platformer, ...)
#pragma once
#include <godot/godot_cpp.h>
class _CLASS_ : public _BASE_ {
public:
	METHOD_REGISTRY {
		registry.register_method("_physics_process", &_physics_process);
	}
	float speed = 300.0f;
	float jump_velocity = -400.0f;
	virtual void _physics_process(double delta) override;
};
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
void _CLASS_::_physics_process(double delta) {
	Vector2 v = velocity;

	// Add the gravity.
	if (!is_on_floor()) {
		v += get_gravity() * (float)delta;
	}

	// Handle Jump.
	if (input::is_action_just_pressed("ui_accept") && is_on_floor()) {
		v.y = jump_velocity;
	}

	// Get the input direction and handle the movement/deceleration.
	// As good practice, you should replace UI actions with custom gameplay actions.
	Vector2 direction = input::get_vector("ui_left", "ui_right", "ui_up", "ui_down");
	if (direction != Vector2::zero) {
		v.x = direction.x * speed;
	} else {
		v.x = Mathf::move_toward(velocity.x, 0, speed);
	}

	velocity = v;
	move_and_slide();
}
