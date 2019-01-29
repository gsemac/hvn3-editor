#include "editor/widgets/RoomEditorStatusStripWidget.h"

namespace hvn3 {
	namespace editor {

		RoomEditorStatusStripWidget::RoomEditorStatusStripWidget() :
			_pop_animation(0.0f, 0.0f, 0) {

			_label = new Gui::Label("");

			AddItem(_label);

		}
		void RoomEditorStatusStripWidget::PopText(const String& text) {

			_pop_animation = Graphics::tween::From(_label->Y()).To(Height()).During(30).Do([this](Graphics::Tween<float>& t) {
				_label->SetY(Math::Ceiling(t.Value()));
			}).Then([=](Graphics::Tween<float>& t) {
				_temp_text = _label->Text();
				_label->SetText(text);
			}).From(Height()).To(0.0f).During(30).Do([this](Graphics::Tween<float>& t) {
				_label->SetY(Math::Ceiling(t.Value()));
			}).Wait(120).Then([=](Graphics::Tween<float>& t) {
				_label->SetText(_temp_text);
			});

		}
		void RoomEditorStatusStripWidget::SetText(const String& text) {

			// New text will scroll in from the bottom.

			_label->SetText(text);

		}
		void RoomEditorStatusStripWidget::OnUpdate(Gui::WidgetUpdateEventArgs& e) {

			MenuStrip::OnUpdate(e);

			_pop_animation.Step();

		}

	}
}