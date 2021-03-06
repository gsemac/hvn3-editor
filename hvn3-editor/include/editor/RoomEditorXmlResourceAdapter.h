#pragma once
#include "hvn3/xml/XmlResourceAdapterBase.h"

#include "editor/RoomEditor.h"

namespace hvn3 {
	namespace editor {

		template <typename BaseAdapterT>
		class RoomEditorXmlResourceAdapter :
			public BaseAdapterT {

		public:
			RoomEditorXmlResourceAdapter(RoomEditor* editor, bool loadResourcesIntoEditor) {

				_editor = editor;
				_load_resources_into_editor = loadResourcesIntoEditor;

			}

			IRoomPtr ImportRoom(const Xml::XmlElement& node) const override {

				IRoomPtr room;

				int width, height;
				width = StringUtils::Parse<int>(node["width"]);
				height = StringUtils::Parse<int>(node["height"]);

				if (_editor->_room_provider)
					room = _editor->_room_provider(SizeI(width, height));
				else
					room = hvn3::make_room<>(SizeI(width, height));

				ReadDefaultProperties(room, node);

				return room;

			}
			Background ImportBackground(const Xml::XmlElement& node) const override {

				if (node.HasAttribute("id")) {

					String id = IO::Path::Combine(_editor->_resource_base_directory, node.GetAttribute("id"));

					// If the background already exists in the background view widget, return it.
					const Background* ptr = _editor->_backgrounds_view->GetBackgroundById(id);

					if (ptr != nullptr)
						return *ptr;

					// Load the background into the background view widget.
					Background bg(Graphics::Bitmap::FromFile(id));

					Xml::XmlResourceAdapterBase<>::ReadDefaultProperties(bg, node);

					if (_load_resources_into_editor)
						_editor->_backgrounds_view->AddBackground(id, bg);

					return bg;

				}
				else
					return BaseAdapterT::ImportBackground(node);

			}
			void ImportTiles(TileManager& data, const Xml::XmlElement& node) const override {

				const Xml::XmlElement* tilesets_node = node.GetChild("tilesets");

				if (tilesets_node != nullptr) {

					for (auto i = tilesets_node->ChildrenBegin(); i != tilesets_node->ChildrenEnd(); ++i) {

						String id = IO::Path::Combine(_editor->_resource_base_directory, (*i)->GetAttribute("id"));
						int tile_w = StringUtils::Parse<int>((*i)->GetAttribute("tile_w"));
						int tile_h = StringUtils::Parse<int>((*i)->GetAttribute("tile_h"));

						Tileset tileset = Tileset::FromFile(id, SizeI(tile_w, tile_h));

						if (_load_resources_into_editor)
							_editor->_tileset_view->AddTileset(tileset, id);

						data.AddTileset(tileset);

					}

				}

				BaseAdapterT::ImportTiles(data, node);

			}
			IObjectPtr ImportObject(const Xml::XmlElement& node) const override {

				std::string name = node.GetAttribute("name");

				IObjectPtr ptr = _editor->_object_registry.MakeObject(name);

				if (_load_resources_into_editor) {

					_editor->_object_list.Add(ptr);
					_editor->_object_list.SetProperty(ptr, "name", name);

					for (auto i = node.AttributesBegin(); i != node.AttributesEnd(); ++i)
						if (!_isDefaultAttribute(i->first))
							_editor->_object_list.SetProperty(ptr, i->first, i->second);

				}

				Xml::XmlResourceAdapterBase<>::ReadDefaultProperties(ptr, node);

				return ptr;

			}

			void ExportBackground(const Background& data, Xml::XmlElement& node) const override {

				// Find the background in the backgrounds list that this background corresponds to.

				String id = _editor->_backgrounds_view->GetIdByBackground(data);

				node.SetAttribute("id", _editor->_makePathRelativeToResourceBaseDirectory(id));

				BaseAdapterT::ExportBackground(data, node);

			}
			void ExportTiles(const TileManager& data, Xml::XmlElement& node) const override {

				// Export all tilesets.

				Xml::XmlElement* tilesets_node = node.AddChild("tilesets");

				for (auto i = _editor->_tileset_view->Tilesets().begin(); i != _editor->_tileset_view->Tilesets().end(); ++i) {

					Xml::XmlElement* tileset_node = tilesets_node->AddChild("tileset");
					tileset_node->SetAttribute("id", _editor->_makePathRelativeToResourceBaseDirectory(_editor->_tileset_view->GetIdByTileset(*i)));
					tileset_node->SetAttribute("tile_w", i->TileSize().width);
					tileset_node->SetAttribute("tile_h", i->TileSize().height);

				}

				BaseAdapterT::ExportTiles(data, node);

			}
			void ExportObject(const IObjectPtr& data, Xml::XmlElement& node) const override {

				auto properties = _editor->_object_list.GetProperties(data);

				for (auto i = properties.begin(); i != properties.end(); ++i)
					node.SetAttribute(i->first, i->second);

				BaseAdapterT::ExportObject(data, node);

			}

		private:
			RoomEditor* _editor;
			bool _load_resources_into_editor;

			bool _isDefaultAttribute(const String& attribute) const {

				return attribute == "name" ||
					attribute == "x" ||
					attribute == "y" ||
					attribute == "id";

			}

		};

	}
}