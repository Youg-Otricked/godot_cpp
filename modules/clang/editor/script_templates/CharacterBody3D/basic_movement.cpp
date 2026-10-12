// meta-description: Classic movement for gravity games (FPS, TPS, ...)
#pragma once

class _CLASS_ : public _BASE_ {
public:
	float speed = 5.0f;
	float jump_velocity = 4.5f;

	virtual void _physics_process(double delta) override;
};
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
void _CLASS_::_physics_process(double delta) {
	Vector3 v = velocity;

	// Add the gravity.
	if (!is_on_floor()) {
		velocity += get_gravity() * (float)delta;
	}

	// Handle Jump.
	if (input::is_action_just_pressed("ui_accept") && is_on_floor()) {
		velocity.y = jump_velocity;
	}

	// Get the input direction and handle the movement/deceleration.
	// As good practice, you should replace UI actions with custom gameplay actions.
	Vector2 input_dir = input::get_vector("ui_left", "ui_right", "ui_up", "ui_down");
	Vector3 direction = (transform::basis * Vector3(input_dir.x, 0, input_dir.y)).normalized();
	if (direction != Vector3::zero) {
		velocity.x = direction.x * speed;
		velocity.z = direction.z * speed;
	} else {
		velocity.x = Mathf::move_toward(velocity.x, 0, speed);
		velocity.z = Mathf::move_toward(velocity.z, 0, speed);
	}

	velocity = v;
	move_and_slide();
}
