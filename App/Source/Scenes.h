#include <string_view>
#include <filesystem>

#include "Core/Utility/Utility.h"

struct SceneInfo
{
	std::string_view ModelPath;
	std::string_view TexturesRoot;
	const float PointCloudDensity = 0.f;
	const float MinBiasShadowMap = 0.f;
	const float MaxBiasShadowMap = 0.f;
	const float MinBiasISM = 0.f;
	const float MaxBiasISM = 0.f;
	const float MinBiasLowISM = 0.f;
	const float MaxBiasLowISM = 0.f;
};

#ifdef UILAYER_CPP

namespace Scenes {
	SceneInfo EmeraldSquare = {
		"Models/EmeraldSquare_v4_1/EmeraldSquare_Dusk.fbx",
		"Models/EmeraldSquare_v4_1",
		0.001f,
		0.0001f,
		0.0002f,
		0.0002f,
		0.0012f,
		0.0001f,
		0.0020f
	};

	SceneInfo BistroExterior = {
		"Models/Bistro_v5_2/BistroExterior.fbx",
		"Models/Bistro_v5_2",
		0.1f,
		0.0001f,
		0.0001f,
		0.0001f,
		0.0100f,
		0.0001f,
		0.0050f
	};

	SceneInfo SanMiguel = {
		"Models/San_Miguel/san-miguel-low-poly.obj",
		"Models/San_Miguel",
		1000.f,
		0.0001f,
		0.0005f,
		0.0001f,
		0.0050f,
		0.0001f,
		0.0060f
	};
}

DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
	return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

void UILayer::LoadEmeraldSquare()
{
	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	if (pAppLayer)
	{
		pAppLayer->LoadScene(Scenes::EmeraldSquare);
		Core::LightManager::GetAmbientStrengthRef() = 0.02f;
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

	Core::LightManager::GetAmbientStrengthRef() = 0.03f;
	DirectX::XMFLOAT4 skyboxColor = { 0.05f, 0.1f, 0.1f, 1.f };
	Core::Renderer::Get()->SetClearColor(skyboxColor);

	Core::Application::Get()->GetCamera()->SetPosition(-21.2f, 5.8f, -2.f);
	Core::Application::Get()->GetCamera()->SetRotation(8.9f, -277.f);

	for (auto* pDirLight : Core::LightManager::GetDirectionalLights())
	{
		pDirLight->SetDirection({ 0.09f, -0.71f, -0.7f });
		pDirLight->SetColor(0.9f, 0.45f, 0.1f);
		pDirLight->SetIntensity(0.1f);
	}

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

	Core::Application::Get()->GetCamera()->SetPosition(6.34f, 1.73f, -0.69f);
	Core::Application::Get()->GetCamera()->SetRotation(2.f, 119.f);

	Core::LightManager::GetAmbientStrengthRef() = 0.01f;
	DirectX::XMFLOAT4 skyboxColor = { 0.9f, 0.55f, 0.25f, 1.f };
	Core::Renderer::Get()->SetClearColor(skyboxColor);

	DirectX::XMFLOAT3 color = { 1.f, 0.8f, 0.4f };
	DirectX::XMFLOAT3 dir = { -0.45f, -0.893f, 0.f };
	DirectX::XMFLOAT3 attenuation = { 2.f, 0.f, 0.f };
	float lightIntensity = 5.f;
	float radius = 20.f;
	float innerAngle = 82.5f;
	float outerAngle = 89.f;
	auto lampLight1_1 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lampLight1_1->SetName("Lamp Light 1_1");
	lampLight1_1->SetPosition(25.1f, 2.8f, -2.f);
	lampLight1_1->SetIntensity(lightIntensity);
	lampLight1_1->SetRadius(radius);
	lampLight1_1->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(lampLight1_1));

	dir = { 0.45f, -0.893f, 0.f };
	auto lampLight1_2 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lampLight1_2->SetName("Lamp Light 1_2");
	lampLight1_2->SetPosition(25.3f, 3.7f, -2.f);
	lampLight1_2->SetIntensity(lightIntensity);
	lampLight1_2->SetRadius(radius);
	lampLight1_2->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(lampLight1_2));

	dir = { -0.45f, -0.893f, 0.f };
	auto lampLight2_1 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lampLight2_1->SetName("Lamp Light 2_1");
	lampLight2_1->SetPosition(25.1f, 9.f, -2.f);
	lampLight2_1->SetIntensity(lightIntensity);
	lampLight2_1->SetRadius(30.f);
	lampLight2_1->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(lampLight2_1));

	dir = { 0.45f, -0.893f, 0.f };
	auto lampLight2_2 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	lampLight2_2->SetName("Lamp Light 2_2");
	lampLight2_2->SetPosition(25.1f, 9.6f, -2.f);
	lampLight2_2->SetIntensity(lightIntensity);
	lampLight2_2->SetRadius(radius);
	lampLight2_2->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(lampLight2_2));

	attenuation = { 7.f, 0.f, 0.f };
	dir = { 0.63f, -0.63f, -0.46f };
	auto roomLight1 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	roomLight1->SetName("Room Light 1");
	roomLight1->SetPosition(6.7f, 4.6f, -0.7f);
	roomLight1->SetIntensity(50.f);
	roomLight1->SetRadius(30.f);
	roomLight1->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(roomLight1));

	attenuation = { 2.f, 0.f, 0.f };
	dir = { 0.685f, -0.685f, -0.25f };
	auto roomLight2 = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	roomLight2->SetName("Room Light 2");
	roomLight2->SetPosition(7.1f, 4.6f, -12.1f);
	roomLight2->SetIntensity(50.f);
	roomLight2->SetRadius(30.f);
	roomLight2->SetAngles(innerAngle, outerAngle);
	pAppLayer->AddLight(std::move(roomLight2));

	attenuation = { 1.f, 0.f, 0.f };
	dir = { 0.22f, -0.85f, -0.54f };
	auto plantLight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	plantLight->SetName("Plant Light");
	plantLight->SetPosition(15.f, 4.f, 2.f);
	plantLight->SetIntensity(10.f);
	plantLight->SetRadius(20.f);
	plantLight->SetAngles(0.f, 89.f);
	pAppLayer->AddLight(std::move(plantLight));

	attenuation = { 2.f, 0.f, 0.f };
	dir = { 0.48f, -0.88f, 0.f };
	auto alleyLight = std::make_unique<Core::SpotLight>(color, attenuation, dir);
	alleyLight->SetName("Alley Light");
	alleyLight->SetPosition(44.8f, 5.9f, -2.6f);
	alleyLight->SetIntensity(5.f);
	alleyLight->SetRadius(10.f);
	alleyLight->SetAngles(0.f, 23.f);
	pAppLayer->AddLight(std::move(alleyLight));

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