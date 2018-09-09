#include "hvn3/gui2/Button.h"
#include "hvn3/gui2/MenuStrip.h"
#include "hvn3/gui2/TextBox.h"
#include "hvn3/gui2/TilesetView.h"
#include "hvn3/gui2/WidgetLayoutBuilder.h"
#include "hvn3/native/FileDialog.h"

#include "editor/RoomEditor.h"
#include "editor/RoomEditorTilesetsWidget.h"

namespace hvn3 {
	namespace editor {

		RoomEditorTilesetsWidget::RoomEditorTilesetsWidget(RoomEditor* editor) :
			Window("") {

			_editor = editor;

			tilesets_context_menu = new Gui::ContextMenu;
			layers_context_menu = new Gui::ContextMenu;
			Gui::MenuStrip* menu_strip = new Gui::MenuStrip;
			tileset_view = nullptr;

			tilesets_context_menu->AddItem("Add Tileset...")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) { _openTileset(); });
			layers_context_menu->AddItem("Layer 0")->SetChecked(true);
			layers_context_menu->AddSeparator();
			layers_context_menu->AddItem("Add Layer...");

			Gui::MenuStripItem* item;

			item = menu_strip->AddItem("");
			item->SetContextMenu(tilesets_context_menu);
			item->AddId(".tileset_btn");

			item = menu_strip->AddItem("");
			item->SetContextMenu(layers_context_menu);
			item->AddId(".layers_btn");

			GetChildren().Add(menu_strip);

		}
		Gui::TilesetView* RoomEditorTilesetsWidget::TilesetView() {
			return tileset_view;
		}
		const std::vector<Tileset>& RoomEditorTilesetsWidget::Tilesets() const {
			return _tilesets;
		}
		String RoomEditorTilesetsWidget::GetIdByTileset(const Tileset& tileset) const {

			int index = 2; // Skip "add" item and separator

			for (auto i = _tilesets.begin(); i != _tilesets.end(); ++i, ++index)
				if (i->Bitmap() == tileset.Bitmap())
					return tilesets_context_menu->ItemAt(index)->Text();

			return String::Empty;

		}
		void RoomEditorTilesetsWidget::AddTileset(const Tileset& tileset, const String& id) {

			bool first_item = (tilesets_context_menu->Count() == 1);

			if (first_item)
				tilesets_context_menu->AddSeparator();

			_tilesets.push_back(tileset);
			tilesets_context_menu->AddItem(id);

			Gui::ContextMenuItem* item = tilesets_context_menu->AddItem(id);

			item->SetEventHandler<Gui::WidgetEventType::OnCheckedStateChanged>([this](Gui::WidgetCheckedStateChangedEventArgs& e) {

			});

			item->SetChecked(first_item);

			if (tileset_view == nullptr) {

				tileset_view = new Gui::TilesetView(_tilesets.back());
				tileset_view->SetDockStyle(Gui::DockStyle::Fill);

				GetChildren().Add(tileset_view);

			}

		}

		void RoomEditorTilesetsWidget::_openTileset() {

			Gui::Window* dialog = new  Gui::Window(250, 200, "Add New Tileset");

			Gui::Label* label_tileset_dir = new Gui::Label("Tileset Path");
			Gui::TextBox* textbox_tileset_dir = new Gui::TextBox(200);
			Gui::Button* button_resource_base_dir = new Gui::Button("...");
			Gui::Label* label_tile_width = new Gui::Label("Tile Width");
			Gui::TextBox* textbox_tile_width = new Gui::TextBox(100, Gui::InputType::Numeric);
			Gui::Label* label_tile_height = new Gui::Label("Tile Height");
			Gui::TextBox* textbox_tile_height = new Gui::TextBox(100, Gui::InputType::Numeric);

			Gui::Button* button_ok = new Gui::Button("OK");

			textbox_tileset_dir->SetAnchor(Gui::Anchor::Left | Gui::Anchor::Right);
			button_ok->SetWidth(100);
			textbox_tile_width->SetText("32");
			textbox_tile_height->SetText("32");
			button_resource_base_dir->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

				FileDialog f(FileDialogFlags::FileMustExist);
				f.SetFilter("Images|*.png");

				if (f.ShowDialog())
					textbox_tileset_dir->SetText(f.FileName());

			});
			button_ok->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

				if (textbox_tileset_dir->Text().Length() > 0) {

					AddTileset(Tileset(Graphics::Bitmap::FromFile(textbox_tileset_dir->Text()), SizeI(StringUtils::Parse<int>(textbox_tile_width->Text()),
						StringUtils::Parse<int>(textbox_tile_height->Text()))), textbox_tileset_dir->Text());

					_editor->_room->GetTiles().AddTileset(_tilesets.back());

				}

				dialog->Close();

			});

			dialog->GetChildren().Add(label_tileset_dir);
			dialog->GetChildren().Add(textbox_tileset_dir);
			dialog->GetChildren().Add(button_resource_base_dir);
			dialog->GetChildren().Add(label_tile_width);
			dialog->GetChildren().Add(textbox_tile_width);
			dialog->GetChildren().Add(label_tile_height);
			dialog->GetChildren().Add(textbox_tile_height);
			dialog->GetChildren().Add(button_ok);

			_editor->_widgets.ShowDialog(std::unique_ptr<Gui::IWidget>(dialog));

			Gui::WidgetLayoutBuilder builder;

			builder.PlaceAt(label_tileset_dir, PointF(0.0f, 0.0f));
			builder.PlaceBottom(textbox_tileset_dir);
			builder.PlaceRight(button_resource_base_dir);
			builder.PlaceBottomOf(label_tile_width, textbox_tileset_dir);
			builder.PlaceBottom(textbox_tile_width);
			builder.PlaceRight(textbox_tile_height);
			builder.PlaceTop(label_tile_height);
			builder.AnchorToInnerEdge(button_ok, Gui::Anchor::Bottom | Gui::Anchor::Right);

		}

	}
}