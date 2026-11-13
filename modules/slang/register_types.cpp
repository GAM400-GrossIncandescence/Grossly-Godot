#include "register_types.h"

#include "core/config/engine.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"

#include "modules/register_module_types.h"
#include "modules/slang/slang_server.h"
#include "modules/slang/slang_spirv_resource_importer.h"

static Ref<ResourceImporterSlangSPIRV> format_loader{ nullptr };

void initialize_slang_module(ModuleInitializationLevel p_level) {
	GDREGISTER_CLASS(SlangServer);
	GDREGISTER_CLASS(ResourceImporterSlangSPIRV);

	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		format_loader.instantiate();
		ResourceFormatImporter::get_singleton()->add_importer(format_loader);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		Engine::Singleton singleton{
			SlangServer::get_class_static(),
			memnew(SlangServer),
			SlangServer::get_class_static()
		};

		singleton.editor_only = true;

		Engine::get_singleton()->add_singleton(singleton);
	}
}

void uninitialize_slang_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		ResourceFormatImporter::get_singleton()->remove_importer(format_loader);
		format_loader = nullptr;
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		Engine::get_singleton()->remove_singleton(SlangServer::get_class_static());
	}
}
