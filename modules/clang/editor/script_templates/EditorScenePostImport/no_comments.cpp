// meta-description: Basic import script template
#pragma once

#if TOOLS

[[Tool]] class _CLASS_ : public _BASE_ {
public:
	virtual GodotObject _post_import(Node scene) override;
};
#endif
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
#if TOOLS
GodotObject _CLASS_::_post_import(Node scene) {
	// Modify the contents of the scene upon import.
	return scene; // Return the modified root node when you're done.
}
#endif

