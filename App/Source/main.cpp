#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

#include "Core/Application/Application.h"
#include "Layer/AppLayer.h"
#include "Layer/UILayer.h"

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	Core::ApplicationSpec appSpec;
	appSpec.Name = "Adaptive Resolution ISMs";
	appSpec.WinSpec.Title = appSpec.Name;
	appSpec.WinSpec.Width = 1920;
	appSpec.WinSpec.Height = 1080;
	appSpec.WinSpec.Type = Core::WindowType::Fullscreen;
	appSpec.WinSpec.bUseVSync = false;
	appSpec.RenderSpec.NearPlane = 0.1f;
	appSpec.RenderSpec.FarPlane = 1000.f;
	appSpec.RenderSpec.WinSpec = appSpec.WinSpec;

	Core::Application app(appSpec);
	app.PushLayer<AppLayer>("AppLayer");
	app.PushLayer<UILayer>("UILayer");
	app.Run();

	return 0;
}