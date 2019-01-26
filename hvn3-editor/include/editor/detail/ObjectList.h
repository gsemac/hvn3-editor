#pragma once

#include "hvn3/math/Point2d.h"
#include "hvn3/math/Rectangle.h"
#include "hvn3/objects/ObjectDefs.h"
#include "hvn3/utility/Utf8String.h"

#include <unordered_map>
#include <utility>
#include <utility>
#include <vector>

namespace hvn3 {
	namespace editor {
		namespace detail {

			class ObjectList {

			public:
				class Item {

				public:
					Item();
					Item(IObjectPtr&& object);

					const IObjectPtr& Object() const;
					const RectangleF& BoundingBox();

					explicit operator bool() const;

					static const Item NULL_ITEM;

				private:
					IObjectPtr _object;
					RectangleF _bounding_box;

				};

				typedef Item value_type;
				typedef std::pair<String, String> property_pair_type;
				typedef std::vector<property_pair_type> property_list_type;

				// Adds an object to the list.
				void Add(IObjectPtr object);
				// Removes an object from the list.
				void Remove(const IObjectPtr& object);
				// Sets the value of the given property to the given object.
				void SetProperty(const IObjectPtr& object, const String& name, const String& value);
				// Sets the value of the given property to the given object.
				void SetProperty(const IObject* object, const String& name, const String& value);

				const property_list_type& GetProperties(const IObjectPtr& object) const;
				const property_list_type& GetProperties(const IObject* object) const;

				// Returns the topmost object whose bounding box contains the given position.
				const value_type& Pick(const PointF& at);

			private:
				std::vector<value_type> _items;
				std::unordered_map<IObject*, property_list_type> _properties;

			};

			bool operator==(const ObjectList::Item& lhs, const ObjectList::Item& rhs);

		}
	}
}