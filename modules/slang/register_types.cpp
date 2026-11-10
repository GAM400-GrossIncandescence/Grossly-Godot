#include "register_types.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"

#include "modules/slang/slang_resource_format_loader.h"

#include <memory>

void initialize_slang_module(ModuleInitializationLevel p_level) {
	GDREGISTER_CLASS(SlangResourceFormatLoader);

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ResourceLoader::add_resource_format_loader(memnew(SlangResourceFormatLoader));
	}
}

void uninitialize_slang_module(ModuleInitializationLevel p_level) {
}
