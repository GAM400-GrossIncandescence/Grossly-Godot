#include "slang_server.h"

#include "slang.h"

#include "core/object/class_db.h"

#include <tuple>

void SlangServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_global_session"), &SlangServer::create_global_session);
}

SlangServer *SlangServer::get_singleton() {
	return singleton;
}
void SlangServer::create_global_session() {
	std::ignore = get_global_session();
}

slang::IGlobalSession *SlangServer::get_global_session() {
	thread_local Slang::ComPtr<slang::IGlobalSession> global_session = [] {
		Slang::ComPtr<slang::IGlobalSession> ptr;
		SlangGlobalSessionDesc desc = {};
		desc.enableGLSL = false;
		slang::createGlobalSession(&desc, ptr.writeRef());
		return ptr;
	}();

	return global_session.get();
}

SlangServer::SlangServer() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

SlangServer::~SlangServer() {}
