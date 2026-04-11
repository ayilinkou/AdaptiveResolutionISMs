#include <stdexcept>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "Vendor/stb/stb_image.h"
#include "RMAECalculator.h"

double RMAECalculator::CalculateFromFiles(std::string_view truth, std::string_view test)
{
	unsigned char* data = nullptr;
	int width, height, channels;
	std::filesystem::path truthPath(truth);
	std::string truthStr = truthPath.string();
	const int channelsRequired = 3; // ignore alpha
	data = stbi_load(truthStr.c_str(), &width, &height, &channels, channelsRequired);
	
	ImageData truthData = {};
	truthData.Data = data;
	truthData.Width = width;
	truthData.Height = height;
	truthData.Channels = channelsRequired;

	if (!truthData.Data)
	{
		std::string errorStr("Failed to load image ");
		errorStr.append(truth);
		throw std::invalid_argument(errorStr);
	}

	std::filesystem::path testPath(test);
	std::string testStr = testPath.string();
	data = nullptr;
	data = stbi_load(testStr.c_str(), &width, &height, &channels, channelsRequired);

	ImageData testData = {};
	testData.Data = data;
	testData.Width = width;
	testData.Height = height;
	testData.Channels = channelsRequired;

	if (!testData.Data)
	{
		std::string errorStr("Failed to load image ");
		errorStr.append(test);
		throw std::invalid_argument(errorStr);
	}

	if (testData.Width != truthData.Width || testData.Height != truthData.Height || testData.Channels != truthData.Channels)
	{
		throw std::invalid_argument("Image dimensions or number of channels don't match!");
	}

	const double RMAE = ComputeRMAE(truthData, testData);
	stbi_image_free(truthData.Data);
	stbi_image_free(testData.Data);

	return RMAE;
}

double RMAECalculator::ComputeRMAE(const ImageData& truth, const ImageData& test)
{
	double error = 0.0;
	double norm = 0.0;

	const unsigned int count = (unsigned int)(truth.Width * truth.Height * truth.Channels);
	for (unsigned int i = 0u; i < count; i++)
	{
		double truthVal = truth.Data[i] / 255.0;
		double testVal = test.Data[i] / 255.0;

		error += std::abs(testVal - truthVal);
		norm += std::abs(truthVal);
	}

	const double RMAE = error / norm;
	return RMAE;
}
