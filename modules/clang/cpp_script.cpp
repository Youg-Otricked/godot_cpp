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
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "scene/main/scene_tree.h"
#endif
#include "core/extension/gdextension_manager.h"
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "core/os/thread.h"
#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#endif
/************ CppLanguage ********/

CppLanguage *CppLanguage::singleton = nullptr;

CppScriptInstance::CppScriptInstance(const Ref<CppScript> &p_script) {
	this->script = p_script;
}

Ref<Script> CppScriptInstance::get_script() const {
	return this->script;
}

ScriptLanguage *CppScriptInstance::get_language() {
	return CppLanguage::get_singleton();
}
bool CppScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	ERR_FAIL_COND_V(script.is_null(), false);
	return false;
}

bool CppScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	ERR_FAIL_COND_V(script.is_null(), false);

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
Variant CppScriptInstance::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	const MethodBind *call = ClassDB::get_method(script.is_valid() ? script->path.get_file().get_basename() : "", p_method);
	if (!call) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
		return Variant();
	}
	return call->call(owner, p_args, p_argcount, r_error);
}
bool CppScriptInstance::has_method(const StringName &p_method) const {
	return ClassDB::has_method(script.is_valid() ? script->path.get_file().get_basename() : "", p_method);
}
void CppScriptInstance::notification(int p_notification, bool p_reversed) {
	if (owner && has_method("_notification")) {
		Callable::CallError err;
		Variant arg = p_notification;
		const Variant *args = &arg;
		callp("_notification", &args, 1, err);
	}
}

static String get_base_class_name(const String &p_base_class_name, const String &p_class_name) {
	String base_class = p_base_class_name;
	return base_class;
}

/************* CppLanguage ****/

bool CppLanguage::is_using_templates() {
	return true;
}
static String make_glue_source(const String &p_class) {
	return String(R"(#include "_CLASS_.hpp"

static void _CLASS__init(godot::ModuleInitializationLevel p_level) {
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(_CLASS_);
	}
}
static void _CLASS__term(godot::ModuleInitializationLevel) {}

extern "C" GDExtensionBool GDE_EXPORT _CLASS__entry(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init(p_get_proc_address, p_library, r_initialization);
	init.register_initializer(_CLASS__init);
	init.register_terminator(_CLASS__term);
	init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
	return init.init();
}
)")
			.replace("_CLASS_", p_class);
}
static String make_gdextension_source(const String &p_class) {
	return String(R"([configuration]
entry_symbol = "_CLASS__entry"
compatibility_minimum = "4.8"
reloadable = true

[libraries]
linux.editor.x86_64 = "res://.godot/cpp_build/_CLASS_.so"
)")
			.replace("_CLASS_", p_class);
}
Ref<Script> CppLanguage::make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const {
	Ref<CppScript> scr = memnew(CppScript);

	String class_name_no_spaces = p_class_name.replace_char(' ', '_');
	String base_class_name = get_base_class_name(p_base_class_name, class_name_no_spaces);
	String processed_template = p_template;
	PackedStringArray parts = p_template.split("// __GODOT_CPP_SOURCE_BEGIN__");
	if (parts.size() == 2) {
		processed_template = parts[1].replace("_BASE_", base_class_name)
									 .replace("_CLASS_", class_name_no_spaces)
									 .replace("_TS_", "    ");
		scr->set_source_code(processed_template);
		processed_template = parts[0].replace("_BASE_", base_class_name)
									 .replace("_CLASS_", class_name_no_spaces)
									 .replace("_TS_", "    ");
		scr->set_header_code(processed_template);
	} else {
		scr->set_source_code("");
		scr->set_header_code("#pragma once");
	}
	scr->set_gdextension_source(make_gdextension_source(class_name_no_spaces));
	scr->set_bindings_source(make_glue_source(class_name_no_spaces));
	return scr;
}
Vector<ScriptLanguage::ScriptTemplate> CppLanguage::get_built_in_templates(const StringName &p_object) {
	Vector<ScriptLanguage::ScriptTemplate> templates;
#ifdef TOOLS_ENABLED
	for (int i = 0; i < TEMPLATES_ARRAY_SIZE; i++) {
		if (TEMPLATES[i].inherit == p_object) {
			templates.append(TEMPLATES[i]);
		}
	}
#endif
	return templates;
}
void CppLanguage::init() {
}
void CppLanguage::_load_prebuilt_extensions() {
	String build_dir = ProjectSettings::get_singleton()->globalize_path("res://.godot/cpp_build");
	Ref<DirAccess> da = DirAccess::open(build_dir);
	if (da.is_null()) {
		return;
	}
	GDExtensionManager *mgr = GDExtensionManager::get_singleton();
	for (const String &f : da->get_files()) {
		if (f.get_extension() != "gdextension") {
			continue;
		}
		String ext = String("res://.godot/cpp_build").path_join(f);
		if (!mgr->is_extension_loaded(ext)) {
			mgr->load_extension(ext);
		}
	}
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
CppLanguage::CppLanguage() {
	ERR_FAIL_COND_MSG(singleton, "Cpp singleton already exists.");
	singleton = this;
}

CppLanguage::~CppLanguage() {
	singleton = nullptr;
}

CppScript::CppScript() : script_list(this) {
	valid = false;
	library_handle = nullptr;
	{
		MutexLock lock(CppLanguage::get_singleton()->mutex);
		CppLanguage::get_singleton()->script_list.add(&script_list);
	}
}
void CppLanguage::reload_all_scripts() {
	print_line("C++: Compiling all scripts");
	Array scripts;
	{
		MutexLock lock(mutex);
		for (CppScript &script : script_list) {
			if (script.get_script_path().is_resource_file()) {
#if DEBUG_ENABLED
				print_verbose("C++: Found: " + script.get_script_path());
#endif
				scripts.push_back(Ref<CppScript>(&script)); //cast to cppscript to avoid being erased by accident
			}
		}
	}
	reload_scripts(scripts);
}
void CppLanguage::reload_scripts(const Array &p_scripts) {
	List<Ref<CppScript>> scripts;
	{
		MutexLock lock(mutex);
		for (CppScript &script : script_list) {
			if (script.is_root_script() && !script.get_script_path().is_empty()) {
				scripts.push_back(Ref<CppScript>(&script)); //cast to cppscript to avoid being erased by accident
			}
		}
	}
	//when someone asks you why dynamically typed languages are easier to write....

	HashMap<Ref<CppScript>, HashMap<ObjectID, List<Pair<StringName, Variant>>>> to_reload;

	for (Ref<CppScript> &scr : scripts) {
		bool reload = p_scripts.has(scr) || to_reload.has(scr->get_base());
		if (!reload) {
			continue;
		}
		to_reload.insert(scr, HashMap<ObjectID, List<Pair<StringName, Variant>>>());
	}

	for (KeyValue<Ref<CppScript>, HashMap<ObjectID, List<Pair<StringName, Variant>>>> &E : to_reload) {
		Ref<CppScript> scr = E.key;
		print_verbose("C++: Reloading: " + scr->get_script_path());
		if (scr->is_built_in()) {
			// TODO: It would be nice to do it more efficiently than loading the whole scene again.
			Ref<PackedScene> scene = ResourceLoader::load(scr->get_script_path().get_slice("::", 0), "", ResourceFormatLoader::CACHE_MODE_IGNORE_DEEP);
			ERR_CONTINUE(scene.is_null());

			Ref<SceneState> state = scene->get_state();
			Ref<CppScript> fresh = state->get_sub_resource(scr->get_script_path());
			ERR_CONTINUE(fresh.is_null());
			scr->set_source_code(fresh->get_source_code());
		} else {
			scr->load_source_code(scr->get_script_path());
		}
		scr->reload(true);
	}
}
void CppLanguage::reload_tool_script(const Ref<Script> &p_script) {
}
void CppLanguage::get_public_functions(List<MethodInfo> *r_functions) const {
}
void CppLanguage::get_public_constants(List<Pair<String, Variant>> *r_constants) const {
}
void CppLanguage::get_public_annotations(List<MethodInfo> *r_annotations) const {
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
bool CppScript::has_header_code() const {
	return !header_source.is_empty();
}
String CppScript::get_header_code() const {
	return header_source;
}
void CppScript::set_header_code(const String &p_code) {
	header_source = p_code;
}
String CppScript::get_script_path() const {
	return path;
}

bool CppScript::can_instantiate() const {
	if (!path_valid) {
		ERR_FAIL_V_MSG(false, "Cannot instantiate C++ script because the associated class could not be found. Script: '" + get_script_path() + "'. Make sure the script exists and contains a class definition with a name that matches the filename of the script exactly (it's case-sensitive).");
	}

	return path_valid;
}
Ref<Script> CppScript::get_base_script() const {
	return base_script;
}
StringName CppScript::get_global_name() const {
	return StringName();
}
bool CppScript::inherits_script(const Ref<Script> &p_script) const {
	Ref<CppScript> gd = p_script;
	if (gd.is_null()) {
		return false;
	}

	const CppScript *s = this;

	while (s) {
		if (s == p_script.ptr()) {
			return true;
		}
		s = s->base.ptr();
	}

	return false;
}
StringName CppScript::get_instance_base_type() const {
	return StringName();
} // this may not work in all scripts, will return empty if so
CppScriptInstance *CppScript::_create_instance(const Variant **p_args, int p_argcount, Object *p_owner, Callable::CallError &r_error) {
	ERR_FAIL_COND_V_MSG(!can_instantiate(), nullptr, "Cannot instantiate C++ script. Script: '" + get_script_path() + "'.");

	/* STEP 1, CREATE */

	CppScriptInstance *instance = memnew(CppScriptInstance(Ref<CppScript>(this)));
	instance->owner = p_owner;
	instance->owner->set_script_instance(instance);
	/* STEP 2, PARTY */

	//@TODO make thread safe
	return instance;
}
static void _copy_node_state(Node *p_from, Node *p_to) {
	List<PropertyInfo> props;
	p_from->get_property_list(&props);
	for (const PropertyInfo &pi : props) {
		if ((pi.usage & PROPERTY_USAGE_STORAGE) && pi.name != "script") {
			p_to->set(pi.name, p_from->get(pi.name));
		}
	}
}
ScriptInstance *CppScript::instance_create(Object *p_this) {
	if (!is_script_valid()) {
		reload();
	}
	ERR_FAIL_COND_V_MSG(!is_script_valid(), nullptr, "Script is invalid!");
	ERR_FAIL_COND_V_MSG(_is_abstract, nullptr, "Can't instantiate an abstract class.");
	Callable::CallError unchecked_error;
	CppScriptInstance *instance = _create_instance(nullptr, 0, p_this, unchecked_error);
	Node *node = Object::cast_to<Node>(p_this);
	if (node && ClassDB::class_exists(cname) && node->get_class_name() != cname) {
		callable_mp(this, &CppScript::_fix_node_type).call_deferred(node->get_instance_id());
	}
	return instance;
}
void CppScript::_fix_node_type(ObjectID p_id) {
	Node *old_node = Object::cast_to<Node>(ObjectDB::get_instance(p_id));
	if (!old_node || old_node->get_class_name() == cname) {
		return;
	}
	bool is_editor = Engine::get_singleton()->is_editor_hint();
#ifdef TOOLS_ENABLED
	if (is_editor && EditorNode::get_singleton() && EditorNode::get_singleton()->get_edited_scene() == old_node) {
		if (!_swap_edited_root(old_node)) {
			ERR_PRINT(vformat("Could not change the root type of '%s'. Use Change Type -> '%s'.", get_script_path(), cname));
		}
		return;
	}
#endif
	SceneTree *tree = old_node->get_tree();
	bool is_root = !old_node->get_parent() || (tree && tree->get_current_scene() == old_node);
	if (is_root) {
		ERR_PRINT(vformat("C++ script '%s' is on a root node of type %s. Save the scene with node type '%s'.",
				get_script_path(), old_node->get_class(), cname));
		return;
	}
	if (!_swap_child(old_node)) {
		ERR_PRINT(vformat("Could not swap '%s' to '%s'.", old_node->get_name(), cname));
	}
}
bool CppScript::_swap_child(Node *p_old) {
	Node *n = Object::cast_to<Node>(ClassDB::instantiate(cname));
	ERR_FAIL_NULL_V(n, false);
	_copy_node_state(p_old, n);
	p_old->replace_by(n);
	p_old->queue_free();
	return true;
}
#ifdef TOOLS_ENABLED
static void _reown(Node *p_node, Node *p_from, Node *p_to) {
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *c = p_node->get_child(i);
		if (c->get_owner() == p_from) {
			c->set_owner(p_to);
		}
		_reown(c, p_from, p_to);
	}
}
bool CppScript::_swap_edited_root(Node *p_old) {
	Node *n = Object::cast_to<Node>(ClassDB::instantiate(cname));
	ERR_FAIL_NULL_V(n, false);
	_copy_node_state(p_old, n);
	n->set_scene_file_path(p_old->get_scene_file_path());
	Vector<Node *> kids;
	for (int i = 0; i < p_old->get_child_count(); i++) {
		kids.push_back(p_old->get_child(i));
	}
	for (Node *kid : kids) {
		p_old->remove_child(kid);
		n->add_child(kid);
		if (kid->get_owner() == p_old) {
			kid->set_owner(n);
		}
		_reown(kid, p_old, n);
	}
	EditorNode::get_singleton()->set_edited_scene(n);
	p_old->queue_free();
	return true;
}
#endif
bool CppScript::has_source_code() const {
	return !source.is_empty();
}
String CppScript::get_source_code() const {
	return source;
}
void CppScript::set_source_code(const String &p_code) {
	source = p_code;
}
String get_sdk_path() {
	return OS::get_singleton()->get_executable_path().get_base_dir().path_join("godot_cpp_sdk");
}
void CppScript::build_compile_arguments(const String &p_source, const String &p_output, List<String> &r_args) const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	String sdk = get_sdk_path();
	String glue = ps->globalize_path("res://.godot/cpp_build").path_join(p_source.get_file().get_basename() + ".cpp");
	r_args.push_back("-shared");
	r_args.push_back("-fPIC");
	r_args.push_back("-std=c++17");
	r_args.push_back("-DTOOLS_ENABLED");
	r_args.push_back("-DDEBUG_ENABLED");
	r_args.push_back("-DHOT_RELOAD_ENABLED");
	r_args.push_back("-I");
	r_args.push_back(sdk.path_join("include"));
	r_args.push_back("-I");
	r_args.push_back(sdk.path_join("gdextension"));
	r_args.push_back("-I");
	r_args.push_back(ps->globalize_path(p_source.get_base_dir()));
	r_args.push_back("-I");
	r_args.push_back(ps->globalize_path("res://.godot/cpp_build"));
	r_args.push_back(ps->globalize_path(p_source));
	r_args.push_back(glue);
	r_args.push_back(sdk.path_join("lib/libgodot-cpp.a"));
	r_args.push_back("-o");
	r_args.push_back(ps->globalize_path(p_output));
}
String get_compiler_path() {
#ifdef TOOLS_ENABLED
	Ref<EditorSettings> settings = EditorSettings::get_singleton();
	if (settings.is_valid()) {
		Variant value = settings->get("cpp/compiler_path");
		if (value.get_type() == Variant::STRING) {
			String configured = value;
			if (!configured.is_empty()) {
				return configured;
			}
		}
	}
#endif
	return "clang++";
}
void CppScript::update_exports() {
}
#ifdef WINDOWS_ENABLED
static constexpr const char *LIB_SUFFIX = ".dll";
#elif defined(MACOS_ENABLED)
static constexpr const char *LIB_SUFFIX = ".dylib";
#else
static constexpr const char *LIB_SUFFIX = ".so";
#endif
Error CppScript::reload(bool p_keep_state) {
	if (_is_reloading) {
		return ERR_ALREADY_IN_USE;
	}
	_is_reloading = true;
	if (path.get_extension() != "cpp") {
		valid = true;
		_is_reloading = false;
		return OK;
	}
	String cname_str = path.get_file().get_basename().replace_char(' ', '_');
	cname = cname_str;
	if (bindings_source.is_empty()) {
		bindings_source = make_glue_source(cname);
	}
	if (gdextension_source.is_empty()) {
		gdextension_source = make_gdextension_source(cname);
	}
	ProjectSettings *ps = ProjectSettings::get_singleton();
	String fname = path.get_file().get_basename();
	String build_dir = "res://.godot/cpp_build";
	DirAccess::make_dir_recursive_absolute(ps->globalize_path(build_dir));
	String ext = build_dir.path_join(fname + ".gdextension");
	String output_path = build_dir.path_join(fname + LIB_SUFFIX);
	String hash_path = ps->globalize_path(build_dir).path_join(fname + ".hash");
	String h_str = String::num_uint64(source.hash64() ^ (header_source.hash64() * 31), 16);
	GDExtensionManager *mgr = GDExtensionManager::get_singleton();
	CppLanguage *lang = CppLanguage::get_singleton();
	bool loaded = mgr->is_extension_loaded(ext);
	if (loaded && lang->built_hashes.has(fname) && lang->built_hashes[fname] == h_str) {
		valid = true;
		_is_reloading = false;
		return OK;
	}
	String stored = FileAccess::exists(hash_path) ? FileAccess::get_file_as_string(hash_path).strip_edges() : String();
	if (!loaded && stored == h_str &&
			FileAccess::exists(ps->globalize_path(output_path)) &&
			FileAccess::exists(ps->globalize_path(ext))) {
		GDExtensionManager::LoadStatus st = mgr->load_extension(ext);
		if (st != GDExtensionManager::LOAD_STATUS_FAILED) {
			lang->built_hashes[fname] = h_str;
			valid = true;
			_is_reloading = false;
			return OK;
		}
	}
#ifdef TOOLS_ENABLED
	if (loaded && Engine::get_singleton()->is_editor_hint() &&
			EditorInterface::get_singleton() &&
			EditorInterface::get_singleton()->is_playing_scene()) {
		valid = true;
		_is_reloading = false;
		return OK;
	}
#endif
	auto write_file = [&](const String &p_path, const String &p_content) -> Error {
		Error e;
		Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE, &e);
		ERR_FAIL_COND_V_MSG(e != OK, e, "Cannot write '" + p_path + "'.");
		f->store_string(p_content);
		return OK;
	};
	Error err = write_file(ps->globalize_path("res://.godot/cpp_build").path_join(fname + ".cpp"), bindings_source);
	if (err != OK) {
		valid = false;
		_is_reloading = false;
		return err;
	}
	err = write_file(build_dir.path_join(fname + ".gdextension"), gdextension_source);
	if (err != OK) {
		valid = false;
		_is_reloading = false;
		return err;
	}
	List<String> args;
	build_compile_arguments(get_script_path(), output_path, args);
	String output;
	int exit_code = 0;
	err = OS::get_singleton()->execute(get_compiler_path(), args, &output, &exit_code, true);
	if (err != OK || exit_code != 0) {
		valid = false;
		_is_reloading = false;
		print_error(output);
		return ERR_COMPILATION_FAILED;
	}
	GDExtensionManager::LoadStatus status = mgr->is_extension_loaded(ext) ? mgr->reload_extension(ext) : mgr->load_extension(ext);
	print_line("C++ reload status=", (int)status, " class_exists=", ClassDB::class_exists(cname));
	if (status == GDExtensionManager::LOAD_STATUS_FAILED) {
		valid = false;
		_is_reloading = false;
		return FAILED;
	}
	Ref<FileAccess> hf = FileAccess::open(hash_path, FileAccess::WRITE);
	if (hf.is_valid()) {
		hf->store_string(h_str);
	}
	lang->built_hashes[fname] = h_str;
	valid = true;
	_is_reloading = false;
	return OK;
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
Error CppScript::load_source_code(const String &p_path) {
	if (p_path.is_empty() || p_path.begins_with("cpp://") || ResourceLoader::get_resource_type(p_path.get_slice("::", 0)) == "PackedScene") {
		return OK;
	}
	valid = false;
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
	header_source = "";
	err = OK;
	Ref<FileAccess> header = FileAccess::open(p_path.get_basename() + ".hpp", FileAccess::READ, &err);
	if (!err) {
		len = header->get_length();
		sourcef.resize(len + 1);
		w = sourcef.ptrw();
		r = header->get_buffer(w, len);
		ERR_FAIL_COND_V(r != len, ERR_CANT_OPEN);
		w[len] = 0;
		s = "";
		if (s.append_utf8((const char *)w, len) != OK) {
			ERR_FAIL_V_MSG(ERR_INVALID_DATA,
					"Header '" + p_path.get_basename() +
							".hpp' contains invalid UTF-8.");
		}
		header_source = s;
	}
	bindings_source = "";
	String pname = p_path.get_file().get_basename();
	String build_dir = ProjectSettings::get_singleton()->globalize_path("res://.godot/cpp_build");
	Error gerr;
	bindings_source = FileAccess::get_file_as_string(build_dir.path_join(pname + ".cpp"), &gerr);
	gdextension_source = FileAccess::get_file_as_string(build_dir.path_join(pname + ".gdextension"), &gerr);
	path = p_path;
	path_valid = true;
#ifdef TOOLS_ENABLED
	set_edited(false);
	set_last_modified_time(FileAccess::get_modified_time(path));
#endif // TOOLS_ENABLED
	return reload();
}
