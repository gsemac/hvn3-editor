#include "hvn3/core/IGameManager.h"
#include "hvn3/core/GameProperties.h"
#include "hvn3/io/Mouse.h"
#include "hvn3/io/Path.h"
#include "hvn3/rooms/RoomManager.h"
#include "hvn3/gui2/Button.h"
#include "hvn3/gui2/DataGrid.h"
#include "hvn3/gui2/MenuStrip.h"
#include "hvn3/gui2/ListBox.h"
#include "hvn3/gui2/RoomView.h"
#include "hvn3/gui2/TextBox.h"
#include "hvn3/gui2/TilesetView.h"
#include "hvn3/gui2/WidgetLayoutBuilder.h"
#include "hvn3/native/FileDialog.h"
#include "hvn3/xml/XmlDocument.h"

#include "editor/RoomEditor.h"
#include "editor/RoomEditorBackgroundsWidget.h"
#include "editor/RoomEditorStatusStripWidget.h"
#include "editor/RoomEditorTilesetsWidget.h"
#include "editor/RoomEditorViewsWidget.h"
#include "editor/RoomEditorXmlResourceAdapter.h"

namespace hvn3 {
	namespace editor {

		void BLOCK_LISTENERS() {

			Keyboard::Listeners::SetBlocked(true);
			Mouse::Listeners::SetBlocked(true);
			ListenerCollection<IDisplayListener>::SetBlocked(true);

		}
		void UNBLOCK_LISTENERS() {

			Keyboard::Listeners::SetBlocked(false);
			Mouse::Listeners::SetBlocked(false);
			ListenerCollection<IDisplayListener>::SetBlocked(false);

		}


		RoomEditor::BackToEditorObject::BackToEditorObject(IRoomPtr editor) :
			Object(NoOne),
			_editor(editor) {
		}
		void RoomEditor::BackToEditorObject::OnKeyPressed(KeyPressedEventArgs& e) {

			if (_context && e.Key() == Key::Escape)
				_context.Get<ROOM_MANAGER>().SetRoom(_editor);

		}
		void RoomEditor::BackToEditorObject::OnContextChanged(ContextChangedEventArgs& e) {
			_context = e.Context();
		}


		RoomEditor::RoomEditor() :
			hvn3::Room(0, 0) {

			_editor_name = "hvn3 Room Editor";
			_default_file_ext = ".hvn3room";

			_editor_mode = EDITOR_MODE_TILES;
			_placing_object = nullptr;
			_highlight_object = nullptr;

			_editor_initialized = false;
			_properties_exit_with_esc = false;
			_has_unsaved_changes = false;
			_zoom_level = 0;

		}
		void RoomEditor::OnCreate(RoomCreateEventArgs& e) {
			Room::OnCreate(e);

			SetBackgroundColor(Color(49, 47, 59));

			// The escape key is used to exit playtesting, so we save the initial value so we can set/restore it as needed.

			if (_editor_initialized)
				_context.Get<GAME_MANAGER>().Properties().ExitWithEscapeKey = _properties_exit_with_esc;
			else
				_properties_exit_with_esc = _context.Get<GAME_MANAGER>().Properties().ExitWithEscapeKey;

			if (!_editor_initialized) {

				_initializeUi();
				_loadPreferences();

			}
			else {
				_resubscribeEditorListeners();
			}

			// Always update the window title in case we're returning from playtesting and the window title needs to be reset.
			_updateWindowTitle();

			_editor_initialized = true;

		}
		void RoomEditor::OnExit(RoomExitEventArgs& e) {
			Room::OnExit(e);

			_unsubscribeEditorListeners();

			_savePreferences();

		}
		void RoomEditor::OnContextChanged(ContextChangedEventArgs& e) {
			Room::OnContextChanged(e);

			_context = e.Context();

			// Provide context to the room (normally done by a RoomManager, so we need to handle it manually).

			if (_room) {

				ContextChangedEventArgs args(_context);
				_room->OnContextChanged(args);

			}

			// Match the room size to the size of the display.

			hvn3::SizeI display_size = _context.Get<GAME_MANAGER>().Display().Size();
			SetSize(display_size.width, display_size.height);

			_widgets.SetDockableRegion(hvn3::RectangleF(static_cast<hvn3::SizeF>(Size())));

		}
		void RoomEditor::OnRender(DrawEventArgs& e) {

			Room::OnRender(e);

			_widgets.OnDraw(e);

			if (_highlight_object != nullptr) {

				Graphics::GraphicsState state = e.Graphics().Save();
				Graphics::Transform transform = e.Graphics().GetTransform();

				PointF obj_pos = _highlight_object->Position();
				PointF offset = _room_view->RoomPositionToGlobalPosition(_highlight_object->Position()) - _highlight_object->Position();

				transform.Translate(offset.x, offset.y);

				e.Graphics().SetTransform(transform);

				//_highlight_object->OnDraw(e);

				float marker_radius = 5.0f;
				float marker_height = 8.0f;

				e.Graphics().DrawSolidCircle(obj_pos.x, obj_pos.y - marker_height, marker_radius, Color::Yellow);
				e.Graphics().DrawCircle(obj_pos.x, obj_pos.y - marker_height, marker_radius, Color::Black, 2.0f);

				e.Graphics().Restore(state);

			}

			//_drawTileCursor(e);

		}
		void RoomEditor::OnUpdate(UpdateEventArgs& e) {
			Room::OnUpdate(e);

			_widgets.OnUpdate(e);

		}
		void RoomEditor::OnDisplaySizeChanged(DisplaySizeChangedEventArgs& e) {

			// Update the room size and dockable region to match the size of the display.

			SetSize(e.Display().Size());
			_widgets.SetDockableRegion(hvn3::RectangleF(static_cast<hvn3::SizeF>(Size())));

		}
		void RoomEditor::OnMouseDown(MouseDownEventArgs& e) {



		}
		void RoomEditor::OnMousePressed(MousePressedEventArgs& e) {



		}
		void RoomEditor::OnMouseMove(MouseMoveEventArgs& e) {

			if (!_room)
				return;

			_mouse_position = e.Position();
			PointF room_position = _room_view->GlobalPositionToRoomPosition(e.Position(), false);
			PointF grid_position = _room_view->GlobalPositionToRoomPosition(e.Position(), true);

			switch (_editor_mode) {

			case EDITOR_MODE_OBJECTS:

				if (_placing_object != nullptr)
					_placing_object->SetPosition(grid_position);

				break;

			}

			_status_strip->SetText(StringUtils::Format("x: {0}, y: {1}", grid_position.x, grid_position.y));

		}
		void RoomEditor::OnMouseReleased(MouseReleasedEventArgs& e) {

			if (_editor_mode == EDITOR_MODE_OBJECTS) {

				_placing_object = nullptr;

				if (e.Button() == MouseButton::Right) {

					// Select the object that was clicked.

					// Get the position that was clicked in room coordinates.
					PointF clicked_pos = e.Position();
					PointF pos = _room_view->GlobalPositionToRoomPosition(clicked_pos, false);

					// Find the closest objects to the point that was clicked.

					std::vector<IObject*> objects;

					_room->Objects().ForEach([&](const IObjectPtr& obj) {
						objects.push_back(obj.get());
					});

					std::sort(objects.begin(), objects.end(), [&](const IObject* lhs, const IObject* rhs) {
						return Math::Geometry::PointDistance(pos, lhs->Position()) < Math::Geometry::PointDistance(pos, rhs->Position());
					});

					Gui::ContextMenu* context_menu = new Gui::ContextMenu;
					Gui::ContextMenu* selection_menu = new Gui::ContextMenu;

					for (auto i = objects.begin(); i != objects.end(); ++i) {

						IObject* ptr = *i;

						auto item = selection_menu->AddItem(StringUtils::ToString(ptr));

						item->SetEventHandler<Gui::WidgetEventType::OnMouseEnter>([this, ptr](Gui::WidgetMouseEnterEventArgs& e) {
							_highlight_object = ptr;
						});
						item->SetEventHandler<Gui::WidgetEventType::OnMouseLeave>([this, ptr](Gui::WidgetMouseLeaveEventArgs& e) {
							_highlight_object = nullptr;
						});

						item->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this, ptr, clicked_pos](Gui::WidgetMouseClickEventArgs& e) {

							// Open up the property editor for the selected object.

							Gui::Window* property_window = new Gui::Window(250, 300, "Object Properties");
							Gui::MenuStrip* tool_strip = new Gui::MenuStrip;
							Gui::Button* ok_button = new Gui::Button("OK");
							Gui::DataGrid* property_grid = new Gui::DataGrid;

							tool_strip->SetDockStyle(Gui::DockStyle::Top);
							tool_strip->AddItem("Add Property")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

								property_grid->AddRow();

							});

							ok_button->SetDockStyle(Gui::DockStyle::Bottom);
							ok_button->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

								const size_t non_default_rows_index = 3;

								for (size_t i = non_default_rows_index; i < property_grid->RowCount(); ++i) {

									Gui::DataGridRow row = property_grid->Rows(i);

									// Skip properties with blank keys.
									if (row[0].Length() <= 0)
										continue;

									_object_list.SetProperty(ptr, row[0], row.Count() > 1 ? row[1] : "");

								}

								property_window->Close();

							});

							property_grid->SetDockStyle(Gui::DockStyle::Fill);
							property_grid->AddColumn("Property");
							property_grid->AddColumn("Value");
							property_grid->AddRow({ "id", StringUtils::ToString(ptr->Id()) });
							property_grid->AddRow({ "x", StringUtils::ToString(ptr->X()) });
							property_grid->AddRow({ "y", StringUtils::ToString(ptr->Y()) });

							// Load any properties that have been added for this object.

							auto existing_properties = _object_list.GetProperties(ptr);

							for (auto i = existing_properties.begin(); i != existing_properties.end(); ++i)
								property_grid->AddRow({ i->first, i->second });

							property_window->SetPosition(clicked_pos);
							property_window->GetChildren().Add(tool_strip);
							property_window->GetChildren().Add(ok_button);
							property_window->GetChildren().Add(property_grid);

							_widgets.Add(property_window);

							Gui::WidgetLayoutBuilder builder;
							builder.AnchorToInnerEdge(ok_button, Gui::Anchor::Left | Gui::Anchor::Right | Gui::Anchor::Bottom);

						});

						if (selection_menu->Count() >= 10) // Limit the number of items
							break;

					}

					auto item = context_menu->AddItem("Select...");
					item->SetContextMenu(selection_menu);

					_room_view->SetContextMenu(context_menu);

				}

			}

		}
		void RoomEditor::OnMouseScroll(MouseScrollEventArgs& e) {

			const int max_zoom_level = 5;

			if (HasFlag(_key_modifiers, KeyModifiers::Control))
				if (e.Direction() == MouseScrollDirection::Up)
					_zoom_level = Math::Min(_zoom_level + 1, max_zoom_level);
				else if (e.Direction() == MouseScrollDirection::Down)
					_zoom_level = Math::Max(_zoom_level - 1, -max_zoom_level);

			float zoom_scale = 1.0f;
			float scale_per_level = 0.5f;

			if (_zoom_level > 0)
				zoom_scale *= (std::pow)(1.0f + scale_per_level, static_cast<float>(_zoom_level));
			else if (_zoom_level < 0)
				zoom_scale *= (std::pow)(1.0f - scale_per_level, static_cast<float>(-_zoom_level));

			_room_view->SetRoomScale(Scale(zoom_scale));

		}
		void RoomEditor::OnKeyPressed(KeyPressedEventArgs& e) {

			_key_modifiers = e.Modifiers();

			if (e.Key() == Key::F5)
				_startPlaytest();
			else if (HasFlag(e.Modifiers(), KeyModifiers::Control)) {

				switch (e.Key()) {
				case Key::N:
					_showRoomNewDialog();
					break;
				case Key::O:
					_showRoomOpenDialog();
					break;
				case Key::S:
					if (_room)
						_showRoomSaveDialog();
					break;
				}

			}

		}
		void RoomEditor::OnKeyUp(KeyUpEventArgs& e) {

			_key_modifiers = e.Modifiers();

		}
		IRoomPtr RoomEditor::Room() {
			return _room;
		}
		void RoomEditor::SetObjectRegistry(const ObjectRegistry& registry) {
			_object_registry = registry;
		}
		void RoomEditor::SetRoomProvider(std::function<IRoomPtr(const SizeI&)>&& provider) {
			_room_provider = std::move(provider);
		}

		void RoomEditor::_drawTileCursor(DrawEventArgs& e) {

			//hvn3::RectangleI tile_selection = _tileset_view->SelectedRegion();
			//hvn3::SizeF size = static_cast<hvn3::SizeF>(hvn3::SizeI(_tileset_view->Tileset().GridSize()));

			//float x = _mouse_position.x;
			//float y = _mouse_position.y;

			//x = Math::Floor(x / size.width) * size.width;

			//x -= size.width / 2.0f;
			//y -= size.height / 2.0f;

			//e.Graphics().DrawBitmap(x, y, _tileset_view->Tileset().At(tile_selection.X(), tile_selection.Y()).bitmap, hvn3::Color::Transluscent);

			//for (int i = 0; i < tile_selection.Width(); ++i)
			//	for (int j = 0; j < tile_selection.Height(); ++j) {

			//		hvn3::SizeF size = static_cast<hvn3::SizeF>(hvn3::SizeI(_tileset_view->Tileset().GridSize()));
			//		float xoff = (i * size.width) - (size.width * tile_selection.Width() / 2.0f) - hvn3::Math::Mod(_room_draw_offset.x, size.width);
			//		float yoff = (j * size.height) - (size.height * tile_selection.Height() / 2.0f) - hvn3::Math::Mod(_room_draw_offset.y, size.height);
			//		float x = hvn3::Math::Round(hvn3::Mouse::x + xoff, size.width) + hvn3::Math::Mod(_room_draw_offset.x, size.width);
			//		float y = hvn3::Math::Round(hvn3::Mouse::y + yoff, size.height) + hvn3::Math::Mod(_room_draw_offset.y, size.height);

			//		e.Graphics().DrawBitmap(x, y, _tileset_view->Tileset().At(tile_selection.X() + i, tile_selection.Y() + j).bitmap, hvn3::Color::Transluscent);

			//	}

		}
		void RoomEditor::_initializeUi() {

			_initializeUiStyles();

			_updateWindowTitle();
			_initializeMenuStrip();
			_initializeToolStrip();

			_tileset_view = new RoomEditorTilesetsWidget(this);
			_tileset_view->SetDockStyle(hvn3::Gui::DockStyle::Fill);

			_objects_view = new hvn3::Gui::ListBox();
			_objects_view->SetVisible(false);
			_objects_view->SetDockStyle(hvn3::Gui::DockStyle::Fill);

			for (auto i = _object_registry.begin(); i != _object_registry.end(); ++i)
				_objects_view->AddItem(i->second->Name());

			_backgrounds_view = new RoomEditorBackgroundsWidget(this);

			_views_view = new RoomEditorViewsWidget;

			hvn3::Gui::Window* window = new hvn3::Gui::Window(0.0f, 0.0f, 300.0f, 0.0f, "");
			window->SetDockStyle(hvn3::Gui::DockStyle::Left);
			window->SetTitleBarVisible(false);
			window->GetChildren().Add(_tileset_view);
			window->GetChildren().Add(_objects_view);
			window->GetChildren().Add(_backgrounds_view);
			window->GetChildren().Add(_views_view);

			_status_strip = new RoomEditorStatusStripWidget;
			_status_strip->SetDockStyle(Gui::DockStyle::Bottom);
			_status_strip->SetText("Create a new room with \"File > New Room\".");

			_room_view = new Gui::RoomView;
			_room_view->SetWidth(200.0f);
			_room_view->SetDockStyle(Gui::DockStyle::Fill);
			_room_view->SetEventHandler<Gui::WidgetEventType::OnMouseDown>([this](Gui::WidgetMouseDownEventArgs& e) { _roomView_OnMouseDown(e); });
			_room_view->SetEventHandler<Gui::WidgetEventType::OnMousePressed>([this](Gui::WidgetMousePressedEventArgs& e) { _roomView_OnMousePressed(e); });

			_widgets.Add(window);
			_widgets.Add(_status_strip);
			_widgets.Add(_room_view);

			_left_panel = window;

		}
		void RoomEditor::_initializeUiStyles() {

			// Create a style instance that will be used as a template for a few different buttons.

			Gui::WidgetStyle style;

			Gui::WidgetStyle::BackgroundPositionProperty background_position;
			background_position.flags = Gui::WidgetStyle::PositionFlags::Center;
			style.SetProperty<Gui::WidgetProperty::BackgroundPosition>(background_position);

			// Create and add styles.

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/tile.png"));
			_widgets.Renderer()->AddStyle(".tile_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/object.png"));
			_widgets.Renderer()->AddStyle(".object_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/background.png"));
			_widgets.Renderer()->AddStyle(".background_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/arrow_d.png"));
			_widgets.Renderer()->AddStyle(".arrow_d", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/arrow_u.png"));
			_widgets.Renderer()->AddStyle(".arrow_u", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/camera.png"));
			_widgets.Renderer()->AddStyle(".view_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/tileset.png"));
			_widgets.Renderer()->AddStyle(".tileset_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/layers.png"));
			_widgets.Renderer()->AddStyle(".layers_btn", style);

			style.SetProperty<Gui::WidgetProperty::BackgroundImage>(Graphics::Bitmap::FromFile("bin/system/icons/flags.png"));
			_widgets.Renderer()->AddStyle(".flags_btn", style);

		}
		void RoomEditor::_initializeMenuStrip() {

			hvn3::Gui::ContextMenu* file_cm = new hvn3::Gui::ContextMenu;
			file_cm->AddItem("New\t\t\t\tCtrl+N")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) { _showRoomNewDialog(); });
			file_cm->AddItem("Open...\t\tCtrl+O")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) { _showRoomOpenDialog(); });
			file_cm->AddItem("Save\t\t\t\tCtrl+S")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {	_showRoomSaveDialog(); });
			file_cm->AddItem("Save As...")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {	_showRoomSaveAsDialog(); });
			file_cm->AddSeparator();
			file_cm->AddItem("Preferences...")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) { _showPreferencesDialog(); });
			file_cm->AddSeparator();
			file_cm->AddItem("Exit")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {	_context.Get<GAME_MANAGER>().Exit(); });

			hvn3::Gui::ContextMenu* view_cm = new hvn3::Gui::ContextMenu;

			auto view_cm_grid_item = view_cm->AddItem("Grid", true);

			view_cm_grid_item->SetEventHandler<Gui::WidgetEventType::OnMousePressed>([=](Gui::WidgetMousePressedEventArgs& e) {
				view_cm_grid_item->SetChecked(!view_cm_grid_item->Checked());
				_room_view->SetGridVisible(view_cm_grid_item->Checked());
			});

			auto view_cm_foregrounds_item = view_cm->AddItem("Foregrounds", true);

			view_cm_foregrounds_item->SetEventHandler<Gui::WidgetEventType::OnMousePressed>([=](Gui::WidgetMousePressedEventArgs& e) {

				if (!_room)
					return;

				view_cm_foregrounds_item->SetChecked(!view_cm_foregrounds_item->Checked());

				Room()->Backgrounds().ForEach([=](Background& i) {

					if (i.IsForeground())
						i.SetVisible(view_cm_foregrounds_item->Checked());

					HVN3_CONTINUE;

				});

			});

			hvn3::Gui::ContextMenu* test_cm = new hvn3::Gui::ContextMenu;
			test_cm->AddItem("Playtest\t\t\tF5")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) { _startPlaytest(); });

			hvn3::Gui::MenuStrip* ms = new hvn3::Gui::MenuStrip;
			ms->AddItem("File")->SetContextMenu(file_cm);
			ms->AddItem("View")->SetContextMenu(view_cm);
			ms->AddItem("Test")->SetContextMenu(test_cm);

			_widgets.Add(ms);

		}
		void RoomEditor::_initializeToolStrip() {

			Gui::MenuStrip* ms = new Gui::MenuStrip;
			Gui::MenuStripItem* item = nullptr;

			// Add button (tiles).
			item = ms->AddItem("");
			item->AddId(".tile_btn");
			item->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {
				_hideAllPanelWindows();
				_tileset_view->SetVisible(true);
				_editor_mode = EDITOR_MODE_TILES;
			});

			// Add button (objects).
			item = ms->AddItem("");
			item->AddId(".object_btn");
			item->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {
				_hideAllPanelWindows();
				_objects_view->SetVisible(true);
				_editor_mode = EDITOR_MODE_OBJECTS;
			});

			// Add button (backgrounds).
			item = ms->AddItem("");
			item->AddId(".background_btn");
			item->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {
				_hideAllPanelWindows();
				_backgrounds_view->SetVisible(true);
				_editor_mode = EDITOR_MODE_BACKGROUNDS;
			});

			// Add button (views).
			item = ms->AddItem("");
			item->AddId(".view_btn");
			item->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([this](Gui::WidgetMouseClickEventArgs& e) {
				_hideAllPanelWindows();
				_views_view->SetVisible(true);
				_editor_mode = EDITOR_MODE_VIEWS;
			});

			_widgets.Add(ms);

		}
		void RoomEditor::_updateWindowTitle() {

			if (!_context)
				return;

			std::string title;

			if (_current_file.size() > 0)
				title = IO::Path::GetFileName(_current_file);
			else
				title = "untitled";

			if (_has_unsaved_changes)
				title.push_back('*');

			title += " - " + _editor_name;

			_context.Get<GAME_MANAGER>().Display().SetTitle(title);

		}
		void RoomEditor::_hideAllPanelWindows() {

			_objects_view->SetVisible(false);
			_tileset_view->SetVisible(false);
			_backgrounds_view->SetVisible(false);
			_views_view->SetVisible(false);

		}
		void RoomEditor::_showPreferencesDialog() {

			Gui::Window* dialog = new  Gui::Window(250, 200, "Preferences");

			Gui::Label* label_resource_base_dir = new Gui::Label("Resource base directory");
			Gui::TextBox* textbox_resource_base_dir = new Gui::TextBox(200);
			Gui::Button* button_resource_base_dir = new Gui::Button("...");
			Gui::Label* label_grid_w = new Gui::Label("Grid Width");
			Gui::Label* label_grid_h = new Gui::Label("Grid Height");
			Gui::TextBox* textbox_grid_w = new Gui::TextBox(100, Gui::InputType::Numeric);
			Gui::TextBox* textbox_grid_h = new Gui::TextBox(100, Gui::InputType::Numeric);
			Gui::Button* button_ok = new Gui::Button("OK");
			button_ok->SetWidth(100);

			textbox_resource_base_dir->SetAnchor(Gui::Anchor::Left | Gui::Anchor::Right);
			textbox_resource_base_dir->SetText(_resource_base_directory);
			textbox_grid_w->SetText(StringUtils::ToString(_room_view->GridCellSize().width));
			textbox_grid_h->SetText(StringUtils::ToString(_room_view->GridCellSize().height));
			button_ok->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

				SizeF grid_cell_size = _room_view->GridCellSize();

				StringUtils::TryParse(textbox_grid_w->Text(), grid_cell_size.width);
				StringUtils::TryParse(textbox_grid_h->Text(), grid_cell_size.height);

				_room_view->SetGridCellSize(grid_cell_size);

				dialog->Close();

			});
			button_resource_base_dir->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

				FileDialog fd(FileDialogFlags::Folder);
				fd.SetInitialDirectory(_resource_base_directory);

				if (fd.ShowDialog()) {
					_resource_base_directory = fd.FileName();
					textbox_resource_base_dir->SetText(_resource_base_directory);
				}

			});

			dialog->GetChildren().Add(label_resource_base_dir);
			dialog->GetChildren().Add(textbox_resource_base_dir);
			dialog->GetChildren().Add(button_resource_base_dir);
			dialog->GetChildren().Add(label_grid_w);
			dialog->GetChildren().Add(label_grid_h);
			dialog->GetChildren().Add(textbox_grid_w);
			dialog->GetChildren().Add(textbox_grid_h);
			dialog->GetChildren().Add(button_ok);

			_widgets.ShowDialog(std::unique_ptr<Gui::IWidget>(dialog));

			Gui::WidgetLayoutBuilder builder;

			builder.PlaceAt(label_resource_base_dir, PointF(0.0f, 0.0f));
			builder.PlaceBottom(textbox_resource_base_dir);
			builder.PlaceRight(button_resource_base_dir);
			builder.PlaceBottomOf(label_grid_w, textbox_resource_base_dir);
			builder.PlaceBottom(textbox_grid_w);
			builder.PlaceRight(textbox_grid_h);
			builder.PlaceTop(label_grid_h);
			builder.AnchorToInnerEdge(button_ok, Gui::Anchor::Bottom | Gui::Anchor::Right);

		}
		void RoomEditor::_showRoomNewDialog() {

			Gui::Window* dialog = new  Gui::Window(250, 200, "New Room");
			Gui::Button* button_create = new Gui::Button(0, 0, 100, 25, "Create");
			Gui::Button* button_cancel = new Gui::Button(0, 0, 100, 25, "Cancel");

			Gui::Label* label_w = new Gui::Label("Width (px)");
			Gui::TextBox* textbox_w = new Gui::TextBox(100, Gui::InputType::Numeric);
			textbox_w->SetText("640");

			Gui::Label* label_h = new Gui::Label("Height (px)");
			Gui::TextBox* textbox_h = new Gui::TextBox(100, Gui::InputType::Numeric);
			textbox_h->SetText("480");

			button_create->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {

				_createNewRoom(StringUtils::Parse<int>(textbox_w->Text()), StringUtils::Parse<int>(textbox_h->Text()));
				dialog->Close();

			});

			button_cancel->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([=](Gui::WidgetMouseClickEventArgs& e) {
				dialog->Close();
			});

			dialog->GetChildren().Add(label_w);
			dialog->GetChildren().Add(label_h);
			dialog->GetChildren().Add(textbox_w);
			dialog->GetChildren().Add(textbox_h);
			dialog->GetChildren().Add(button_create);
			dialog->GetChildren().Add(button_cancel);

			_widgets.ShowDialog(std::unique_ptr<Gui::IWidget>(dialog));

			Gui::WidgetLayoutBuilder builder;

			builder.PlaceAt(label_w, PointF(0.0f, 0.0f));
			builder.PlaceBottom(textbox_w);
			builder.PlaceRight(textbox_h);
			builder.CenterHorizontally({ textbox_w, textbox_h });
			builder.PlaceTopOf(label_w, textbox_w);
			builder.PlaceTopOf(label_h, textbox_h);
			builder.AnchorToInnerEdge(button_cancel, Gui::Anchor::Bottom | Gui::Anchor::Right);
			builder.PlaceLeftOf(button_create, button_cancel);

		}
		void RoomEditor::_showRoomOpenDialog() {

			FileDialog f(FileDialogFlags::FileMustExist);
			f.SetFilter("HVN3 Room|*" + _default_file_ext);

			if (_last_directory.size() > 0)
				f.SetInitialDirectory(_last_directory);

			if (f.ShowDialog())
				_loadRoomFromFileIntoEditor(f.FileName());

		}
		void RoomEditor::_showRoomSaveDialog() {

			if (!_room)
				return;

			if (IO::File::Exists(_current_file))
				_saveRoomToFile(_current_file, false);
			else
				_showRoomSaveAsDialog();

		}
		void RoomEditor::_showRoomSaveAsDialog() {

			if (!_room)
				return;

			std::string fname = IO::Path::GetFileName(_current_file);

			if (fname.size() <= 0)
				fname = "untitled";

			if (!StringUtils::EndsWith(fname, _default_file_ext))
				fname += _default_file_ext;

			FileDialog f(FileDialogFlags::Save);
			f.SetDefaultExtension(_default_file_ext);
			f.SetFilter("hvn3 Room|*" + _default_file_ext);
			f.SetFileName(IO::Path::GetFileName(fname));

			if (_last_directory.size() > 0)
				f.SetInitialDirectory(_last_directory);

			if (f.ShowDialog())
				_saveRoomToFile(f.FileName(), false);

		}
		void RoomEditor::_showRoomViewContextMenu() {



		}
		void RoomEditor::_loadPreferences() {

			SizeF room_view_grid_cell_size(32.0f, 32.0f);

			if (IO::File::Exists("editor_preferences.xml")) {

				Xml::XmlDocument pref = Xml::XmlDocument::Open("editor_preferences.xml");
				Xml::XmlElement* node = nullptr;

				if (node = pref.Root().GetChild("last_directory"), node != nullptr)
					_last_directory = node->Text();

				if (node = pref.Root().GetChild("last_file"), node != nullptr)
					_current_file = node->Text();

				if (node = pref.Root().GetChild("resource_base_directory"), node != nullptr)
					_resource_base_directory = node->Text();

				if (node = pref.Root().GetChild("grid"), node != nullptr)
					room_view_grid_cell_size = SizeF(StringUtils::Parse<float>(node->GetAttribute("w")), StringUtils::Parse<float>(node->GetAttribute("h")));

			}

			// Apply loaded preferences.

			if (_room_view != nullptr)
				_room_view->SetGridCellSize(room_view_grid_cell_size);

			if (_current_file.size() > 0 && IO::File::Exists(_current_file))
				_loadRoomFromFileIntoEditor(_current_file);

		}
		void RoomEditor::_savePreferences() {

			Xml::XmlDocument pref("preferences");
			pref.Root().AddChild("last_directory")->SetText(_last_directory);
			pref.Root().AddChild("last_file")->SetText(_current_file);
			pref.Root().AddChild("resource_base_directory")->SetText(_resource_base_directory);

			Xml::XmlElement* node = pref.Root().AddChild("grid");
			node->SetAttribute("w", _room_view->GridCellSize().width);
			node->SetAttribute("h", _room_view->GridCellSize().height);

			pref.Save("editor_preferences.xml");

		}
		void RoomEditor::_createNewRoom(int width, int height) {

			// Create a new room instance and set it as the current instance.
			if (_room_provider)
				_room = _room_provider(SizeI(width, height));
			else
				_room = hvn3::make_room<>(width, height);

			// Set the room's starting background color.
			_room->SetBackgroundColor(Color::Silver);

			// Set the tile size to match the grid size.
			_room->Tiles().SetTileSize(static_cast<SizeI>(_room_view->GridCellSize()));

			// Provide context to the room (normally done by a RoomManager, so we need to handle it manually).
			ContextChangedEventArgs args(_context);
			_room->OnContextChanged(args);

			// Update the room tied to the RoomView widget.
			_room_view->SetRoom(_room);

			_current_file = "";
			_has_unsaved_changes = true;

			_updateWindowTitle();

		}
		IRoomPtr RoomEditor::_loadRoomFromFileIntoMemory(const std::string& file_path, bool load_resources_into_editor) {

			RoomEditorXmlResourceAdapter<Xml::XmlResourceAdapterBase<>> adapter(this, load_resources_into_editor);
			Xml::XmlDocument document = Xml::XmlDocument::Open(file_path);

			IRoomPtr room = adapter.ImportRoom(document.Root());

			// Provide context to the room (normally done by a RoomManager, so we need to handle it manually).

			if (room) {

				ContextChangedEventArgs args(_context);
				room->OnContextChanged(args);

			}

			return room;

		}
		void RoomEditor::_loadRoomFromFileIntoEditor(const std::string& file_path) {

			BLOCK_LISTENERS();

			_room = _loadRoomFromFileIntoMemory(file_path, true);

			_room_view->SetRoom(_room);
			_room_view->SetGridCellSize(static_cast<SizeF>(_room->Tiles().TileSize()));

			_last_directory = IO::Path::GetDirectoryName(file_path);
			_current_file = file_path;

			_has_unsaved_changes = false;
			_updateWindowTitle();

			UNBLOCK_LISTENERS();

			_status_strip->SetText("Successfully loaded room from " + IO::Path::GetFileName(file_path));

		}
		void RoomEditor::_saveRoomToFile(const std::string& file_path, bool is_temporary_file) {

			assert(static_cast<bool>(_room));

			RoomEditorXmlResourceAdapter<Xml::XmlResourceAdapterBase<>> adapter(this, false);
			Xml::XmlDocument document;

			adapter.ExportRoom(_room, document.Root());
			document.Save(file_path);

			// Write metadata files for tilesets.

			// Update the tileset list with the current tileset in case it was modified (flags, etc.).

			if (_tileset_view->Tilesets().size() > 0)
				*_tileset_view->GetTilesetById(_tileset_view->GetIdByTileset(_tileset_view->TilesetView()->Tileset())) = _tileset_view->TilesetView()->Tileset();

			for (auto i = _tileset_view->Tilesets().begin(); i != _tileset_view->Tilesets().end(); ++i) {

				Xml::XmlDocument meta("tileset");

				meta.Root().SetAttribute("version", 0.1f);
				meta.Root().SetAttribute("tile_width", i->TileSize().width);
				meta.Root().SetAttribute("tile_height", i->TileSize().height);

				std::stringstream flags;
				for (size_t j = 0; j < i->Count(); ++j)
					flags << i->At(j).flag << ',';

				meta.Root().AddChild("flags")->SetText(flags.str());

				meta.Save(IO::Path::SetExtension(_tileset_view->GetIdByTileset(*i), ".xml"));

			}

			if (!is_temporary_file) {

				_last_directory = IO::Path::GetDirectoryName(file_path);
				_current_file = file_path;

				_has_unsaved_changes = false;
				_updateWindowTitle();
			}

		}
		void RoomEditor::_startPlaytest() {

			if (!_room)
				return;

			// Export the room to a temporary file.
			std::string temp_path = IO::Path::GetTemporaryFilePath();
			_saveRoomToFile(temp_path, true);

			// Import the room back from the temporary file (why not just parse XML from string?).
			IRoomPtr test_room = _loadRoomFromFileIntoMemory(temp_path, false);
			test_room->Objects().Create<BackToEditorObject>(_context.Get<ROOM_MANAGER>().Room());

			_properties_exit_with_esc = _context.Get<GAME_MANAGER>().Properties().ExitWithEscapeKey;
			_context.Get<GAME_MANAGER>().Properties().ExitWithEscapeKey = false;

			_context.Get<ROOM_MANAGER>().SetRoom(test_room);

		}
		void RoomEditor::_roomView_OnMouseDown(Gui::WidgetMouseDownEventArgs& e) {

			if (!_room)
				return;

			if (_editor_mode == EDITOR_MODE_TILES) {

				if (_tileset_view->TilesetView() == nullptr || !_room_view->HasFocus())
					return;

				hvn3::RectangleI tile_selection = _tileset_view->TilesetView()->SelectedRegion();
				hvn3::PointF tile_map_position = _room_view->GlobalPositionToGridCell(e.Position());

				if (tile_map_position.x < 0.0f || tile_map_position.y < 0.0f || tile_map_position.x >= _room->Tiles().Columns() || tile_map_position.y >= _room->Tiles().Rows())
					return;

				int tile_index = 0;

				if (e.Button() == MouseButton::Left)
					// Calculate the value that will be assigned to the tile.
					tile_index = (tile_selection.Y() * _tileset_view->TilesetView()->Tileset().Columns()) + tile_selection.X() + 1;

				// Assign the tile to the tile map, and apply auto-tiling if applicable.
				_room->Tiles().SetTile(tile_map_position.x, tile_map_position.y, tile_index, 0);
				//AutoTileRenderer().ApplyAutoTilingAt(_room->GetTiles(), tile_map_position.x, tile_map_position.y, 0);

				_has_unsaved_changes = true;
				_updateWindowTitle();

			}

		}
		void RoomEditor::_roomView_OnMousePressed(Gui::WidgetMousePressedEventArgs& e) {

			if (_editor_mode == EDITOR_MODE_OBJECTS) {

				if (e.Button() == MouseButton::Left) {

					// Create a new object at the mouse position.

					if (!e.Position().In(_room_view->Bounds()))
						return;

					auto selected_item = _objects_view->SelectedItem();

					if (selected_item == nullptr)
						return;

					BLOCK_LISTENERS();

					IObjectPtr obj = _object_registry.MakeObject(selected_item->Text());
					PointF pos = _room_view->GlobalPositionToRoomPosition(e.Position(), true);

					obj->SetPosition(pos);

					// Store the "name" property so that it can be saved when the map is saved.
					// Both the name and ID of the object are required to be saved later (since different objects can have the same ID).
					_object_list.Add(obj);
					_object_list.SetProperty(obj, "name", selected_item->Text());

					_placing_object = obj.get();

					_room->Objects().Add(obj);

					UNBLOCK_LISTENERS();

					_has_unsaved_changes = true;
					_updateWindowTitle();

				}

			}

		}
		void RoomEditor::_unsubscribeEditorListeners() {

			ListenerCollection<IKeyboardListener>::Remove(this);
			ListenerCollection<IMouseListener>::Remove(this);
			ListenerCollection<IDisplayListener>::Remove(this);

			ListenerCollection<IKeyboardListener>::Remove(&_widgets);
			ListenerCollection<IMouseListener>::Remove(&_widgets);

		}
		void RoomEditor::_resubscribeEditorListeners() {

			ListenerCollection<IKeyboardListener>::Add(this);
			ListenerCollection<IMouseListener>::Add(this);
			ListenerCollection<IDisplayListener>::Add(this);

			ListenerCollection<IKeyboardListener>::Add(&_widgets);
			ListenerCollection<IMouseListener>::Add(&_widgets);

		}
		std::string RoomEditor::_makePathRelativeToResourceBaseDirectory(const std::string& path) {

			std::string::size_type index = StringUtils::IndexOf(path, _resource_base_directory);

			if (index == std::string::npos)
				return path;

			index += _resource_base_directory.size();

			return path.substr(index, path.size() - index);

		}

	}
}