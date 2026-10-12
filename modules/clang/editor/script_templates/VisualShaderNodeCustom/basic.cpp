// meta-description: Visual shader's node plugin template
#pragma once
[[Tool]]
[[GlobalClass]] class VisualShaderNode_CLASS_ : public _BASE_ {
public:
	virtual String _get_name() override;
	virtual String _get_category() override;
	virtual String _get_description() override;
	virtual VisualShaderNode::PortType _get_return_icon_type() override;
	virtual int _get_input_port_count() override;
	virtual String _get_input_port_name(int port) override;
	virtual VisualShaderNode::PortType _get_input_port_type(int port) override;
	virtual int _get_output_port_count() override;
	virtual String _get_output_port_name(int port) override;
	virtual VisualShaderNode::PortType _get_output_port_type(int port) override;
	virtual String _get_code(Collections::Vector<String> input_vars, Collections::Vector<String> output_vars, Shader::Mode mode, VisualShader::Type type) override;
};
// __GODOT_CPP_SOURCE_BEGIN__
#include "_CLASS_.h"
String VisualShaderNode_CLASS_::_get_name() {
	return "_CLASS_";
}

String VisualShaderNode_CLASS_::_get_category() {
	return "";
}

String VisualShaderNode_CLASS_::_get_description() {
	return "";
}

VisualShaderNode::PortType VisualShaderNode_CLASS_::_get_return_icon_type() {
	return 0;
}

int VisualShaderNode_CLASS_::_get_input_port_count() {
	return 0;
}

String VisualShaderNode_CLASS_::_get_input_port_name(int port) {
	return "";
}

VisualShaderNode::PortType VisualShaderNode_CLASS_::_get_input_port_type(int port) {
	return 0;
}

int VisualShaderNode_CLASS_::_get_output_port_count() {
	return 1;
}

String VisualShaderNode_CLASS_::_get_output_port_name(int port) {
	return "result";
}

VisualShaderNode::PortType VisualShaderNode_CLASS_::_get_output_port_type(int port) {
	return 0;
}

String VisualShaderNode_CLASS_::_get_code(Collections::Vector<String> input_vars, Collections::Vector<String> output_vars, Shader::Mode mode, VisualShader::Type type) {
	return "";
}
