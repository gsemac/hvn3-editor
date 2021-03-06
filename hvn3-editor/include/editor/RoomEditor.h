#pragma once
#include "hvn3/gui2/GuiManager.h"
#include "hvn3/io/DisplayListener.h"
#include "hvn3/io/KeyboardListener.h"
#include "hvn3/io/MouseListener.h"
#include "hvn3/objects/Object.h"
#include "hvn3/rooms/Room.h"
#include "hvn3/xml/XmlResourceAdapterBase.h"

#include "editor/ObjectRegistry.h"
#include "editor/detail/ObjectList.h"

#include <string>

namespace hvn3 {

	namespace Gui {
		class ListBox;
		class RoomView;
		class Window;
	}

	namespace editor {

		class RoomEditorBackgroundsWidget;
		class RoomEditorStatusStripWidget;
		class RoomEditorTilesetsWidget;
		class RoomEditorViewsWidget;

		template <typename T>
		class RoomEditorXmlResourceAdapter;

		class RoomEditor :
			public Room,
			public DisplayListener,
			public MouseListener,
			public KeyboardListener {

			template <typename>
			friend class RoomEditorXmlResourceAdapter;
			friend class RoomEditorBackgroundsWidget;
			friend class RoomEditorTilesetsWidget;
			friend class RoomEditorViewsWidget;

			enum EDITOR_MODE {
				EDITOR_MODE_TILES,
				EDITOR_MODE_OBJECTS,
				EDITOR_MODE_BACKGROUNDS,
				EDITOR_MODE_VIEWS
			};

			class BackToEditorObject :
				public Object,
				public KeyboardListener {

			public:
				BackToEditorObject(IRoomPtr editor);

				void OnKeyPressed(KeyPressedEventArgs& e) override;
				void OnContextChanged(ContextChangedEventArgs& e) override;

			private:
				IRoomPtr _editor;
				Context _context;

			};

		public:
			RoomEditor();

			void OnCreate(RoomCreateEventArgs& e) override;
			void OnExit(RoomExitEventArgs& e) override;
			void OnContextChanged(ContextChangedEventArgs& e) override;
			void OnRender(DrawEventArgs& e) override;
			void OnUpdate(UpdateEventArgs& e) override;
			void OnDisplaySizeChanged(DisplaySizeChangedEventArgs& e) override;
			void OnMouseDown(MouseDownEventArgs& e) override;
			void OnMousePressed(MousePressedEventArgs& e) override;
			void OnMouseMove(MouseMoveEventArgs& e) override;
			void OnMouseReleased(MouseReleasedEventArgs& e) override;
			void OnMouseScroll(MouseScrollEventArgs& e) override;
			void OnKeyPressed(KeyPressedEventArgs& e) override;
			void OnKeyUp(KeyUpEventArgs& e) override;

			// Sets the object registry used for listing and instantiating objects.
			void SetObjectRegistry(const ObjectRegistry& registry);
			void SetRoomProvider(std::function<IRoomPtr(const SizeI&)>&& provider);

			// Returns the room currently being edited.
			IRoomPtr Room();

		private:
			std::string _editor_name;
			std::string _default_file_ext;
			std::string _current_file;
			std::string _resource_base_directory;
			std::string _last_directory;
			detail::ObjectList _object_list;
			//std::unordered_map<IObject*, std::vector<std::pair<String, String>>> _object_properties;

			bool _editor_initialized;
			bool _has_unsaved_changes;
			int _zoom_level;
			PointF _mouse_position;
			KeyModifiers _key_modifiers;
			MouseButton _mouse_buttons;
			EDITOR_MODE _editor_mode;
			detail::ObjectList::Item _selected_object;
			ObjectRegistry _object_registry;
			std::function<IRoomPtr(const SizeI&)> _room_provider;
			bool _properties_exit_with_esc;
			hvn3::IRoomPtr _room;
			Context _context;

			hvn3::Gui::GuiManager _widgets;
			hvn3::Gui::Window* _left_panel;
			hvn3::Gui::ListBox* _objects_view;
			hvn3::Gui::RoomView* _room_view;
			RoomEditorTilesetsWidget* _tileset_view;
			RoomEditorBackgroundsWidget* _backgrounds_view;
			RoomEditorViewsWidget* _views_view;
			RoomEditorStatusStripWidget* _status_strip;

			void _drawTileCursor(DrawEventArgs& e);
			void _unsubscribeEditorListeners();
			void _resubscribeEditorListeners();
			std::string _makePathRelativeToResourceBaseDirectory(const std::string& path);

			void _initializeUi(); // Initializes the editor user interface.
			void _initializeUiStyles(); // Initializes widget styles.
			void _initializeMenuStrip();
			void _initializeToolStrip();

			void _updateWindowTitle();
			void _hideAllPanelWindows();

			void _showPreferencesDialog();
			void _showRoomNewDialog();
			void _showRoomOpenDialog();
			void _showRoomSaveDialog();
			void _showRoomSaveAsDialog();
			void _showRoomViewContextMenu();

			void _loadPreferences(); // Loads user preferences from disk if preferences file exists.
			void _savePreferences(); // Saves user preferences to disk.

			void _createNewRoom(int width, int height);
			IRoomPtr _loadRoomFromFileIntoMemory(const std::string& file_path, bool load_resources_into_editor);
			void _loadRoomFromFileIntoEditor(const std::string& file_path);
			void _saveRoomToFile(const std::string& file_path, bool is_temporary_file);
			void _startPlaytest();

			void _roomView_OnMouseDown(Gui::WidgetMouseDownEventArgs& e);
			void _roomView_OnMousePressed(Gui::WidgetMousePressedEventArgs& e);

		};

	}
}