/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"
#include "cpp_script_resource_format.h"
#include "cpp_script.h"

#ifdef TOOLS_ENABLED
// highlighter, lsp
#endif // TOOLS_ENABLED

#ifdef TESTS_ENABLED
// tests
#endif

#include "core/io/file_access.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/object/class_db.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/export/editor_export.h"
#include "editor/translations/editor_translation_parser.h"

#ifndef CPP_NO_LSP
#include "core/config/engine.h"
#endif
#endif // TOOLS_ENABLED

#ifdef TESTS_ENABLED
#include "tests/test_macros.h"
#endif

CppLanguage *script_language_cpp = nullptr;
Ref<ResourceFormatLoaderCppScript> resource_loader_cpp;
Ref<ResourceFormatSaverCppScript> resource_saver_cpp;

#ifdef TOOLS_ENABLED

class CppScriptExportPlugin : public EditorExportPlugin {
	GDSOFTCLASS(CppScriptExportPlugin, EditorExportPlugin);

	static constexpr EditorExportPreset::ScriptExportMode DEFAULT_SCRIPT_MODE = EditorExportPreset::MODE_SCRIPT_BINARY_TOKENS_COMPRESSED;
	EditorExportPreset::ScriptExportMode script_mode = DEFAULT_SCRIPT_MODE;

protected:
	virtual void _export_begin(const HashSet<String> &p_features, bool p_debug, const String &p_path, int p_flags) override {
		script_mode = DEFAULT_SCRIPT_MODE;

		const Ref<EditorExportPreset> &preset = get_export_preset();
		if (preset.is_valid()) {
			script_mode = preset->get_script_export_mode();
		}
	}

	virtual void _export_file(const String &p_path, const String &p_type, const HashSet<String> &p_features) override {
		if (p_path.get_extension() != "cpp" || script_mode == EditorExportPreset::MODE_SCRIPT_TEXT) {
			return;
		}

		Vector<uint8_t> file = FileAccess::get_file_as_bytes(p_path);
		if (file.is_empty()) {
			return;
		}

		String source = String::utf8(reinterpret_cast<const char *>(file.ptr()), file.size());
		// compress
		// add_file(p_path.get_basename() + ".cpp", file, true);
	}

public:
	virtual String get_name() const override { return "C++"; }
};

static void _editor_init() {
	// instantiate export?
#ifdef TOOLS_ENABLED
	// syntax highlighter
#endif
}

#endif // TOOLS_ENABLED

void initialize_clang_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		GDREGISTER_CLASS(CppScript);

		script_language_cpp = memnew(CppLanguage);
		ScriptServer::register_language(script_language_cpp);
		resource_loader_cpp.instantiate();
		ResourceLoader::add_resource_format_loader(resource_loader_cpp);

		resource_saver_cpp.instantiate();
		ResourceSaver::add_resource_format_saver(resource_saver_cpp);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// editor init
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// syntax highlighter setup
#ifndef CLANG_NO_LSP
	// clang lsp setup
#endif // !CLANG_NO_LSP
	}
#endif // TOOLS_ENABLED
}

void uninitialize_clang_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		if (resource_loader_cpp.is_valid()) {
			ResourceLoader::remove_resource_format_loader(resource_loader_cpp);
			resource_loader_cpp.unref();
		}

		if (resource_saver_cpp.is_valid()) {
			ResourceSaver::remove_resource_format_saver(resource_saver_cpp);
			resource_saver_cpp.unref();
		}

		if (script_language_cpp) {
			ScriptServer::unregister_language(script_language_cpp);
			memdelete(script_language_cpp);
			script_language_cpp = nullptr;
		}
	}

    if (script_language_cpp) {
        ScriptServer::unregister_language(script_language_cpp);
        memdelete(script_language_cpp);
        script_language_cpp = nullptr;
	}
}

#ifdef TESTS_ENABLED
// todo: test compiler
#endif

