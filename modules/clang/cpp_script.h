/**************************************************************************/
/*  cpp_script.h                                                          */
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

#pragma once

#include "core/doc_data.h"
#include "core/object/script_language.h"

#ifdef TOOLS_ENABLED
#include "core/io/resource_loader.h"
#include "core/object/editor_language.h"
#include "editor/plugins/editor_plugin.h"

#endif

class CppScript;
class CppInstance;
class CppLanguage;

template <typename TScriptInstance, typename TScriptLanguage>
TScriptInstance *cast_script_instance(ScriptInstance *p_inst) {
	return dynamic_cast<TScriptInstance *>(p_inst);
}

#define CAST_CPP_INSTANCE(m_inst) (cast_script_instance<CppInstance, CppLanguage>(m_inst))

class CppScriptInstance : public ScriptInstance {
	friend class CppScript;
	friend class CppLanguage;
	Ref<CppScript> script;
	Object *owner = nullptr;

public:
	bool set(const StringName &, const Variant &) override;
	bool get(const StringName &, Variant &) const override;
	void get_property_list(List<PropertyInfo> *) const override;
	Variant::Type get_property_type(const StringName &, bool *) const override;
	void validate_property(PropertyInfo &) const override;
	Object *get_owner() override { return owner; }
	bool property_can_revert(const StringName &) const override;
	bool property_get_revert(const StringName &, Variant &) const override;

	void get_method_list(List<MethodInfo> *) const override;
	bool has_method(const StringName &) const override;
	Variant callp(const StringName &, const Variant **, int, Callable::CallError &) override;

	void notification(int, bool = false) override;
	Ref<Script> get_script() const override;
	ScriptLanguage *get_language() override;
	CppScriptInstance(const Ref<CppScript> &p_script, Object *p_owner);
	~CppScriptInstance() override = default;
};
class CppLanguage : public ScriptLanguage {
	GDCLASS(CppLanguage, ScriptLanguage);
	friend class CppScript;
	friend class CppScriptInstance;
	static CppLanguage *singleton;

public:
	_FORCE_INLINE_ static CppLanguage *get_singleton() {
		return singleton;
	}
	String get_name() const override;
	void init() override;
	void finish() override;
	String get_type() const override;
	String get_extension() const override;
	Vector<String> get_reserved_words() const override;
	virtual Ref<Script> make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const override;
	bool is_control_flow_keyword(const String &p_string) const override;
	Vector<String> get_comment_delimiters() const override;
	Vector<String> get_doc_comment_delimiters() const override;
	Vector<String> get_string_delimiters() const override;
	bool supports_builtin_mode() const override;
	String make_function(const String &p_class, const String &p_name, const PackedStringArray &p_args) const override;
	void add_global_constant(const StringName &p_variable, const Variant &p_value) override;
	void add_named_global_constant(const StringName &p_name, const Variant &p_value) override {}
	void remove_named_global_constant(const StringName &p_name) override {}
	String debug_get_error() const override { return String(); }
	int debug_get_stack_level_count() const override { return 0; }
	int debug_get_stack_level_line(int p_level) const override { return -1; }
	String debug_get_stack_level_function(int p_level) const override { return String(); }
	String debug_get_stack_level_source(int p_level) const override { return String(); }
	void debug_get_stack_level_locals(int p_level, List<String> *r_locals, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	void debug_get_stack_level_members(int p_level, List<String> *r_members, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	ScriptInstance *debug_get_stack_level_instance(int p_level) override { return nullptr; }
	void debug_get_globals(List<String> *r_globals, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	String debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems = -1, int p_max_depth = -1) override {}
	Vector<StackInfo> debug_get_current_stack_info() override { return Vector<StackInfo>(); }
	void reload_all_scripts() override;
	void reload_scripts(const Array &p_scripts) override;
	void reload_tool_script(const Ref<Script> &p_script) override;
	void get_public_functions(List<MethodInfo> *r_functions) const override;
	void get_public_constants(List<Pair<String, Variant>> *r_constants) const override;
	void get_public_annotations(List<MethodInfo> *r_annotations) const override;
	void profiling_start() override {}
	void profiling_stop() override {}
	void profiling_set_save_native_calls(bool p_enable) override {}
#ifdef TOOLS_ENABLED
	EditorLanguage editor_language;
	virtual EditorLanguage *get_editor_language() override {
		return &editor_language;
	}
#endif
	int profiling_get_accumulated_data(ProfilingInfo *, int) override {
		return 0;
	}
	int profiling_get_frame_data(ProfilingInfo *, int) override {
		return 0;
	}
	bool is_using_templates() override { return false; }
	Vector<ScriptTemplate> get_built_in_templates(const StringName &) override {
		return {};
	}
	String validate_path(const String &) const override {
		return String();
	}
	CppLanguage();
	~CppLanguage() override;
};
class CppScript : public Script {
	GDCLASS(CppScript, Script);
	bool _is_abstract = false;
	bool tool = false;
	bool valid = false;
	bool path_valid = false;
	String source;
	String path;
	CppScript *_owner = nullptr; //for subclasses

public:
	CppScript *get_root_script();
	bool is_root_script() const { return _owner == nullptr; }
	virtual bool can_instantiate() const override;
	virtual Ref<Script> get_base_script() const override;
	virtual StringName get_global_name() const override;
	bool inherits_script(const Ref<Script> &p_script) const override;
	virtual StringName get_instance_base_type() const override; // this may not work in all scripts, will return empty if so
	virtual ScriptInstance *instance_create(Object *p_this) override;
	virtual bool has_source_code() const override;
	virtual String get_source_code() const override;
	virtual void set_source_code(const String &p_code) override;
	virtual void update_exports() override;
	virtual Error reload(bool p_keep_state = false) override;
	virtual void set_path_cache(const String &p_path) override;
	virtual void set_path(const String &p_path, bool p_take_over = false) override;
	String get_script_path() const;
	Error load_source_code(const String &p_path);
	bool get_property_default_value(const StringName &p_property, Variant &r_value) const override;
#ifdef TOOLS_ENABLED
	StringName get_doc_class_name() const override { return StringName(); }
	Vector<DocData::ClassDoc> get_documentation() const override { return {}; }
	String get_class_icon_path() const override { return String(); }
#endif
	virtual void get_script_method_list(List<MethodInfo> *p_list) const override;
	virtual bool has_method(const StringName &p_method) const override;
	virtual bool has_static_method(const StringName &p_method) const override;

	virtual int get_script_method_argument_count(const StringName &p_method, bool *r_is_valid = nullptr) const override;

	virtual MethodInfo get_method_info(const StringName &p_method) const override;

	virtual void get_script_property_list(List<PropertyInfo> *p_list) const override;

	virtual ScriptLanguage *get_language() const override;

	virtual bool has_script_signal(const StringName &p_signal) const override;
	virtual void get_script_signal_list(List<MethodInfo> *r_signals) const override;
	bool is_tool() const override { return tool; }
	bool is_abstract() const override { return _is_abstract; }
	virtual bool is_script_valid() const override { return valid; }
	virtual const Variant get_rpc_config() const override;
	CppScript() = default;
	~CppScript() override = default;
};
