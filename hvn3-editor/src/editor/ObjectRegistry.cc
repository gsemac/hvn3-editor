#pragma once

#include "editor/ObjectRegistry.h"

namespace hvn3 {
	namespace editor {

		IObjectPtr ObjectRegistry::MakeObject(const std::string& key) const {

			return _registry.at(key)->New();

		}
		ObjectRegistry::registry_type::iterator ObjectRegistry::begin() {
			return _registry.begin();
		}
		ObjectRegistry::registry_type::iterator ObjectRegistry::end() {
			return _registry.end();
		}

	}
}