#pragma once
#include <hvn3/objects/IObject.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace hvn3 {
	namespace editor {

		class ObjectRegistry {

			class IObjectInstantiator {
			public:
				virtual IObject* Create() const = 0;
				virtual const std::string& Name() const = 0;
				virtual ObjectId Id() const = 0;
			};

			template<typename ObjectType>
			class ObjectInstantiator :
				public IObjectInstantiator {

			public:
				ObjectInstantiator(const std::string& name) :
					_name(name) {

					_instantiated_once = false;

				}

				IObject* Create() const override {

					IObject* object = new ObjectType;

					_initMembersFromObject(object);

					return object;

				}
				const std::string& Name() const override {
					return _name;
				}
				ObjectId Id() const override {

					if (_instantiated_once)
						return _id;

					auto temp = std::make_unique<ObjectType>();

					_initMembersFromObject(temp.get());

					return _id;

				}

			private:
				std::string _name;
				mutable ObjectId _id;
				// Set to true when the object has been instantiated at least once.
				// This means any values that require the object to be instantiated will have been set.
				mutable bool _instantiated_once;

				void _initMembersFromObject(IObject* object) const {

					if (_instantiated_once)
						return;

					_id = object->Id();

					_instantiated_once = true;

				}

			};

		public:
			typedef std::unordered_map<std::string, std::shared_ptr<IObjectInstantiator>> registry_type;

			ObjectRegistry() = default;

			template<typename ObjectType>
			void RegisterObject(const std::string& name) {

				auto instantiator = std::shared_ptr<IObjectInstantiator>(new ObjectInstantiator<ObjectType>(name));

				_registry[name] = std::move(instantiator);

			}

			IObject* CreateByName(const std::string& key) const {
				return _registry.at(key)->Create();
			}
			IObject* CreateById(ObjectId key) const {

				for (auto i = _registry.begin(); i != _registry.end(); ++i)
					if (i->second->Id() == key)
						return i->second->Create();

				// Return a null object ptr if no object exists with the given id.
				return nullptr;

			}
			const std::string& GetNameById(ObjectId key) const {

				static std::string empty = "";

				for (auto i = _registry.begin(); i != _registry.end(); ++i)
					if (i->second->Id() == key)
						return i->first;

				return empty;

			}

			registry_type::iterator begin() {
				return _registry.begin();
			}
			registry_type::iterator end() {
				return _registry.end();
			}

		private:
			registry_type _registry;

		};

	}
}