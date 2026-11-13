#include "slang_server.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "editor/import/resource_importer_shader_file.h"

#if TOOLS_ENABLED
#include "slang.h"

#include <slang-com-ptr.h>
#endif

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

slang::IGlobalSession *SlangServer::get_global_session() const {
#if TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		return nullptr;
	}

	thread_local Slang::ComPtr<slang::IGlobalSession> global_session = [] {
		Slang::ComPtr<slang::IGlobalSession> ptr;
		SlangGlobalSessionDesc desc = {};
		desc.enableGLSL = false;
		slang::createGlobalSession(&desc, ptr.writeRef());
		return ptr;
	}();

	return global_session.get();
#else
	return nullptr;
#endif
}
[[nodiscard]] bool SlangServer::is_available() const {
#if TOOLS_ENABLED
	return get_global_session() != nullptr;
#else
	return Engine::get_singleton()->is_editor_hint();
#endif
}

SlangServer::SlangServer() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

SlangServer::~SlangServer() {}
