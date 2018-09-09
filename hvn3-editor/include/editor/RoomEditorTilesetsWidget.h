#pragma once
#include "hvn3/gui2/Window.h"
#include "hvn3/tilesets/Tileset.h"

#include <vector>

namespace hvn3 {

	namespace Gui {
		class ContextMenu;
		class TilesetView;
	}

	namespace editor {

		class RoomEditor;

		class RoomEditorTilesetsWidget :
			public Gui::Window {

		public:
			RoomEditorTilesetsWidget(RoomEditor* editor);

			Gui::TilesetView* TilesetView();
			const std::vector<Tileset>& Tilesets() const;
			String GetIdByTileset(const Tileset& tileset) const;
			void AddTileset(const Tileset& tileset, const String& id);

		private:
			RoomEditor* _editor;
			Gui::ContextMenu* tilesets_context_menu;
			Gui::ContextMenu* layers_context_menu;
			Gui::TilesetView* tileset_view;
			std::vector<Tileset> _tilesets;

			void _openTileset();

		};

	}
}