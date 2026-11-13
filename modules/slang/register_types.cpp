#include "register_types.h"

#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"

#include "modules/register_module_types.h"

#if TOOLS_ENABLED
#include "modules/slang/slang_spirv_resource_importer.h"
static Ref<ResourceImporterSlangSPIRV> format_loader{ nullptr };
#endif

void initialize_slang_module(ModuleInitializationLevel p_level) {
#if TOOLS_ENABLED
	GDREGISTER_CLASS(ResourceImporterSlangSPIRV);

	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		format_loader.instantiate();
		ResourceFormatImporter::get_singleton()->add_importer(format_loader);
	}
#endif
}

void uninitialize_slang_module(ModuleInitializationLevel p_level) {
#if TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		ResourceFormatImporter::get_singleton()->remove_importer(format_loader);
		format_loader = nullptr;
	}
#endif
}
