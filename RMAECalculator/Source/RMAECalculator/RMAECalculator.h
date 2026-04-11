#include <string>

class RMAECalculator
{
private:
	struct ImageData
	{
		unsigned char* Data = nullptr;
		int Width = 0;
		int Height = 0;
		int Channels = 0;
	};

public:
	static double CalculateFromFiles(std::string_view truth, std::string_view test);

private:
	static double ComputeRMAE(const ImageData& truth, const ImageData& test);
};