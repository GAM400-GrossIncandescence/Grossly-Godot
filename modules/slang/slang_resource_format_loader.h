#pragma once

#include "core/io/resource_loader.h"

class SlangResourceFormatLoader final : public ResourceFormatLoader {
	GDCLASS(SlangResourceFormatLoader, ResourceFormatLoader);

public:
	Ref<Resource> load(
			const String &p_path,
			const String &p_original_path,
			Error *r_error,
			bool p_use_sub_threads,
			float *r_progress,
			CacheMode p_cache_mode) override;

	bool recognize_path(const String &p_path, const String &p_for_type = String()) const override;

	bool handles_type(const String &p_type) const override;
};
