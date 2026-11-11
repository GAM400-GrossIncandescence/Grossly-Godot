#include "slang_resource_format_loader.h"

#include "slang.h"

#include "core/os/memory.h"
#include "servers/rendering/rendering_device_binds.h"

Ref<Resource> SlangResourceFormatLoader::load(
		const String &p_path,
		const String &p_original_path,
		Error *r_error,
		bool p_use_sub_threads,
		float *r_progress,
		CacheMode p_cache_mode) {
	return memnew(RDShaderSPIRV);
}

bool SlangResourceFormatLoader::recognize_path(const String &p_path, const String &p_for_type) const {
	return p_path.ends_with("slang");
}

bool SlangResourceFormatLoader::handles_type(const String &p_type) const {
	return p_type == "RDShaderSPIRV";
}
