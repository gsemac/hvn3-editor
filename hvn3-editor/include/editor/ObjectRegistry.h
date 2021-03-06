#pragma once

#include "hvn3/objects/IObject.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace hvn3 {
	namespace editor {

		class ObjectRegistry {

			// Interface used for instantiating objects from the registry.
			class IObjectRegistryItem {

			public:
				// Creates a new instance of the object using its default constructor and returns a pointer to it.
				virtual IObjectPtr New() const = 0;
				// Returns the name of the object.
				virtual const std::string& Name() const = 0;

			};

			template<typename ObjectType>
			class ObjectRegistryItem :
				public IObjectRegistryItem {

			public:
				ObjectRegistryItem(const std::string& name);

				IObjectPtr New() const override;
				const std::string& Name() const override;

			private:
				std::string _name;

			};

		public:
			typedef std::unordered_map<std::string, std::shared_ptr<IObjectRegistryItem>> registry_type;

			ObjectRegistry() = default;

			// Adds a new object type to the registry.
			template<typename ObjectType>
			void RegisterObject(const std::string& name);

			// Creates a new instance of the object with the given name using its default constructor and returns a pointer to it.
			IObjectPtr MakeObject(const std::string& key) const;

			registry_type::iterator begin();
			registry_type::iterator end();

		private:
			registry_type _registry;

		};

		template<typename ObjectType>
		ObjectRegistry::ObjectRegistryItem<ObjectType>::ObjectRegistryItem(const std::string& name) :
			_name(name) {
		}
		template<typename ObjectType>
		IObjectPtr ObjectRegistry::ObjectRegistryItem<ObjectType>::New() const {
			return hvn3::make_object<ObjectType>();
		}
		template<typename ObjectType>
		const std::string& ObjectRegistry::ObjectRegistryItem<ObjectType>::Name() const {
			return _name;
		}

		template<typename ObjectType>
		void ObjectRegistry::RegisterObject(const std::string& name) {

			std::unique_ptr<IObjectRegistryItem> item(new ObjectRegistryItem<ObjectType>(name));

			_registry[name] = std::move(item);

		}

	}
}