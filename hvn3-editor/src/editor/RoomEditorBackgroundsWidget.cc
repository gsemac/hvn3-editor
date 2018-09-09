#include "hvn3/backgrounds/BackgroundManager.h"
#include "hvn3/gui2/CheckBox.h"
#include "hvn3/gui2/ListBox.h"
#include "hvn3/gui2/MenuStrip.h"
#include "hvn3/gui2/TextBox.h"
#include "hvn3/gui2/WidgetLayoutBuilder.h"
#include "hvn3/native/FileDialog.h"
#include "hvn3/rooms/Room.h"

#include "editor/RoomEditorBackgroundsWidget.h"
#include "editor/RoomEditor.h"

namespace hvn3 {
		namespace editor {

			RoomEditorBackgroundsWidget::RoomEditorBackgroundsWidget(RoomEditor* editor) :
				Window("") {

				SetDockStyle(Gui::DockStyle::Fill);
				SetVisible(false);
				SetBorderStyle(Gui::Window::BorderStyle::None);

				_editor = editor;
				_block_current_background_update = false;

				_w_menustrip = new Gui::MenuStrip;
				_w_menustrip->AddItem("+")->SetEventHandler<Gui::WidgetEventType::OnMouseClick>([&](Gui::WidgetMouseClickEventArgs& e) {

					FileDialog dialog(FileDialogFlags::FileMustExist);
					dialog.SetFilter("Images|*.jpg;*.png");

					if (dialog.ShowDialog()) {

						Background bg(Graphics::Bitmap::FromFile(dialog.FileName()));

						_editor->Room()->GetBackgrounds().Add(bg);

						_backgrounds.push_back(bg);
						_backgrounds_list->AddItem(dialog.FileName());

						_backgrounds_list->SetSelectedIndex(_backgrounds_list->Count() - 1);

					}

				});

				_w_menustrip->AddItem("")->AddId(".arrow_d");
				_w_menustrip->AddItem("")->AddId(".arrow_u");

				_backgrounds_list = new Gui::ListBox;
				_backgrounds_list->SetDockStyle(Gui::DockStyle::Top);
				_backgrounds_list->SetAnchor(Gui::Anchor::Bottom);
				_backgrounds_list->SetHeight(300.0f);

				_w_cb_visible = new Gui::CheckBox("Visible when room starts");
				_w_cb_foreground = new Gui::CheckBox("Foreground image");
				_w_cb_tile_hor = new Gui::CheckBox("Tile Hor.");
				_w_cb_tile_vert = new Gui::CheckBox("Tile Vert.");
				_w_cb_stretch = new Gui::CheckBox("Stretch");

				_w_lb_x = new Gui::Label("X:");
				_w_lb_y = new Gui::Label("Y:");
				_w_lb_hor_speed = new Gui::Label("Hor. Speed:");
				_w_lb_vert_speed = new Gui::Label("Vert. Speed:");

				_w_tb_x = new Gui::TextBox(60, Gui::InputType::Decimal);
				_w_tb_y = new Gui::TextBox(60, Gui::InputType::Decimal);
				_w_tb_hor_speed = new Gui::TextBox(60, Gui::InputType::Decimal);
				_w_tb_vert_speed = new Gui::TextBox(60, Gui::InputType::Decimal);

				auto update_selected_background_lambda = [=]() {

					if (_block_current_background_update)
						return;

					int index = _backgrounds_list->SelectedIndex();

					if (index < 0)
						return;

					assert(static_cast<size_t>(index) < _backgrounds.size());

					Background& bg = _backgrounds[static_cast<size_t>(index)];

					bg.SetVisible(_w_cb_visible->Checked());
					bg.SetForeground(_w_cb_foreground->Checked());
					bg.SetTiledHorizontally(_w_cb_tile_hor->Checked());
					bg.SetTiledVertically(_w_cb_tile_vert->Checked());

					float x, y;

					if (StringUtils::TryParse(_w_tb_x->Text(), x))
						bg.SetOffset(x, bg.Offset().y);

					if (StringUtils::TryParse(_w_tb_y->Text(), y))
						bg.SetOffset(bg.Offset().x, y);

					_updateRoomBackgrounds();

				};

				_backgrounds_list->SetEventHandler<Gui::WidgetEventType::OnSelectedItemChanged>([=](Gui::WidgetSelectedItemChangedEventArgs& e) {

					if (e.Index() < 0)
						return;

					Background& bg = _backgrounds[static_cast<size_t>(e.Index())];

					_block_current_background_update = true;

					_w_cb_visible->SetChecked(bg.Visible());
					_w_cb_foreground->SetChecked(bg.IsForeground());
					_w_cb_tile_hor->SetChecked(bg.IsTiledHorizontally());
					_w_cb_tile_vert->SetChecked(bg.IsTiledVertically());

					_w_tb_x->SetText(StringUtils::ToString(bg.Offset().x));
					_w_tb_y->SetText(StringUtils::ToString(bg.Offset().y));
					_w_tb_hor_speed->SetText(StringUtils::ToString(bg.Velocity().X()));
					_w_tb_vert_speed->SetText(StringUtils::ToString(bg.Velocity().Y()));

					_block_current_background_update = false;

				});

				for (auto i : { _w_cb_visible, _w_cb_foreground, _w_cb_tile_hor, _w_cb_tile_vert, _w_cb_stretch }) {
					i->SetEventHandler<Gui::WidgetEventType::OnCheckedStateChanged>([=](Gui::WidgetCheckedStateChangedEventArgs& e) {
						update_selected_background_lambda();
					});
				}

				for (auto i : { _w_tb_x, _w_tb_y, _w_tb_hor_speed, _w_tb_vert_speed }) {
					i->SetEventHandler<Gui::WidgetEventType::OnTextChanged>([=](Gui::WidgetTextChangedEventArgs& e) {
						update_selected_background_lambda();
					});
				}

				GetChildren().Add(_w_menustrip);
				GetChildren().Add(_backgrounds_list);

				GetChildren().Add(_w_cb_visible);
				GetChildren().Add(_w_cb_foreground);
				GetChildren().Add(_w_cb_tile_hor);
				GetChildren().Add(_w_cb_tile_vert);
				GetChildren().Add(_w_cb_stretch);

				GetChildren().Add(_w_lb_x);
				GetChildren().Add(_w_lb_y);
				GetChildren().Add(_w_lb_hor_speed);
				GetChildren().Add(_w_lb_vert_speed);

				GetChildren().Add(_w_tb_x);
				GetChildren().Add(_w_tb_y);
				GetChildren().Add(_w_tb_hor_speed);
				GetChildren().Add(_w_tb_vert_speed);

			}

			String RoomEditorBackgroundsWidget::GetIdByBackground(const Background& background) const {

				int index = 0;

				for (auto i = _backgrounds.begin(); i != _backgrounds.end(); ++i, ++index)
					if (i->Bitmap() == background.Bitmap())
						return _backgrounds_list->ItemAt(index)->Text();

				return String::Empty;

			}
			const Background* RoomEditorBackgroundsWidget::GetBackgroundById(const String& id) const {

				size_t index = 0;

				for (auto i = _backgrounds_list->GetChildren().begin(); i != _backgrounds_list->GetChildren().end(); ++i, ++index)
					if (i->widget->Text() == id)
						return &_backgrounds[index];

				return nullptr;

			}
			void RoomEditorBackgroundsWidget::AddBackground(const String& id, const Background& background) {

				_backgrounds.push_back(background);
				_backgrounds_list->AddItem(id);

			}
			void RoomEditorBackgroundsWidget::OnRendererChanged(Gui::WidgetRendererChangedEventArgs& e) {

				WidgetBase::OnRendererChanged(e);

				Gui::WidgetLayoutBuilder builder;

				builder.PlaceBottomOf(_backgrounds_list, _w_menustrip);
				builder.PlaceBottom(_w_cb_visible);
				_w_cb_visible->SetX(5.0f);
				builder.PlaceBottom(_w_cb_foreground);
				builder.PlaceBottom(_w_cb_tile_hor);
				builder.PlaceBottom(_w_cb_stretch);
				builder.PlaceRightOf(_w_cb_tile_vert, _w_cb_tile_hor);
				builder.PlaceBottomOf(_w_lb_x, _w_cb_stretch);
				builder.PlaceRight(_w_tb_x);
				builder.PlaceRight(_w_lb_y);
				builder.PlaceRight(_w_tb_y);

				builder.PlaceBottomOf(_w_tb_hor_speed, _w_tb_y);
				builder.PlaceLeft(_w_lb_hor_speed);
				builder.PlaceBottomOf(_w_tb_vert_speed, _w_tb_hor_speed);
				builder.PlaceLeft(_w_lb_vert_speed);

			}

			void RoomEditorBackgroundsWidget::_updateRoomBackgrounds() {

				size_t index = 0;

				_editor->Room()->GetBackgrounds().ForEach([&, this](Background& i) {
					i = _backgrounds[index++];
					HVN3_CONTINUE;
				});

			}

		}
}