#pragma once

#include "core/error/error_list.h"
#include "servers/rendering/rendering_device_binds.h"

class SlangSPIRV : public RDShaderSPIRV {
	GDCLASS(SlangSPIRV, RDShaderSPIRV);

protected:
	static void _bind_methods();

private:
	String source;

	String entry_point;

public:
	void set_source(String source);

	const String &get_source() const;

	void set_entry_point(String entry_point);

	const String &get_entry_point() const;

	Error compile();
};
