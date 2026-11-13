#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/io/file_access.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"
#include "core/object/property_info.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

#include "modules/slang/slang_server.h"
#include "modules/slang/slang_spirv.h"

#include <slang-com-ptr.h>
#include <slang.h>

#include <array>
#if TOOLS_ENABLED
#include "slang_spirv_resource_importer.h"

String ResourceImporterSlangSPIRV::get_importer_name() const {
	return "slang_shader";
}

String ResourceImporterSlangSPIRV::get_visible_name() const {
	return "SlangSPIRV Importer";
}

void ResourceImporterSlangSPIRV::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("slang");
}

String ResourceImporterSlangSPIRV::get_save_extension() const {
	return "res";
}

String ResourceImporterSlangSPIRV::get_resource_type() const {
	return "RDShaderSPIRV";
}

float ResourceImporterSlangSPIRV::get_priority() const {
	return 1.0;
}
int ResourceImporterSlangSPIRV::get_import_order() const {
	return 0;
}
int ResourceImporterSlangSPIRV::get_format_version() const {
	return 0;
}
bool ResourceImporterSlangSPIRV::can_import_threaded() const {
	return false;
}

String ResourceImporterSlangSPIRV::get_preset_name(int p_idx) const {
	return p_idx == 0 ? "Default" : "Unknown";
}

int ResourceImporterSlangSPIRV::get_preset_count() const {
	return 1;
}

void ResourceImporterSlangSPIRV::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption{
			PropertyInfo{
					Variant::Type::INT,
					"shader_type",
					PROPERTY_HINT_ENUM,
					"Compute,Library",
					PROPERTY_USAGE_EDITOR,
			},
			static_cast<int32_t>(ShaderType::Compute),
	});
	r_options->push_back(ImportOption{
			PropertyInfo{
					Variant::Type::STRING,
					"entry_point",
			},
			"main" });
}

bool ResourceImporterSlangSPIRV::get_option_visibility(
		const String &p_path,
		const String &p_option,
		const HashMap<StringName, Variant> &p_options) const {
	return true;
}
Error ResourceImporterSlangSPIRV::import(
		ResourceUID::ID p_source_id,
		const String &p_source_file,
		const String &p_save_path,
		const HashMap<StringName, Variant> &p_options,
		List<String> *r_platform_variants,
		List<String> *r_gen_files, Variant *r_metadata) {
	ShaderType type = p_options.has("shader_type") ? static_cast<ShaderType>(p_options.get("shader_type")) : ShaderType::Library;
	String entry_point = p_options.has("entry_point") ? static_cast<String>(p_options.get("entry_point")) : "main";

	String source = FileAccess::get_file_as_string(p_source_file);

	if (source.is_empty()) {
		return FileAccess::get_open_error();
	}

	Ref<RDShaderSPIRV> shader = memnew(RDShaderSPIRV);

	if (type == ShaderType::Compute) {
		Vector<uint8_t> spirvBytes;

		Error result = try_compile_slang(
				p_source_file.utf8(),
				source.utf8(),
				entry_point.utf8(),
				spirvBytes);

		if (result == Error::OK) {
			shader->set_stage_bytecode(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE, spirvBytes);
		}
	}

	return ResourceSaver::save(shader, p_save_path + "." + get_save_extension());
}
Error ResourceImporterSlangSPIRV::try_compile_slang(
		CharString p_source_file,
		CharString p_source,
		CharString p_entry_point,
		Vector<uint8_t> &r_spirv) {
	slang::TargetDesc target{
		.format = SlangCompileTarget::SLANG_SPIRV
	};

	slang::IGlobalSession *global_session = SlangServer::get_singleton()->get_global_session();

	ERR_FAIL_NULL_V(global_session, Error::ERR_FILE_MISSING_DEPENDENCIES);

	Slang::ComPtr<slang::ISession> session;
	SlangResult result = global_session->createSession(
			slang::SessionDesc{
					.targets = &target,
					.targetCount = 1,
			},
			session.writeRef());

	ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::FAILED, "Failed to get slang global session");

	r_spirv.clear();

	Slang::ComPtr<slang::IModule> module_;
	*module_.writeRef() = session->loadModuleFromSourceString(
			"compute_shader",
			p_source_file.ptr(),
			p_source.ptr());

	Slang::ComPtr<slang::IEntryPoint> entry;

	result = module_->findEntryPointByName(p_entry_point.ptr(), entry.writeRef());

	ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_COMPILATION_FAILED, "Failed to find entry point");

	std::array<slang::IComponentType *const, 2> components{
		*module_.readRef(), *entry.readRef()
	};

	Slang::ComPtr<slang::IComponentType> program{};

	result = session->createCompositeComponentType(
			components.data(),
			components.size(),
			program.writeRef());

	ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_COMPILATION_FAILED, "Failed to create slang composite components during compilation");

	Slang::ComPtr<slang::IComponentType> linked_program{};

	Slang::ComPtr<ISlangBlob> link_error;

	// TODO: [Kishan] error logging
	result = program->link(linked_program.writeRef(), link_error.writeRef());

	String error{};
	if (link_error) {
		error.append_utf8(Span<char>((char *)link_error.get()->getBufferPointer(), link_error.get()->getBufferSize()));
	}
	ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_LINK_FAILED, "Failed to link program: " + error);

	Slang::ComPtr<ISlangBlob> *blob{};
	result = program->getEntryPointCode(0, 0, blob->writeRef());

	ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_LINK_FAILED, "Failed to find entry point in linked slang program");

	r_spirv.clear();
	r_spirv.reserve(blob->get()->getBufferSize());

	for (size_t i = 0; i < blob->get()->getBufferSize(); i++) {
		r_spirv.push_back(static_cast<const uint8_t *>(blob->get()->getBufferPointer())[i]);
	}

	return Error::OK;
}

#endif
