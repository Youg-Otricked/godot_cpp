// meta-description: Base template for Node with default Godot cycle methods
#pragma once

class _CLASS_ : public _BASE_
{
public:
    // Called when the node enters the scene tree for the first time.
    virtual void _ready() override;

    // Called every frame. 'delta' is the elapsed time since the previous frame.
    virtual void _process(double delta) override;
};
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
// Called when the node enters the scene tree for the first time.
void _CLASS_::_ready()
{
}

// Called every frame. 'delta' is the elapsed time since the previous frame.
void _CLASS_::_process(double delta)
{
}
