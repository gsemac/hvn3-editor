#pragma once
#include "hvn3/gui2/MenuStrip.h"
#include "hvn3/gui2/Label.h"
#include "hvn3/graphics/Tween.h"

namespace hvn3 {
	namespace editor {

		class RoomEditorStatusStripWidget :
			public Gui::MenuStrip {

		public:
			RoomEditorStatusStripWidget();

			void PopText(const String& text);
			void SetText(const String& text) override;
			void OnUpdate(Gui::WidgetUpdateEventArgs& e) override;

		private:
			Gui::Label* _label;
			String _temp_text;
			Graphics::Tween<float> _pop_animation;

		};

	}
}