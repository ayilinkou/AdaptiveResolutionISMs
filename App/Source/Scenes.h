#include <string_view>
#include <filesystem>

#include "Core/Utility/Utility.h"

struct SceneInfo
{
	std::string_view ModelPath;
	std::string_view TexturesRoot;
	const float PointCloudDensity = 0.f;
};

#ifdef UILAYER_CPP

namespace Scenes {
	constexpr SceneInfo EmeraldSquareDusk = { "Models/EmeraldSquare_v4_1/EmeraldSquare_Dusk.fbx", "Models/EmeraldSquare_v4_1", 0.0005f };
	constexpr SceneInfo BistroExterior = { "Models/Bistro_v5_2/BistroExterior.fbx", "Models/Bistro_v5_2", 0.1f };
	constexpr SceneInfo SanMiguel = { "Models/San_Miguel/san-miguel-low-poly.obj", "Models/San_Miguel", 0.01f };
}

DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
	return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

void UILayer::LoadEmeraldSquareNight()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
	{
		pAppLayer->LoadScene(Scenes::EmeraldSquareDusk);
		Core::LightManager::GetAmbientStrengthRef() = 0.001f;
		Core::Renderer::Get()->SetClearColor({ 0.05f, 0.05f, 0.1f, 1.f });

		Core::Application::Get()->GetCamera()->SetPosition(-18.2f, 5.7f, -0.1f);
		Core::Application::Get()->GetCamera()->SetRotation(17.1f, -89.9f);

		DirectX::XMFLOAT3 color = { 1.f, 0.8f, 0.4f };
		DirectX::XMFLOAT3 attenuation = { 1.f, 0.f, 0.f };
		DirectX::XMFLOAT3 dir = { 0.f, -1.f, 0.f };
		float streetLightIntensity = 5.f;
		auto streetLightOne = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightOne->SetName("Street Light 1");
		streetLightOne->SetPosition(-55.6f, 5.7f, 4.9f);
		streetLightOne->SetIntensity(streetLightIntensity);
		pAppLayer->AddLight(std::move(streetLightOne));

		auto streetLightTwo = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightTwo->SetName("Street Light 2");
		streetLightTwo->SetPosition(-55.6f, 5.7f, -5.6f);
		streetLightTwo->SetIntensity(streetLightIntensity);
		pAppLayer->AddLight(std::move(streetLightTwo));

		auto streetLightThree = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightThree->SetName("Street Light 3");
		streetLightThree->SetPosition(-66.2f, 5.7f, -5.6f);
		streetLightThree->SetIntensity(streetLightIntensity);
		pAppLayer->AddLight(std::move(streetLightThree));

		auto streetLightFour = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightFour->SetName("Street Light 4");
		streetLightFour->SetPosition(-71.3f, 5.7f, -21.f);
		streetLightFour->SetIntensity(streetLightIntensity);
		pAppLayer->AddLight(std::move(streetLightFour));

		auto streetLightFive = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		streetLightFive->SetName("Street Light 5");
		streetLightFive->SetPosition(-21.2f, 5.7f, -5.6f);
		streetLightFive->SetIntensity(streetLightIntensity);
		pAppLayer->AddLight(std::move(streetLightFive));

		auto parkSpotlight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		parkSpotlight->SetName("Park Spotlight");
		parkSpotlight->SetPosition(-57.3f, 0.3f, -19.6f);
		parkSpotlight->SetDirection(-0.46f, 0.67f, 0.6f);
		parkSpotlight->SetIntensity(5.f);
		pAppLayer->AddLight(std::move(parkSpotlight));

		auto cornerSpotlight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		cornerSpotlight->SetName("Corner Spotlight");
		cornerSpotlight->SetPosition(-68.8f, 0.7f, 8.f);
		cornerSpotlight->SetIntensity(5.f);
		cornerSpotlight->SetDirection(0.74f, 0.15f, 0.65f);
		cornerSpotlight->SetAngles(0.f, 89.f);
		pAppLayer->AddLight(std::move(cornerSpotlight));

		auto spotLightSix = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		spotLightSix->SetName("Crossing Light");
		spotLightSix->SetPosition(-41.1f, 5.7f, -22.1f);
		spotLightSix->SetIntensity(5.f);
		spotLightSix->SetAttenuation(0.f, 0.1f, 7.f);
		spotLightSix->SetDirection(0.2f, -0.446f, 0.872f);
		spotLightSix->SetAngles(0.f, 40.f);
		pAppLayer->AddLight(std::move(spotLightSix));

		auto busOneHeadlightLeft = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		busOneHeadlightLeft->SetName("Bus 1 left headlight");
		busOneHeadlightLeft->SetPosition(-50.4f, 1.f, 4.4f);
		busOneHeadlightLeft->SetIntensity(2.f);
		busOneHeadlightLeft->SetDirection(-0.98f, -0.2f, 0.f);
		busOneHeadlightLeft->SetAngles(0.f, 50.f);
		busOneHeadlightLeft->SetAttenuation(0.f, 0.3f, 5.5f);
		pAppLayer->AddLight(std::move(busOneHeadlightLeft));

		auto busOneHeadlightRight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
		busOneHeadlightRight->SetName("Bus 1 right headlight");
		busOneHeadlightRight->SetPosition(-50.4f, 1.f, 6.1f);
		busOneHeadlightRight->SetIntensity(2.f);
		busOneHeadlightRight->SetDirection(-0.98f, -0.2f, 0.f);
		busOneHeadlightRight->SetAngles(0.f, 50.f);
		busOneHeadlightRight->SetAttenuation(0.f, 0.3f, 5.5f);
		pAppLayer->AddLight(std::move(busOneHeadlightRight));

		color = { 1.f, 1.f, 1.f };
		dir = { 0.2f, -0.6f, 0.4f };
		auto dirLight = std::make_unique<Core::DirectionalLight>(color, dir);
		dirLight->SetIntensity(0.005f);
		pAppLayer->AddLight(std::move(dirLight));
	}
	ToggleVisibility();
}

void UILayer::LoadBistroExterior()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
		pAppLayer->LoadScene(Scenes::BistroExterior);

	Core::LightManager::GetAmbientStrengthRef() = 0.003f;
	DirectX::XMFLOAT4 skyboxColor = { 0.05f, 0.1f, 0.1f, 1.f };
	Core::Renderer::Get()->SetClearColor(skyboxColor);

	Core::Application::Get()->GetCamera()->SetPosition(-21.2f, 5.8f, -2.f);
	Core::Application::Get()->GetCamera()->SetRotation(8.9f, -277.f);

	Core::LightManager::GetDirectionalLights()[0]->SetActive(false);

	DirectX::XMFLOAT3 color = { 1.f, 0.8f, 0.4f };
	DirectX::XMFLOAT3 attenuation = { 1.f, 0.f, 0.f };
	float lightIntensity = 5.f;

	// street lights
	/*const DirectX::XMFLOAT3 streetLightPositions[3] = {
		{  -6.93f, 6.9f,  6.7f },
		{  -3.30f, 6.9f, -7.4f },
		{ -15.43f, 6.9f, -3.3f }
	};

	const DirectX::XMFLOAT3 streetLightOffsets[4] = {
		{  0.6f, 0.0f,  0.0f },
		{ -0.6f, 0.0f,  0.0f },
		{  0.0f, 0.0f,  0.6f },
		{  0.0f, 0.0f, -0.6f },
	};

	const DirectX::XMFLOAT3 streetLightDirs[4] = {
		{ -0.5f, -0.866f,  0.0f },
		{  0.5f, -0.866f,  0.0f },
		{  0.0f, -0.866f, -0.5f },
		{  0.0f, -0.866f,  0.5f }
	};

	for (UINT i = 0u; i < _countof(streetLightPositions); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			auto streetLight = std::make_unique<Core::SpotLight>(color, attenuation, streetLightDirs[j]);
			std::string name = "Street Light " + std::to_string(i) + "_" + std::to_string(j);
			streetLight->SetName(name);
			streetLight->SetPosition(streetLightPositions[i] + streetLightOffsets[j]);
			streetLight->SetIntensity(lightIntensity);
			streetLight->SetAngles(0.f, 89.f);
			pAppLayer->AddLight(std::move(streetLight));
		}
	}*/

	const DirectX::XMFLOAT3 shopLightPositions[8] = {
		{   3.17f, 6.9f, 16.8f },
		{   1.27f, 6.9f, 13.3f },
		{  -0.33f, 6.9f, 10.4f },
		{  -2.13f, 6.9f,  7.0f },
		{   2.03f, 6.9f, -3.5f },
		{   5.77f, 6.9f, -4.9f },
		{   8.97f, 6.9f, -6.1f },
		{  12.47f, 6.6f, -7.3f },
	};

	const DirectX::XMFLOAT3 shopLightDirs[8] = {
		{ -0.297f, -0.948f,  0.113f},
		{ -0.297f, -0.948f,  0.113f},
		{ -0.297f, -0.948f,  0.113f},
		{ -0.297f, -0.948f,  0.113f},
		{ -0.163f, -0.975f, -0.151f},
		{ -0.163f, -0.975f, -0.151f},
		{ -0.163f, -0.975f, -0.151f},
		{ -0.163f, -0.975f, -0.151f}
	};

	for (UINT i = 0u; i < _countof(shopLightPositions); i++)
	{
		auto shopLight = std::make_unique<Core::SpotLight>(color, attenuation, shopLightDirs[i]);
		std::string name = "Shop Light " + std::to_string(i);
		shopLight->SetName(name);
		shopLight->SetPosition(shopLightPositions[i]);
		shopLight->SetIntensity(lightIntensity);
		shopLight->SetAngles(0.f, 89.f);
		pAppLayer->AddLight(std::move(shopLight));
	}

	ToggleVisibility();
}

void UILayer::LoadSanMiguel()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
		pAppLayer->LoadScene(Scenes::SanMiguel);

	Core::Application::Get()->GetCamera()->SetPosition(-21.2f, 5.8f, -2.f);
	Core::Application::Get()->GetCamera()->SetRotation(8.9f, -277.f);

	Core::LightManager::GetAmbientStrengthRef() = 0.01f;
	DirectX::XMFLOAT4 skyboxColor = { 0.05f, 0.1f, 0.1f, 1.f };
	Core::Renderer::Get()->SetClearColor(skyboxColor);

	DirectX::XMFLOAT3 color = { 1.f, 1.f, 1.f };
	DirectX::XMFLOAT3 dir = { 0.11f, -0.98f, -0.18f };
	auto dirLight = std::make_unique<Core::DirectionalLight>(color, dir);
	pAppLayer->AddLight(std::move(dirLight));

	color = { 1.f, 0.8f, 0.4f };
	dir = { 0.f, -1.f, 0.f };
	DirectX::XMFLOAT3 attenuation = { 1.f, 0.f, 0.f };
	float lightIntensity = 1.f;
	auto lightOne = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lightOne->SetName("Light 1");
	lightOne->SetPosition(-55.6f, 5.7f, 4.9f);
	lightOne->SetIntensity(lightIntensity);
	pAppLayer->AddLight(std::move(lightOne));

	auto lightTwo = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lightTwo->SetName("Light 2");
	lightTwo->SetPosition(-55.6f, 5.7f, -5.6f);
	lightTwo->SetIntensity(lightIntensity);
	pAppLayer->AddLight(std::move(lightTwo));

	auto lightThree = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lightThree->SetName("Light 3");
	lightThree->SetPosition(-66.2f, 5.7f, -5.6f);
	lightThree->SetIntensity(lightIntensity);
	pAppLayer->AddLight(std::move(lightThree));

	auto lightFour = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lightFour->SetName("Light 4");
	lightFour->SetPosition(-71.3f, 5.7f, -21.f);
	lightFour->SetIntensity(lightIntensity);
	pAppLayer->AddLight(std::move(lightFour));

	ToggleVisibility();
}

void UILayer::LoadFromFile()
{	
	std::vector<COMDLG_FILTERSPEC> fileTypes;
	fileTypes.push_back({ L"All Files", L"*.*" });
	fileTypes.push_back({ L"Models",    L"*.fbx;*.gltf;*.obj" });

	std::wstring wPath = Core::StaticUtils::OpenFileDialog(fileTypes);

	if (wPath.empty())
		return;

	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
	{
		std::filesystem::path path(wPath);
		std::filesystem::path parentPath(path.parent_path());

		std::string modelPath = path.string();
		std::string texturesRoot = parentPath.string();

		SceneInfo scene;
		scene.ModelPath = modelPath;
		scene.TexturesRoot = texturesRoot;
		pAppLayer->LoadScene(scene);

		ToggleVisibility();
	}
}

#endif