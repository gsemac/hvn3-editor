#include "editor/RoomEditor.h"
#include "hvn3/hvn3.h"

using namespace hvn3;

int main(int argc, char* argv[]) {

	// Initialize game properties.
	System::Properties properties;
	properties.DisplaySize = SizeI(960, 720);
	properties.DisplayTitle = "Rescind7";
	properties.FrameRate = 60.0;
	properties.FixedFrameRate = true;
	properties.DebugMode = false;
	properties.ScalingMode = ScalingMode::Fixed;
	properties.DisplayFlags = DisplayFlags::Resizable;

	// Create a game manager for running the game.
	GameManager manager(properties);

	Graphics::Bitmap::SetDefaultBitmapFlags(Graphics::BitmapFlags::Default);

	// Initialize the editor.
	RoomPtr editor(new editor::RoomEditor);
	manager.Context().GetRooms().SetRoom(editor);

	// Run the game loop.
	manager.Loop();

	return 0;

}