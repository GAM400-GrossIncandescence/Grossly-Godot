#pragma once

#include "slang.h"

#include "core/object/object.h"

#include <slang-com-ptr.h>

class SlangServer : public Object {
	GDCLASS(SlangServer, Object);

protected:
	static void _bind_methods();

private:
	inline static SlangServer *singleton{ nullptr };

public:
	static SlangServer *get_singleton();

	void create_global_session();

	slang::IGlobalSession *get_global_session();

	SlangServer();

	~SlangServer();
};
