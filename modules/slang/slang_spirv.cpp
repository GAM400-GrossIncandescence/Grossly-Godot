#include "slang_spirv.h"

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/property_info.h"

void SlangSPIRV::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_source"), &SlangSPIRV::get_source);

	// jesus christ godot's formatter is ugly
	ADD_PROPERTY(
			PropertyInfo(
					Variant::STRING,
					"source",
					PROPERTY_HINT_NONE,
					"",
					PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY),
			"",
			"get_source");
}
void SlangSPIRV::set_source(String source) {}
void SlangSPIRV::set_entry_point(String entry_point) {}

const String &SlangSPIRV::get_entry_point() const {
	return entry_point;
}

const String &SlangSPIRV::get_source() const {
	return source;
}
