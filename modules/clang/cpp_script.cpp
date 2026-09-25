/**************************************************************************/
/*  cpp_script.cpp			                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "cpp_script.h"
#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/plugins/editor_plugin.h"
#endif

/************ CppLanguage ********/

CppLanguage *CppLanguage::singleton = nullptr;

CppScriptInstance::CppScriptInstance(const Ref<CppScript> &p_script, Object *p_owner) {
	this->script = p_script;
	this->owner = p_owner;
}

Ref<Script> CppScriptInstance::get_script() const {
	return this->script;
}

ScriptLanguage *CppScriptInstance::get_language() {
	return CppLanguage::get_singleton();
}
bool CppScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	return false;
}
bool CppScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	return false;
}
void CppScriptInstance::get_property_list(List<PropertyInfo> *r_properties) const {
}
Variant::Type CppScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	return Variant::NIL;
}
void CppScriptInstance::validate_property(PropertyInfo &p_property) const {
}
bool CppScriptInstance::property_can_revert(const StringName &p_name) const {
	return false;
}
bool CppScriptInstance::property_get_revert(const StringName &p_name, Variant &r_ret) const {
	return false;
}

void CppScriptInstance::get_method_list(List<MethodInfo> *r_list) const {
}
bool CppScriptInstance::has_method(const StringName &p_method) const {
	return false;
}
Variant CppScriptInstance::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
	return Variant();
}
void CppScriptInstance::notification(int p_notification, bool p_reversed) {
}

static String get_base_class_name(const String &p_base_class_name, const String &p_class_name) {
	String base_class = p_base_class_name;
	if (p_class_name == base_class) {
		base_class = "Godot." + base_class;
	}
	return base_class;
}

/************* CppLanguage ****/

Ref<Script> CppLanguage::make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const {
	Ref<CppScript> scr;
	scr.instantiate();

	String class_name_no_spaces = p_class_name.replace_char(' ', '_');
	String base_class_name = get_base_class_name(p_base_class_name, class_name_no_spaces);
	String processed_template = p_template;
	processed_template = processed_template.replace("_BASE_", base_class_name)
								 .replace("_CLASS_", class_name_no_spaces)
								 .replace("_TS_", "4");
	scr->set_source_code(processed_template);
	return scr;
}
void CppLanguage::init() {
}
void CppLanguage::finish() {
}
String CppLanguage::get_name() const {
	return "C++";
}
String CppLanguage::get_type() const {
	return "cpp";
}
String CppLanguage::get_extension() const {
	return "cpp";
}
Vector<String> CppLanguage::get_reserved_words() const {
	return Vector<String>();
}
bool CppLanguage::is_control_flow_keyword(const String &p_string) const {
	return false;
}
Vector<String> CppLanguage::get_comment_delimiters() const {
	return { "//", "/*", "*/" };
}

Vector<String> CppLanguage::get_string_delimiters() const {
	return { "\"", "'" };
}
Vector<String> CppLanguage::get_doc_comment_delimiters() const {
	return Vector<String>();
}
bool CppLanguage::supports_builtin_mode() const {
	return false;
}
String CppLanguage::make_function(const String &p_class, const String &p_name, const PackedStringArray &p_args) const {
	return String();
}
void CppLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {
}
void CppLanguage::reload_all_scripts() {
}
void CppLanguage::reload_scripts(const Array &p_scripts) {
}
void CppLanguage::reload_tool_script(const Ref<Script> &p_script) {
}
void CppLanguage::get_public_functions(List<MethodInfo> *r_functions) const {
}
void CppLanguage::get_public_constants(List<Pair<String, Variant>> *r_constants) const {
}
void CppLanguage::get_public_annotations(List<MethodInfo> *r_annotations) const {
}
CppLanguage::CppLanguage() {
	ERR_FAIL_COND_MSG(singleton, "C# singleton already exists.");
	singleton = this;
}

CppLanguage::~CppLanguage() {
	singleton = nullptr;
}

/********** CppScript ****************/
CppScript *CppScript::get_root_script() {
	CppScript *result = this;
	while (result->_owner) {
		result = result->_owner;
	}
	return result;
}

void CppScript::set_path_cache(const String &p_path) {
	if (is_root_script()) {
		Script::set_path_cache(p_path);
	}
	path = p_path;
	path_valid = true;
}
void CppScript::set_path(const String &p_path, bool p_take_over) {
	if (is_root_script()) {
		Script::set_path(p_path, p_take_over);
	}
	path = p_path;
	path_valid = true;
}

String CppScript::get_script_path() const {
	return path;
}

bool CppScript::can_instantiate() const {
	return false;
}
Ref<Script> CppScript::get_base_script() const {
	return Ref<Script>();
}
StringName CppScript::get_global_name() const {
	return StringName();
}
bool CppScript::inherits_script(const Ref<Script> &p_script) const {
	return false;
}
StringName CppScript::get_instance_base_type() const {
	return StringName();
} // this may not work in all scripts, will return empty if so
ScriptInstance *CppScript::instance_create(Object *p_this) {
	return nullptr;
}
bool CppScript::has_source_code() const {
	return !source.is_empty();
}
String CppScript::get_source_code() const {
	return source;
}
void CppScript::set_source_code(const String &p_code) {
	source = p_code;
}
void CppScript::update_exports() {
}
Error CppScript::reload(bool p_keep_state) {
	return Error();
}
bool CppScript::get_property_default_value(const StringName &p_property, Variant &r_value) const {
	return false;
}
void CppScript::get_script_method_list(List<MethodInfo> *p_list) const {
}
bool CppScript::has_method(const StringName &p_method) const {
	return false;
}
bool CppScript::has_static_method(const StringName &p_method) const {
	return false;
}

int CppScript::get_script_method_argument_count(const StringName &p_method, bool *r_is_valid) const {
	return 0;
}

MethodInfo CppScript::get_method_info(const StringName &p_method) const {
	return MethodInfo{};
}

void CppScript::get_script_property_list(List<PropertyInfo> *p_list) const {
}

ScriptLanguage *CppScript::get_language() const {
	return CppLanguage::get_singleton();
}

bool CppScript::has_script_signal(const StringName &p_signal) const {
	return false;
}
void CppScript::get_script_signal_list(List<MethodInfo> *r_signals) const {
}
const Variant CppScript::get_rpc_config() const {
	return Variant();
}
Error CppScript::load_source_code(const String &p_path) {
	if (p_path.is_empty() || p_path.begins_with("cpp://") || ResourceLoader::get_resource_type(p_path.get_slice("::", 0)) == "PackedScene") {
		return OK;
	}

	Vector<uint8_t> sourcef;
	Error err;
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
	if (err) {
		const char *err_name;
		if (err < 0 || err >= ERR_MAX) {
			err_name = "(invalid error code)";
		} else {
			err_name = error_names[err];
		}
		ERR_FAIL_COND_V_MSG(err, err, "Attempt to open script '" + p_path + "' resulted in error '" + err_name + "'.");
	}

	uint64_t len = f->get_length();
	sourcef.resize(len + 1);
	uint8_t *w = sourcef.ptrw();
	uint64_t r = f->get_buffer(w, len);
	ERR_FAIL_COND_V(r != len, ERR_CANT_OPEN);
	w[len] = 0;

	String s;
	if (s.append_utf8((const char *)w, len) != OK) {
		ERR_FAIL_V_MSG(ERR_INVALID_DATA, "Script '" + p_path + "' contains invalid unicode (UTF-8), so it was not loaded. Please ensure that scripts are saved in valid UTF-8 unicode.");
	}

	source = s;
	path = p_path;
	path_valid = true;
#ifdef TOOLS_ENABLED
	set_edited(false);
	set_last_modified_time(FileAccess::get_modified_time(path));
#endif // TOOLS_ENABLED
	return OK;
}
