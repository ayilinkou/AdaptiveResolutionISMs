#include "Core/Application.h"
#include "AppLayer.h"

int main()
{
	Core::ApplicationSpec appSpec;
	appSpec.Name = "Adaptive Resolution ISMs";
	appSpec.WinSpec.Title = appSpec.Name;
	appSpec.WinSpec.Width = 1280;
	appSpec.WinSpec.Height = 720;
	appSpec.WinSpec.bFullscreen = false;
	appSpec.WinSpec.bUseVSync = false;
	appSpec.RenderSpec.NearPlane = 0.1f;
	appSpec.RenderSpec.FarPlane = 1000.f;
	appSpec.RenderSpec.WinSpec = appSpec.WinSpec;

	Core::Application app(appSpec);
	app.PushLayer<AppLayer>("AppLayer");
	app.Run();

	return 0;
}