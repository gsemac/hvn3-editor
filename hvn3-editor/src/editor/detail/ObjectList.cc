#include "hvn3/core/DrawEventArgs.h"
#include "hvn3/graphics/GraphicsPath.h"
#include "hvn3/objects/IObject.h"

#include "editor/detail/ObjectList.h"

#include <algorithm>
#include <cassert>

namespace hvn3 {
	namespace editor {
		namespace detail {

			ObjectList::Item::Item() {}
			ObjectList::Item::Item(IObjectPtr&& object) {

				this->_object = std::move(object);

			}
			const IObjectPtr& ObjectList::Item::Object() const {

				return _object;

			}
			const RectangleF& ObjectList::Item::BoundingBox() {

				if (_bounding_box.Width() <= 0.0f && _bounding_box.Height() <= 0.0f) {

					GraphicsPath path;
					Graphics::Graphics gfx(path);
					DrawEventArgs args(gfx);

					_object->OnDraw(args);

					_bounding_box = path.BoundingBox();

				}

				_bounding_box.SetPosition(_object->Position());

				return _bounding_box;

			}
			ObjectList::Item::operator bool() const {

				return static_cast<bool>(_object);

			}

			const ObjectList::Item ObjectList::Item::NULL_ITEM;



			void ObjectList::Add(IObjectPtr object) {

				// Initialize the properties vector for this object.
				_properties[object.get()] = std::vector<std::pair<String, String>>();

				// Move the object into the list.
				_items.push_back(std::move(Item(std::move(object))));

			}
			void ObjectList::Remove(const IObjectPtr& object) {

				// Remove the properties vector for this object.
				_properties.erase(object.get());

				// Remove the object from the list.
				_items.erase(std::remove_if(_items.begin(), _items.end(), [&](const Item& x)->bool {
					return x.Object() == object;
				}), _items.end());

			}
			void ObjectList::SetProperty(const IObjectPtr& object, const String& name, const String& value) {

				SetProperty(object.get(), name, value);

			}
			void ObjectList::SetProperty(const IObject* object, const String& name, const String& value) {

				// Find the object in the properties map.

				auto properties_iter = _properties.find(const_cast<IObject*>(object));

				assert(properties_iter != _properties.end());

				// If the given property already exists, update its value.

				auto property_iter = std::find_if(properties_iter->second.begin(), properties_iter->second.end(), [&](const property_pair_type& x) {
					return x.first == name;
				});

				if (property_iter != properties_iter->second.end())
					property_iter->second = value;
				else
					properties_iter->second.push_back(std::make_pair(name, value));

			}
			const ObjectList::property_list_type& ObjectList::GetProperties(const IObjectPtr& object) const {

				return GetProperties(object.get());

			}
			const ObjectList::property_list_type& ObjectList::GetProperties(const IObject* object) const {

				auto properties_iter = _properties.find(const_cast<IObject*>(object));

				assert(properties_iter != _properties.end());

				return properties_iter->second;

			}
			const ObjectList::value_type& ObjectList::Pick(const PointF& at) {

				// Sort the list by depth first.

				std::sort(_items.begin(), _items.end(), [](const Item& lhs, const Item& rhs) {
					return lhs.Object()->Depth() < rhs.Object()->Depth();
				});

				// Return the first object whose bounding box contains the given point.

				for (auto i = _items.begin(); i != _items.end(); ++i)
					if (i->BoundingBox().ContainsPoint(at))
						return *i;

				// If no such objects exist, return an empty item that the user can check for.
				return Item::NULL_ITEM;

			}



			bool operator==(const ObjectList::Item& lhs, const ObjectList::Item& rhs) {

				return lhs.Object() == rhs.Object();

			}

		}
	}
}