#pragma once

#if TOOLS_ENABLED

#include "core/object/object.h"

namespace slang {
struct IGlobalSession;
}

class SlangServer : public Object {
	GDCLASS(SlangServer, Object);

protected:
	static void _bind_methods();

private:
	inline static SlangServer *singleton{ nullptr };

public:
	static SlangServer *get_singleton();

	void create_global_session();

	[[nodiscard]] slang::IGlobalSession *get_global_session() const;

	[[nodiscard]] bool is_available() const;

	SlangServer();

	~SlangServer();
};
#endif
