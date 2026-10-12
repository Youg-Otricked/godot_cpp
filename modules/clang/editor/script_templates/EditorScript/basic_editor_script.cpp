// meta-description: Basic editor script template
#pragma once

#if TOOLS
[[Tool]] class _CLASS_ : public _BASE_ {
public:
	virtual void _run() override;
};
#endif
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
#if TOOLS
// Called when the script is executed (using File -> Run in Script Editor).
void _CLASS_::_run() {
}
#endif
