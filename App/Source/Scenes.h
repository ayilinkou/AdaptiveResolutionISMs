#include <string_view>

struct SceneInfo
{
	std::string_view ModelPath;
	std::string_view TexturesRoot;
	const float PointCloudDensity = 0.f;
};

#ifdef UILAYER_CPP

namespace Scenes {
	constexpr SceneInfo EmeraldSquareDay = { "Models/EmeraldSquare_v4_1/EmeraldSquare_Day.fbx", "Models/EmeraldSquare_v4_1", 0.0005f };
	constexpr SceneInfo EmeraldSquareDusk = { "Models/EmeraldSquare_v4_1/EmeraldSquare_Dusk.fbx", "Models/EmeraldSquare_v4_1", 0.0005f };
	constexpr SceneInfo BistroExterior = { "Models/Bistro_v5_2/BistroExterior.fbx", "Models/Bistro_v5_2" };
	constexpr SceneInfo BistroInterior = { "Models/Bistro_v5_2/BistroInterior.fbx", "Models/Bistro_v5_2" };
	constexpr SceneInfo BistroInteriorWine = { "Models/Bistro_v5_2/BistroInterior_Wine.fbx", "Models/Bistro_v5_2" };

}

void UILayer::LoadEmeraldSquareNight()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
	{
		pAppLayer->LoadScene(Scenes::EmeraldSquareDusk);
		Core::LightManager::GetAmbientStrengthRef() = 0.05f;
		Core::Renderer::Get()->SetClearColor({ 0.05f, 0.05f, 0.1f, 1.f });

		Core::Application::Get()->GetCamera()->SetPosition(-18.2f, 5.7f, -0.1f);
		Core::Application::Get()->GetCamera()->SetRotation(17.1f, -89.9f);

		DirectX::XMFLOAT3 color = { 1.f, 0.8f, 0.4f };
		DirectX::XMFLOAT3 attenuation = { 1.f, 0.f, 0.f };
		DirectX::XMFLOAT3 dir = { 0.f, -1.f, 0.f };
		auto streetLightOne = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightOne->SetName("Street Light 1");
		streetLightOne->SetPosition(-55.6f, 5.7f, 4.9f);
		streetLightOne->SetIntensity(20.f);
		pAppLayer->AddLight(std::move(streetLightOne));

		auto streetLightTwo = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightTwo->SetName("Street Light 2");
		streetLightTwo->SetPosition(-55.6f, 5.7f, -5.6f);
		streetLightTwo->SetIntensity(20.f);
		pAppLayer->AddLight(std::move(streetLightTwo));

		auto streetLightThree = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightThree->SetName("Street Light 3");
		streetLightThree->SetPosition(-66.2f, 5.7f, -5.6f);
		streetLightThree->SetIntensity(20.f);
		pAppLayer->AddLight(std::move(streetLightThree));

		auto streetLightFour = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightFour->SetName("Street Light 4");
		streetLightFour->SetPosition(-71.3f, 5.7f, -21.f);
		streetLightFour->SetIntensity(20.f);
		pAppLayer->AddLight(std::move(streetLightFour));

		auto streetLightFive = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightFive->SetName("Street Light 5");
		streetLightFive->SetPosition(-21.2f, 5.7f, -5.6f);
		streetLightFive->SetIntensity(20.f);
		pAppLayer->AddLight(std::move(streetLightFive));

		auto parkSpotlight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		parkSpotlight->SetName("Park Spotlight");
		parkSpotlight->SetPosition(-57.3f, 0.3f, -19.6f);
		parkSpotlight->SetDirection(-0.46f, 0.67f, 0.6f);
		parkSpotlight->SetIntensity(30.f);
		pAppLayer->AddLight(std::move(parkSpotlight));

		auto cornerSpotlight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		cornerSpotlight->SetName("Corner Spotlight");
		cornerSpotlight->SetPosition(-68.8f, 0.7f, 8.f);
		cornerSpotlight->SetIntensity(20.f);
		cornerSpotlight->SetDirection(0.74f, 0.15f, 0.65f);
		cornerSpotlight->SetAngles(0.f, 89.f);
		pAppLayer->AddLight(std::move(cornerSpotlight));

		auto spotLightSix = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		spotLightSix->SetName("Crossing Light");
		spotLightSix->SetPosition(-41.1f, 5.7f, -22.1f);
		spotLightSix->SetIntensity(20.f);
		spotLightSix->SetAttenuation(0.f, 0.1f, 7.f);
		spotLightSix->SetDirection(0.2f, -0.446f, 0.872f);
		spotLightSix->SetAngles(0.f, 40.f);
		pAppLayer->AddLight(std::move(spotLightSix));

		auto busOneHeadlightLeft = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		busOneHeadlightLeft->SetName("Bus 1 left headlight");
		busOneHeadlightLeft->SetPosition(-50.4f, 1.f, 4.4f);
		busOneHeadlightLeft->SetIntensity(10.f);
		busOneHeadlightLeft->SetDirection(-0.98f, -0.2f, 0.f);
		busOneHeadlightLeft->SetAngles(0.f, 50.f);
		busOneHeadlightLeft->SetAttenuation(0.f, 0.3f, 5.5f);
		pAppLayer->AddLight(std::move(busOneHeadlightLeft));

		auto busOneHeadlightRight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		busOneHeadlightRight->SetName("Bus 1 right headlight");
		busOneHeadlightRight->SetPosition(-50.4f, 1.f, 6.1f);
		busOneHeadlightRight->SetIntensity(10.f);
		busOneHeadlightRight->SetDirection(-0.98f, -0.2f, 0.f);
		busOneHeadlightRight->SetAngles(0.f, 50.f);
		busOneHeadlightRight->SetAttenuation(0.f, 0.3f, 5.5f);
		pAppLayer->AddLight(std::move(busOneHeadlightRight));

		color = { 1.f, 1.f, 1.f };
		dir = { 0.2f, -0.6f, 0.4f };
		auto dirLight = std::make_unique<Core::DirectionalLight>(color, dir);
		dirLight->SetIntensity(0.1f);
		pAppLayer->AddLight(std::move(dirLight));
	}
	ToggleVisibility();
}

void UILayer::LoadEmeraldSquareDusk()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
	{
		pAppLayer->LoadScene(Scenes::EmeraldSquareDusk);

		auto dirLight = std::make_unique<Core::DirectionalLight>(DirectX::XMFLOAT3(1.f, 0.7f, 0.5f), DirectX::XMFLOAT3(0.9f, -0.3f, 0.2f));
		dirLight->SetIntensity(1.5f);
		pAppLayer->AddLight(std::move(dirLight));

		Core::Renderer::Get()->SetClearColor({ 1.f, 0.7f, 0.5f, 1.f });
	}
	ToggleVisibility();
}

void UILayer::LoadBistroExterior()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
		pAppLayer->LoadScene(Scenes::BistroExterior);

	Core::LightManager::GetAmbientStrengthRef() = 0.2f;
	DirectX::XMFLOAT4 skyboxColor = { 1.f, 0.7f, 0.5f, 1.f };
	Core::Renderer::Get()->SetClearColor(skyboxColor);

	Core::Application::Get()->GetCamera()->SetPosition(-21.2f, 5.8f, -2.f);
	Core::Application::Get()->GetCamera()->SetRotation(8.9f, -277.f);

	for (auto* pDirLight : Core::LightManager::GetDirectionalLights())
	{
		pDirLight->SetDirection({ 0.3f, -0.9f, -0.3f });
		pDirLight->SetColor(skyboxColor.x, skyboxColor.y, skyboxColor.z);
	}

	ToggleVisibility();
}

void UILayer::LoadBistroInterior()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
		pAppLayer->LoadScene(Scenes::BistroInterior);
	ToggleVisibility();
}

void UILayer::LoadBistroInteriorWine()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
		pAppLayer->LoadScene(Scenes::BistroInteriorWine);
	ToggleVisibility();
}

#endif