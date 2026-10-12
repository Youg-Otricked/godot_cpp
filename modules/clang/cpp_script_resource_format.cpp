/**************************************************************************/
/*  cpp_script_resource_format.cpp                                        */
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

#include "cpp_script_resource_format.h"
#ifdef TOOLS_ENABLED
#include "core/config/project_settings.h"
#endif
#include "core/io/dir_access.h"
#include "core/io/file_access.h"

#include "modules/clang/cpp_script.h"

Ref<Resource> ResourceFormatLoaderCppScript::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	Error err;
	Ref<CppScript> scr = memnew(CppScript);
	err = scr->load_source_code(p_path);
	if (err && scr.is_valid()) {
		// If !scr.is_valid(), the error was likely from scr->load_source_code(), which already generates an error.
		ERR_PRINT_ED(vformat(R"(Failed to load script "%s" with error "%s".)", p_original_path, error_names[err]));
	}
	if (r_error) {
		*r_error = scr.is_valid() ? OK : err;
	}

	return scr;
}

void ResourceFormatLoaderCppScript::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("cpp");
	p_extensions->push_back("cc");
	p_extensions->push_back("cxx");
	p_extensions->push_back("hpp");
	p_extensions->push_back("h");
	p_extensions->push_back("hxx");
}

bool ResourceFormatLoaderCppScript::handles_type(const String &p_type) const {
	return p_type == "Script" || p_type == "CppScript" || p_type == CppLanguage::get_singleton()->get_type();
}

String ResourceFormatLoaderCppScript::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	return (el == "cpp" || el == "cc" || el == "cxx") ? String("CppScript") : String();
}

Error ResourceFormatSaverCppScript::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<CppScript> sqscr = p_resource;
	ERR_FAIL_COND_V(sqscr.is_null(), ERR_INVALID_PARAMETER);

	String source = sqscr->get_source_code();

	{
		Error err;
		Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);

		ERR_FAIL_COND_V_MSG(err, err, "Cannot save C++ file '" + p_path + "'.");

		file->store_string(source);
		if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
			return ERR_CANT_CREATE;
		}
		if (sqscr->has_header_code()) {
			String header_source = sqscr->get_header_code();
			Ref<FileAccess> header = FileAccess::open(p_path.get_basename() + ".hpp", FileAccess::WRITE, &err);
			ERR_FAIL_COND_V_MSG(err, err, "Cannot save C++ Header file '" + p_path.get_basename() + ".hpp" + "'.");
			header->store_string(header_source);
			if (header->get_error() != OK && header->get_error() != ERR_FILE_EOF) {
				return ERR_CANT_CREATE;
			}
		}
		ProjectSettings *ps = ProjectSettings::get_singleton();
		String name = p_path.get_file().get_basename();
		String build_dir = ps->globalize_path("res://.godot/cpp_build");
		DirAccess::make_dir_recursive_absolute(build_dir);

		if (sqscr->has_bindings_source()) {
			Ref<FileAccess> glue = FileAccess::open(build_dir.path_join(name + ".cpp"), FileAccess::WRITE, &err);
			ERR_FAIL_COND_V_MSG(err, err, "Cannot save C++ glue file for '" + name + "'.");
			glue->store_string(sqscr->get_bindings_source());
		}
		if (sqscr->has_gdextension_source()) {
			Ref<FileAccess> gd = FileAccess::open(build_dir.path_join(name + ".gdextension"), FileAccess::WRITE, &err);
			ERR_FAIL_COND_V_MSG(err, err, "Cannot save gdextension file for '" + name + "'.");
			gd->store_string(sqscr->get_gdextension_source());
		}
	}

	if (ScriptServer::is_reload_scripts_on_save_enabled()) {
		CppLanguage::get_singleton()->reload_tool_script(p_resource);
	}

	return OK;
}

void ResourceFormatSaverCppScript::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	if (Object::cast_to<CppScript>(*p_resource)) {
		p_extensions->push_back("cpp");
		p_extensions->push_back("cc");
		p_extensions->push_back("cxx");
		p_extensions->push_back("hpp");
		p_extensions->push_back("h");
		p_extensions->push_back("hxx");
	}
}

bool ResourceFormatSaverCppScript::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<CppScript>(*p_resource) != nullptr;
}
