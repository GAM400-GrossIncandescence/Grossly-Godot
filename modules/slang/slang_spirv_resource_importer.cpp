#if TOOLS_ENABLED

#include "slang_spirv_resource_importer.h"

#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/io/file_access.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"
#include "core/object/property_info.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "servers/rendering/rendering_device_binds.h"

#include <slang-com-ptr.h>
#include <slang.h>

#include <array>

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
	return RDShaderSPIRV::get_class_static();
}

float ResourceImporterSlangSPIRV::get_priority() const {
	return 1.0;
}
int ResourceImporterSlangSPIRV::get_import_order() const {
	return IMPORT_ORDER_DEFAULT;
}
int ResourceImporterSlangSPIRV::get_format_version() const {
	return 0;
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

	ERR_FAIL_COND_V(source.is_empty(), FileAccess::get_open_error());

	Ref<RDShaderSPIRV> shader = memnew(RDShaderSPIRV);

	if (type == ShaderType::Compute) {
		Vector<uint8_t> spirvBytes;

		print_verbose("Compiling slang shader: " + p_source_file);

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

namespace {
static thread_local slang::IGlobalSession *global_session{};
}

Error ResourceImporterSlangSPIRV::try_compile_slang(
		CharString p_source_file,
		CharString p_source,
		CharString p_entry_point,
		Vector<uint8_t> &r_spirv) {
	print_verbose("SlangImport: getting global session");

	if (global_session == nullptr) {
		SlangGlobalSessionDesc desc{};
		desc.enableGLSL = false;
		desc.apiVersion = 0;
		SlangResult result = slang::createGlobalSession(&desc, &global_session);
		ERR_FAIL_COND_V(result != SLANG_OK || global_session == nullptr, ERR_UNAVAILABLE);
	}

	print_verbose("SlangImport: creating session");

	Slang::ComPtr<slang::ISession> session{};
	{
		slang::TargetDesc target{};
		target.format = SlangCompileTarget::SLANG_SPIRV;
		target.profile = global_session->findProfile(SPIRV_PROFILE);

		slang::SessionDesc session_desc;
		session_desc.targets = &target;
		session_desc.targetCount = 1;

		SlangResult result = global_session->createSession(session_desc, session.writeRef());

		ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::FAILED, "Failed to get slang global session");
	}

	print_verbose("SlangImport: loading module");

	Slang::ComPtr<slang::IModule> module_{};
	{
		Slang::ComPtr<ISlangBlob> diagnostic{};
		module_ = session->loadModuleFromSourceString(
				"compute_shader",
				p_source_file.ptr(),
				p_source.ptr(),
				diagnostic.writeRef());

		ERR_FAIL_NULL_V_MSG(module_, ERR_COMPILATION_FAILED, "Failed to load module: " + blob_to_string(diagnostic));
	}

	print_verbose("SlangImport: finding entry point");
	Slang::ComPtr<slang::IEntryPoint> entry{};
	{
		SlangResult result = module_->findEntryPointByName(p_entry_point.ptr(), entry.writeRef());
		ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_COMPILATION_FAILED, "Failed to find entry point");
	}

	print_verbose("SlangImport: creating composite program");

	Slang::ComPtr<slang::IComponentType> program{};
	{
		std::array<slang::IComponentType *const, 2> components{
			*module_.readRef(),
			*entry.readRef(),
		};

		Slang::ComPtr<ISlangBlob> diagnostic{};
		SlangResult result = session->createCompositeComponentType(
				components.data(),
				components.size(),
				program.writeRef(),
				diagnostic.writeRef());

		ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_COMPILATION_FAILED, "Failed to create slang composite components during compilation: " + blob_to_string(diagnostic));
	}

	print_verbose("SlangImport: linking program");

	Slang::ComPtr<slang::IComponentType> linked_program{};
	{
		Slang::ComPtr<ISlangBlob> diagnostic{};
		SlangResult result = program->link(linked_program.writeRef(), diagnostic.writeRef());

		ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_LINK_FAILED, "Failed to link program: " + blob_to_string(diagnostic));
	}

	print_verbose("SlangImport: getting SpirV from shader");

	Slang::ComPtr<ISlangBlob> spirv_blob{};
	{
		Slang::ComPtr<ISlangBlob> diagnostic{};
		SlangResult result = program->getEntryPointCode(0, 0, spirv_blob.writeRef(), diagnostic.writeRef());

		ERR_FAIL_COND_V_MSG(result != SLANG_OK, Error::ERR_LINK_FAILED, "Failed to find entry point in linked slang program: " + blob_to_string(diagnostic));
	}

	r_spirv.clear();
	r_spirv.reserve(spirv_blob->getBufferSize());

	for (size_t i = 0; i < spirv_blob->getBufferSize(); i++) {
		r_spirv.push_back(static_cast<const uint8_t *>(spirv_blob.get()->getBufferPointer())[i]);
	}

	return Error::OK;
}

String ResourceImporterSlangSPIRV::blob_to_string(const Slang::ComPtr<ISlangBlob> &blob) {
	if (blob == nullptr) {
		return "";
	}

	const void *buffer{ blob->getBufferPointer() };
	size_t length{ blob->getBufferSize() / sizeof(char) };

	print_line("Blob: ", blob->getBufferSize());

	return String::ascii(Span<char>(static_cast<const char *>(buffer), length));
}

#endif
