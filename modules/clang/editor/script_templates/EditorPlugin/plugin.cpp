// meta-description: Basic plugin template
#pragma once

#if TOOLS

[[Tool]] class _CLASS_ : public _BASE_ {
public:
	virtual void _enter_tree() override;
	virtual void _exit_tree() override;
};
#endif
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"

#if TOOLS
void _CLASS_::_enter_tree() {
	// Initialization of the plugin goes here.
}

void _CLASS_::_exit_tree() {
	// Clean-up of the plugin goes here.
}
#endif
