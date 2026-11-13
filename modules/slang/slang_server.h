#pragma once

#if TOOLS_ENABLED

#include "core/object/object.h"

namespace slang {
struct IGlobalSession;
}

struct ISlangBlob;

namespace Slang {
template <typename T>
class ComPtr;
}

class SlangServer : public Object {
	GDCLASS(SlangServer, Object);

protected:
	static void _bind_methods();

private:
	inline static SlangServer *singleton{ nullptr };

public:
	static SlangServer *get_singleton();

#if TOOLS_ENABLED
	static String blob_to_string(const Slang::ComPtr<ISlangBlob> &blob);
#endif

	SlangServer();

	~SlangServer();
};

#endif
