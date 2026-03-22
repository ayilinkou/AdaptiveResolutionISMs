#include "Core/Application.h"

int main()
{
	Core::ApplicationSpec appSpec;
	appSpec.Name = "Adaptive Resolution ISMs";
	appSpec.WinSpec.Title = appSpec.Name;
	appSpec.WinSpec.Width = 1280;
	appSpec.WinSpec.Height = 720;
	appSpec.WinSpec.bFullscreen = false;
	appSpec.WinSpec.bResizable = false;
	appSpec.WinSpec.bUseVSync = false;

	Core::Application app(appSpec);
	app.Run();

	return 0;
}