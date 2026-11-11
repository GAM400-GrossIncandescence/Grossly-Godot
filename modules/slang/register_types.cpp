#include "register_types.h"

#include "core/config/engine.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"

#include "modules/register_module_types.h"
#include "modules/slang/slang_resource_format_loader.h"
#include "modules/slang/slang_server.h"
#include "modules/slang/slang_spirv.h"

static Ref<SlangResourceFormatLoader> format_loader{ nullptr };

void initialize_slang_module(ModuleInitializationLevel p_level) {
	GDREGISTER_CLASS(SlangServer);
	GDREGISTER_CLASS(SlangResourceFormatLoader);
	GDREGISTER_CLASS(SlangSPIRV);

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		format_loader = memnew(SlangResourceFormatLoader);
		ResourceLoader::add_resource_format_loader(format_loader);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		Engine::Singleton singleton{
			"SlangServer",
			memnew(SlangServer),
			"SlangServer",
		};

		singleton.editor_only = true;

		Engine::get_singleton()->add_singleton(singleton);

		SlangServer::get_singleton()->create_global_session();
	}
}

void uninitialize_slang_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ResourceLoader::remove_resource_format_loader(format_loader);
		format_loader = nullptr;
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		Engine::get_singleton()->remove_singleton("SlangServer");
	}
}
