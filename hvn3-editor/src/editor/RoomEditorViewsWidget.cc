#include "editor/RoomEditorViewsWidget.h"
#include "hvn3/gui2/MenuStrip.h"
#include "hvn3/gui2/ListBox.h"

namespace hvn3 {
	namespace editor {

		RoomEditorViewsWidget::RoomEditorViewsWidget() :
			Window("") {

			SetDockStyle(Gui::DockStyle::Fill);
			SetBorderStyle(Gui::Window::BorderStyle::None);
			SetVisible(false);

			auto menu_strip = new Gui::MenuStrip;
			menu_strip->AddItem("+");
			menu_strip->AddItem("")->AddId(".arrow_d");
			menu_strip->AddItem("")->AddId(".arrow_u");

			auto views_list = new Gui::ListBox;
			views_list->SetDockStyle(Gui::DockStyle::Top);
			views_list->SetAnchor(Gui::Anchor::Bottom);
			views_list->SetHeight(300.0f);

			GetChildren().Add(menu_strip);
			GetChildren().Add(views_list);

		}

	}
}