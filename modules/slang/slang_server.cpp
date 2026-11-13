#include "slang_server.h"

#if TOOLS_ENABLED
#include "slang.h"

#include <slang-com-ptr.h>
#endif

void SlangServer::_bind_methods() {
}

SlangServer *SlangServer::get_singleton() {
	return singleton;
}

#if TOOLS_ENABLED

String SlangServer::blob_to_string(const Slang::ComPtr<ISlangBlob> &blob) {
	if (blob == nullptr) {
		return "";
	}

	const void *buffer{ blob->getBufferPointer() };
	size_t length{ blob->getBufferSize() / sizeof(char) };

	print_line("Blob: ", blob->getBufferSize());

	return String::ascii(Span<char>(static_cast<const char *>(buffer), length));
}

#endif

SlangServer::SlangServer() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

SlangServer::~SlangServer() {}
