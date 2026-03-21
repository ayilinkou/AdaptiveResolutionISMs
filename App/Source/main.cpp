#include "Core/Application.h"

int main()
{
	Core::ApplicationSpec appSpec;
	appSpec.Name = "Adaptive Resolution ISMs";

	Core::Application app(appSpec);

	return 0;
}