#pragma once
#include "hvn3/backgrounds/Background.h"
#include "hvn3/gui2/Window.h"

#include <vector>

namespace hvn3 {

	namespace Gui {
		class CheckBox;
		class Label;
		class ListBox;
		class MenuStrip;
		class TextBox;
	}

	namespace editor {

		class RoomEditor;

		class RoomEditorBackgroundsWidget :
			public Gui::Window {

		public:
			RoomEditorBackgroundsWidget(RoomEditor* editor);

			String GetIdByBackground(const Background& background) const;
			const Background* GetBackgroundById(const String& id) const;
			void AddBackground(const String& id, const Background& background);

			void OnRendererChanged(Gui::WidgetRendererChangedEventArgs& e) override;

		private:
			RoomEditor* _editor;
			Gui::ListBox* _backgrounds_list;
			std::vector<Background> _backgrounds;
			bool _block_current_background_update;
			Gui::MenuStrip* _w_menustrip;
			Gui::CheckBox *_w_cb_visible,
				*_w_cb_foreground,
				*_w_cb_tile_hor,
				*_w_cb_tile_vert,
				*_w_cb_stretch;
			Gui::Label *_w_lb_x,
				*_w_lb_y,
				*_w_lb_hor_speed,
				*_w_lb_vert_speed;
			Gui::TextBox *_w_tb_x,
				*_w_tb_y,
				*_w_tb_hor_speed,
				*_w_tb_vert_speed;

			void _updateRoomBackgrounds();

		};

	}
}