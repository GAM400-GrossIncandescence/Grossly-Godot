#pragma once

#include "servers/rendering/rendering_device_binds.h"

class SlangSPIRV : public RDShaderSPIRV {
	GDCLASS(SlangSPIRV, RDShaderSPIRV);

protected:
	static void _bind_methods();

private:
	String source;

public:
	const String &get_source();
};
