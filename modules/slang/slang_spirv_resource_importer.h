#pragma once

#include "core/error/error_list.h"
#include "core/io/resource_importer.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

#if TOOLS_ENABLED

class ResourceImporterSlangSPIRV final : public ResourceImporter {
	GDCLASS(ResourceImporterSlangSPIRV, ResourceImporter);

public:
	enum class ShaderType {
		Compute,
		Library,
		// We do not support slang shaders for tesselation, fragment, or vertex
	};

	String get_importer_name() const override;

	String get_visible_name() const override;

	void get_recognized_extensions(List<String> *p_extensions) const override;

	String get_save_extension() const override;

	String get_resource_type() const override;

	float get_priority() const override;

	int get_import_order() const override;

	int get_format_version() const override;

	bool can_import_threaded() const override;

	String get_preset_name(int p_idx) const override;

	int get_preset_count() const override;

	void get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset = 0) const override;

	bool get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const override;

	Error import(
			ResourceUID::ID p_source_id,
			const String &p_source_file,
			const String &p_save_path,
			const HashMap<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = nullptr,
			Variant *r_metadata = nullptr) override;

public:
	static Error try_compile_slang(CharString p_source_file, CharString p_source, CharString p_entry_point, Vector<uint8_t> &r_spirv);
};

#endif
